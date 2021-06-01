#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
 * MomoDB - A Simple SQLite clone
 */


typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandOutcomes;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} StatementPreparationOutcomes;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

// Input Buffer to persistently accept user input
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef struct {
    StatementType type;
} Statement;

// utility to print the prompt
void print_prompt() { printf("MomoDB > ");}


InputBuffer* new_input_buffer()
{
    // create a new Input Buffer out of a chunk of memory
    // TODO setup a free() call for this 
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    // initialize all values to null
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    // send the pointer back
    return input_buffer;
}

void read_input(InputBuffer* input_buffer)
{
    // function to read the user's input into the Input Buffer
    // this takes input from STDIN and dumps it into our input buffer
    // think of it as a fancy, extensible version of scanf()
    ssize_t input_length = getline(
            &(input_buffer->buffer), 
            &(input_buffer->buffer_length), 
            stdin
    );

    // make sure the read was succesfull
    if (input_length <= 0)
    {
        printf("Error reading input!\n");
        exit(EXIT_FAILURE);
    }

    // remove the trailing newline \n
    input_buffer->input_length = input_length - 1;
    input_buffer->buffer[input_length - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer)
{
    // function to free the allocated memory for the Input Buffer
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCommandOutcomes do_meta_command(InputBuffer* input_buffer)
{
    // as of right now, we only support the exit meta command.
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

StatementPreparationOutcomes prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    // this is the simplest SQL compiler to exist
    // function to prepare a statement based on the command
    // parse the SQL here?
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    // if we've reached here, we don't know what command this is
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement)
{
    // TODO fill in
    switch (statement->type)
    {
        case (STATEMENT_INSERT):
            printf("We've just executed an insert statement!\n");
            break;
        case (STATEMENT_SELECT):
            printf("We've just executed a select statement!\n");
            break;
    }
}

int main(int argc, char* argv[])
{
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
            switch(do_meta_command(input_buffer))
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
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf(
                        "Unrecognized Keyword at the start of '%s'\n",
                        input_buffer->buffer
                        );
                continue;
        }

        // finally, execute the statement
        execute_statement(&exec_statement);
        printf("Executed!\n");
    }

    return 0;
}

