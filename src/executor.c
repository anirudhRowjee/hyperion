#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "utils.h"
#include "pager.h"
#include "database.h"


MetaCommandOutcomes do_meta_command(InputBuffer* input_buffer, Table* table)
{
    // as of right now, we only support the exit meta command.
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

ExecuteResult execute_insert(Statement* statement, Table* table)
{
    // make sure the table isn't full
    if (table->row_count >= TABLE_MAX_ROWS)
    {
        return EXECUTE_TABLE_FULL;
    }
    else
    {
        // get the row we need to insert from the statement parser
        Row* row_to_insert = &(statement->row_to_insert);
        // serialize the row and write it to the location in the table
        serialize_row(row_to_insert, row_slot(table, table->row_count));
        // increment the number of rows
        table->row_count += 1;
        return EXECUTE_SUCCESS;
    }
}

ExecuteResult execute_select(Statement* statement, Table* table)
{
    // create a new temporary location to hold the row
    Row row;
    for (uint32_t i = 0; i < table->row_count; i++)
    {
        // copy from memory to temporary
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table)
{
    // TODO fill in
    switch (statement->type)
    {
        case (STATEMENT_INSERT):
            return execute_insert(statement, table);
        case (STATEMENT_SELECT):
            return execute_select(statement, table);
    }
}

