#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// gcc -o hyperion src/globals.h src/utils.c src/parser.c src/executor.c src/database.c src/main.c

#include "globals.h"
#include "utils.h"
#include "parser.h"
#include "pager.h"
#include "database.h"
#include "executor.h"

/*
 * Hyperion - A Simple SQLite clone
 *
 * As of now, There exists only a single table
 * ---------------------------------------------
 *  id          integer
 *  username    varchar(32)
 *  email       varchar(255)
 *  -------------------------------------------
 *
 *  Hyperion supports Insertion and Read operations.
 */


int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        printf("Must Supply A Database Filename.\n");
        exit(EXIT_FAILURE);
    }

    char* filename = argv[1];

    // initialize the table
    Table* table = db_open(filename);

    // initialize the new input buffer to accept the commands
    // Since this persists, we use it throughout the lifetime of the application
    InputBuffer* input_buffer = new_input_buffer();

    // loop
    while (true)
    {
        print_prompt();
        read_input(input_buffer);

        // handle meta-command case
        if (input_buffer->buffer[0] == '.')
        {
            switch(do_meta_command(input_buffer, table))
            {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecongized command %s\n", input_buffer->buffer);
                    continue;
            }
        }

        // prepare a Statement to execute
        Statement exec_statement;

        // parse the statement into internal representation
        switch (prepare_statement(input_buffer, &exec_statement))
        {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax Error: Could not Parse Statement\n");
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf(
                        "Unrecognized Keyword at the start of '%s'\n",
                        input_buffer->buffer
                        );
                continue;
            case (PREPARE_STRING_TOO_LONG):
                printf("The String is too long.\n");
                continue;
            case (PREPARE_NEGATIVE_ID):
                printf("The ID cannot be negative.\n");
        }

        // finally, execute the statement
        switch(execute_statement(&exec_statement, table))
        {
            case (EXECUTE_SUCCESS):
                printf("Executed\n");
                break;
            case (EXECUTE_TABLE_FULL):
                printf("Error: The Table is Full!\n");
                break;
        }

        // printf("Executed!\n");
    }

    return 0;
}

