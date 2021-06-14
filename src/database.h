/* 
 * DATABASE 
 * ------------------------
 *  This file contains utilities for the following purposes
 *  1. Opening the Database from the file
 *  2. Loading said data into memory
 *  3. Running a query on the in-memory cache
 *  4. Flushing the Cache to Disk once the database is closed
 */
#ifndef database_h
#define database_h

#include "globals.h"

Table* db_open(const char* filename);
void db_close(Table* table);

#endif

