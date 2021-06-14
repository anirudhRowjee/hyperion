#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "parser.h"

// dedicated function to prepare the insert statement using 
// strtok() to prevent buffer overflows from scanf()
StatementPreparationOutcomes prepare_insert(InputBuffer* input_buffer, Statement* statement)
{

    statement->type = STATEMENT_INSERT;
    // local variables to hold tokenized data
    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if (id_string == NULL || username == NULL || email == NULL)
    {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if (id < 0)
    {
        return PREPARE_NEGATIVE_ID;
    }


    if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE)
    {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

StatementPreparationOutcomes prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    // this is the simplest SQL compiler to exist
    // function to prepare a statement based on the command
    // parse the SQL here?
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        return prepare_insert(input_buffer, statement);
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    // if we've reached here, we don't know what command this is
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

