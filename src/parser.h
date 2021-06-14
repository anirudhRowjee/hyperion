/*
 * PARSER
 * ------------------
 *  This file contains utilities for the following - 
 *  1. Accepting the query from the user
 *  2. Parsing the Query to decide if it's a meta-command or an SQL query
 *  3. Tokenizing the SQL query into internal representation
 *  4. Preparing the internal representation to be passed to the executor
 */
#ifndef parser_h
#define parser_h

#include "globals.h"

StatementPreparationOutcomes prepare_insert(InputBuffer* input_buffer, Statement* statement);
StatementPreparationOutcomes prepare_statement(InputBuffer* input_buffer, Statement* statement);

#endif
