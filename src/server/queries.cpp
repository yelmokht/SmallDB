#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

#include <fstream>
#include <iostream>

#include "student.hpp"
#include "queries.hpp"
#include "common.hpp"

// execute_* ///////////////////////////////////////////////////////////////////

void execute_select(int fout, database_t *const db, const char *const field, const char *const value)
{
	char buffer[512];
	int count = 0;
	std::function<bool(const student_t &)> predicate = get_filter(field, value);
	if (!predicate)
	{
		query_fail_bad_filter(fout, field, value);
		return;
	}
	for (const student_t &s : db->data)
	{
		if (predicate(s))
		{
			student_to_str(buffer, &s, sizeof(buffer));
			sendSocket(fout, buffer);
			++count;
		}
	}
	sprintf(buffer, "%d student(s) selected\n", count);
	sendSocket(fout, buffer);
	sprintf(buffer, "%d", EOF);
	sendSocket(fout, buffer);
}

void execute_update(int fout, database_t *const db, const char *const ffield, const char *const fvalue, const char *const efield, const char *const evalue)
{
	char buffer[512];
	int count = 0;
	std::function<bool(const student_t &)> predicate = get_filter(ffield, fvalue);
	if (!predicate)
	{
		query_fail_bad_filter(fout, ffield, fvalue);
		return;
	}
	std::function<void(student_t &)> updater = get_updater(efield, evalue);
	if (!updater)
	{
		query_fail_bad_update(fout, efield, evalue);
		return;
	}
	for (student_t &s : db->data)
	{
		if (predicate(s))
		{
			updater(s);
			++count;
		}
	}
	sprintf(buffer, "%d student(s) updated\n", count);
	sendSocket(fout, buffer);
	sprintf(buffer, "%d", EOF);
	sendSocket(fout, buffer);
}

void execute_insert(int fout, database_t *const db, const char *const fname,
					const char *const lname, const unsigned id, const char *const section,
					const tm birthdate)
{
	db->data.emplace_back();
	student_t *s = &db->data.back();
	s->id = id;
	snprintf(s->fname, sizeof(s->fname), "%s", fname);
	snprintf(s->lname, sizeof(s->lname), "%s", lname);
	snprintf(s->section, sizeof(s->section), "%s", section);
	s->birthdate = birthdate;
	char buffer[512];
	student_to_str(buffer, s, sizeof(buffer));
	sendSocket(fout, buffer);
	sprintf(buffer, "%d", EOF);
	sendSocket(fout, buffer);
}

void execute_delete(int fout, database_t *const db, const char *const field,
					const char *const value)
{
	std::function<bool(const student_t &)> predicate = get_filter(field, value);
	if (!predicate)
	{
		query_fail_bad_filter(fout, field, value);
		return;
	}
	int old_size = db->data.size();
	auto new_end = remove_if(db->data.begin(), db->data.end(), predicate);
	db->data.erase(new_end, db->data.end());
	int new_size = db->data.size();
	int diff = old_size - new_size;
	std::string buffer = std::to_string(diff) + " deleted student(s)\n";
	sendSocket(fout, buffer);
	sendSocket(fout, std::to_string(EOF));
}

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(int fout, database_t *db, const char *const query)
{
	char ffield[32], fvalue[64]; // filter data
	int counter;
	if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2)
	{
		query_fail_bad_format(fout, "select");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		query_fail_too_long(fout, "select");
	}
	else
	{
		execute_select(fout, db, ffield, fvalue);
	}
}

void parse_and_execute_update(int fout, database_t *db, const char *const query)
{
	char ffield[32], fvalue[64]; // filter data
	char efield[32], evalue[64]; // edit data
	int counter;
	if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue,
			   &counter) != 4)
	{
		query_fail_bad_format(fout, "update");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		query_fail_too_long(fout, "update");
	}
	else
	{
		execute_update(fout, db, ffield, fvalue, efield, evalue);
	}
}

void parse_and_execute_insert(int fout, database_t *db, const char *const query)
{
	char fname[64], lname[64], section[64], date[11];
	unsigned id;
	tm birthdate;
	int counter;
	if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || strptime(date, "%d/%m/%Y", &birthdate) == NULL)
	{
		query_fail_bad_format(fout, "insert");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		query_fail_too_long(fout, "insert");
	}
	else
	{
		execute_insert(fout, db, fname, lname, id, section, birthdate);
	}
}

void parse_and_execute_delete(int fout, database_t *db, const char *const query)
{
	char ffield[32], fvalue[64]; // filter data
	int counter;
	if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2)
	{
		query_fail_bad_format(fout, "delete");
	}
	else if (static_cast<unsigned>(counter) < strlen(query))
	{
		query_fail_too_long(fout, "delete");
	}
	else
	{
		execute_delete(fout, db, ffield, fvalue);
	}
}

void parse_and_execute(int fout, database_t *db, const char *const query, mutex_t *mutex)
{
	if (strncmp("select", query, sizeof("select") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->reader_registration);
		if (mutex->readers_c == 0)
			pthread_mutex_lock(&mutex->write_access);
		mutex->readers_c++;
		pthread_mutex_unlock(&mutex->new_access);
		pthread_mutex_unlock(&mutex->reader_registration);
		parse_and_execute_select(fout, db, query);
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
		parse_and_execute_update(fout, db, query);
		pthread_mutex_unlock(&mutex->write_access);
	}
	else if (strncmp("insert", query, sizeof("insert") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->write_access);
		pthread_mutex_unlock(&mutex->new_access);
		parse_and_execute_insert(fout, db, query);
		pthread_mutex_unlock(&mutex->write_access);
	}
	else if (strncmp("delete", query, sizeof("delete") - 1) == 0)
	{
		pthread_mutex_lock(&mutex->new_access);
		pthread_mutex_lock(&mutex->write_access);
		pthread_mutex_unlock(&mutex->new_access);
		parse_and_execute_delete(fout, db, query);
		pthread_mutex_unlock(&mutex->write_access);
	}
	else
	{
		query_fail_bad_query_type(fout);
	}
}

// query_fail_* ///////////////////////////////////////////////////////////////

void query_fail_bad_query_type(int fout)
{
	std::string buffer = "Query fail: Bad query type!";
	sendSocket(fout, buffer);
	sendSocket(fout, std::to_string(EOF));
}

void query_fail_bad_format(int fout, const char *const query_type)
{
	std::string buffer = "Query fail: Bad format: ";
	buffer.append(query_type);
	sendSocket(fout, buffer);
	sendSocket(fout, std::to_string(EOF));
}

void query_fail_too_long(int fout, const char *const query_type)
{
	std::string buffer = "Query fail: Bad too log: ";
	buffer.append(query_type);
	sendSocket(fout, buffer);
	sendSocket(fout, std::to_string(EOF));
}

void query_fail_bad_filter(int fout, const char *const field, const char *const filter)
{
	std::string buffer = "Query fail: Bad filter: ";
	buffer.append(field);
	buffer.append(filter);
	sendSocket(fout, buffer);
	sendSocket(fout, std::to_string(EOF));
}

void query_fail_bad_update(int fout, const char *const field, const char *const filter)
{
	std::string buffer = "Query fail: Bad update: ";
	buffer.append(field);
	buffer.append(filter);
	sendSocket(fout, buffer);
	sendSocket(fout, std::to_string(EOF));
}
