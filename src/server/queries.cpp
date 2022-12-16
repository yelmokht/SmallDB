#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <fstream>
#include <iostream>

#include "../common.hpp"
#include "queries.hpp"
#include "student.hpp"

using namespace std;

// execute_* ///////////////////////////////////////////////////////////////////

vector<string> execute_select(int fout, database_t *const db, const char *const field, const char *const value)
{
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	int count = 0;
	std::function<bool(const student_t &)> predicate = get_filter(field, value);
	if (!predicate)
	{
		return query_fail_bad_filter(fout, field, value);
	}
	for (const student_t &s : db->data)
	{
		if (predicate(s))
		{
			// Génération du message partielle
			student_to_str(buffer, &s, sizeof(buffer));
			messages.push_back(buffer);
			++count;
		}
	}
	// Génération du message
	sprintf(buffer, "%d student(s) selected\n", count);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> execute_update(int fout, database_t *const db, const char *const ffield, const char *const fvalue, const char *const efield, const char *const evalue)
{
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	int count = 0;
	std::function<bool(const student_t &)> predicate = get_filter(ffield, fvalue);
	if (!predicate)
	{
		return query_fail_bad_filter(fout, ffield, fvalue);
	}
	std::function<void(student_t &)> updater = get_updater(efield, evalue);
	if (!updater)
	{
		return query_fail_bad_update(fout, efield, evalue);
	}
	for (student_t &s : db->data)
	{
		if (predicate(s))
		{
			updater(s);
			++count;
		}
	}
	// Génération du message
	sprintf(buffer, "%d student(s) updated\n", count);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> execute_insert(int fout, database_t *const db, const char *const fname,
							  const char *const lname, const unsigned id, const char *const section,
							  const tm birthdate)
{
	vector<string> messages;
	db->data.emplace_back();
	student_t *s = &db->data.back();
	s->id = id;
	snprintf(s->fname, sizeof(s->fname), "%s", fname);
	snprintf(s->lname, sizeof(s->lname), "%s", lname);
	snprintf(s->section, sizeof(s->section), "%s", section);
	s->birthdate = birthdate;
	// Génération du message
	char buffer[BUFFER_SIZE];
	student_to_str(buffer, s, sizeof(buffer));
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> execute_delete(int fout, database_t *const db, const char *const field,
							  const char *const value)
{
	vector<string> messages;
	std::function<bool(const student_t &)> predicate = get_filter(field, value);
	if (!predicate)
	{
		return query_fail_bad_filter(fout, field, value);
	}
	int old_size = db->data.size();
	auto new_end = remove_if(db->data.begin(), db->data.end(), predicate);
	db->data.erase(new_end, db->data.end());
	// Génération du message
	int new_size = db->data.size();
	int diff = old_size - new_size;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%d student(s) deleted\n", diff);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

// parse_and_execute_* ////////////////////////////////////////////////////////

vector<string> parse_and_execute_select(int fout, database_t *db, const char *const query)
{
	char ffield[32], fvalue[64]; // filter data
	int counter;
	if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2)
	{
		return query_fail_bad_format(fout, "select");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		return query_fail_too_long(fout, "select");
	}
	else
	{
		return execute_select(fout, db, ffield, fvalue);
	}
}

vector<string> parse_and_execute_update(int fout, database_t *db, const char *const query)
{
	char ffield[32], fvalue[64]; // filter data
	char efield[32], evalue[64]; // edit data
	int counter;
	if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue,
			   &counter) != 4)
	{
		return query_fail_bad_format(fout, "update");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		return query_fail_too_long(fout, "update");
	}
	else
	{
		return execute_update(fout, db, ffield, fvalue, efield, evalue);
	}
}

vector<string> parse_and_execute_insert(int fout, database_t *db, const char *const query)
{
	char fname[64], lname[64], section[64], date[11];
	unsigned id;
	tm birthdate;
	int counter;
	if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || strptime(date, "%d/%m/%Y", &birthdate) == NULL)
	{
		return query_fail_bad_format(fout, "insert");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		return query_fail_too_long(fout, "insert");
	}
	else
	{
		return execute_insert(fout, db, fname, lname, id, section, birthdate);
	}
}

vector<string> parse_and_execute_delete(int fout, database_t *db, const char *const query)
{
	char ffield[32], fvalue[64]; // filter data
	int counter;
	if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2)
	{
		return query_fail_bad_format(fout, "delete");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		return query_fail_too_long(fout, "delete");
	}
	else
	{
		return execute_delete(fout, db, ffield, fvalue);
	}
}

vector<string> parse_and_execute(int fout, database_t *db, const char *const query, mutex_t *mutex)
{
	vector<string> messages;
	if (strncmp("select", query, sizeof("select") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->reader_registration);
		if (mutex->readers_c == 0)
			pthread_mutex_lock(&mutex->write_access);
		mutex->readers_c++;
		pthread_mutex_unlock(&mutex->new_access);
		pthread_mutex_unlock(&mutex->reader_registration);
		messages = parse_and_execute_select(fout, db, query);
		pthread_mutex_lock(&mutex->reader_registration);
		mutex->readers_c--;
		if (mutex->readers_c == 0)
			pthread_mutex_unlock(&mutex->write_access);
		pthread_mutex_unlock(&mutex->reader_registration);
	}
	else if (strncmp("update", query, sizeof("update") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->write_access);
		pthread_mutex_unlock(&mutex->new_access);
		messages = parse_and_execute_update(fout, db, query);
		pthread_mutex_unlock(&mutex->write_access);
	}
	else if (strncmp("insert", query, sizeof("insert") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->write_access);
		pthread_mutex_unlock(&mutex->new_access);
		messages = parse_and_execute_insert(fout, db, query);
		pthread_mutex_unlock(&mutex->write_access);
	}
	else if (strncmp("delete", query, sizeof("delete") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->write_access);
		pthread_mutex_unlock(&mutex->new_access);
		messages = parse_and_execute_delete(fout, db, query);
		pthread_mutex_unlock(&mutex->write_access);
	}
	else
	{
		messages = query_fail_bad_query_type(fout);
	}
	return messages;
}

// query_fail_* ///////////////////////////////////////////////////////////////

vector<string> query_fail_bad_query_type(int fout)
{
	vector<string> messages;
	char buffer[] = "Error: Bad query type!\n";
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> query_fail_bad_format(int fout, const char *const query_type)
{
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "Error: Bad format: %s\n", query_type);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> query_fail_too_long(int fout, const char *const query_type)
{
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "Error: Bad too long: %s\n", query_type);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> query_fail_bad_filter(int fout, const char *const field, const char *const filter)
{
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "Error: Bad filter: %s=%s\n", field, filter);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}

vector<string> query_fail_bad_update(int fout, const char *const field, const char *const filter)
{
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "Error: Bad update: %s=%s\n", field, filter);
	messages.push_back(buffer);
	messages.push_back(END_OF_MESSAGE);
	return messages;
}
