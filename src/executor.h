/*
 * EXECUTOR
 * ---------
 *  This file contains the utilities to dispatch the appropriate
 *  functions to handle various types of the internal command representation.
 */
#ifndef executor_h
#define executor_h

#include "globals.h"

MetaCommandOutcomes do_meta_command(InputBuffer* input_buffer, Table* table);
ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);
ExecuteResult execute_statement(Statement* statement, Table* table);

#endif
