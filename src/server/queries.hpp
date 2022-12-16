#ifndef _QUERIES_HPP
#define _QUERIES_HPP

#include <pthread.h>
#include <semaphore.h>

#include <cstdio>

#include "db.hpp"

typedef struct
{
    pthread_mutex_t new_access, write_access, reader_registration;
    int readers_c = 0;
} mutex_t;

// execute_* //////////////////////////////////////////////////////////////////

std::vector<std::string> execute_select(int fout, database_t *const db, const char *const field,
                                        const char *const value);

std::vector<std::string> execute_update(int fout, database_t *const db, const char *const ffield,
                                        const char *const fvalue, const char *const efield, const char *const evalue);

std::vector<std::string> execute_insert(int fout, database_t *const db, const char *const fname,
                                        const char *const lname, const unsigned id, const char *const section,
                                        const tm birthdate);

std::vector<std::string> execute_delete(int fout, database_t *const db, const char *const field,
                                        const char *const value);

// parse_and_execute_* ////////////////////////////////////////////////////////

std::vector<std::string> parse_and_execute_select(int fout, database_t *db, const char *const query);

std::vector<std::string> parse_and_execute_update(int fout, database_t *db, const char *const query);

std::vector<std::string> parse_and_execute_insert(int fout, database_t *db, const char *const query);

std::vector<std::string> parse_and_execute_delete(int fout, database_t *db, const char *const query);

std::vector<std::string> parse_and_execute(int fout, database_t *db, const char *const query, mutex_t *mutex);

// query_fail_* ///////////////////////////////////////////////////////////////

/** Those methods write a descriptive error message on fout */

std::vector<std::string> query_fail_bad_query_type(int fout);

std::vector<std::string> query_fail_bad_format(int fout, const char *const query_type);

std::vector<std::string> query_fail_too_long(int fout, const char *const query_type);

std::vector<std::string> query_fail_bad_filter(int fout, const char *const field, const char *const filter);

std::vector<std::string> query_fail_bad_update(int fout, const char *const field, const char *const filter);

#endif
