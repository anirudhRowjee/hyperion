#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/*
 * MomoDB - A Simple SQLite clone
 *
 * As of now, There exists only a single table
 * ---------------------------------------------
 *  id          integer
 *  username    varchar(32)
 *  email       varchar(255)
 *  -------------------------------------------
 *
 *  MomoDB supports Insertion and Read operations.
 */


// define some constants
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100


typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandOutcomes;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} StatementPreparationOutcomes;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef enum {
    EXECUTE_TABLE_FULL,
    EXECUTE_SUCCESS
} ExecuteResult;

// Input Buffer to persistently accept user input
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

// this is a row of our table
typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;


// define field constant sizes
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

// define attribute sizes
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);

// define memory offsets for easy serialization
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

// table definitions and limits

const uint32_t PAGE_SIZE = 4096; 
// 4Kb as most operating systems size pages at 4Kb
// this means that pages won't be broken up by the operating system
// as 4Kb is the virtual memory size of the system
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t row_count;
    void* pages[TABLE_MAX_PAGES];
} Table;


// function to create a new table
Table* new_table()
{
    Table* table = malloc(sizeof(Table));
    // initialize all values to zero or null
    table->row_count = 0;
    // initialize blank pages
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table* table)
{
    // free all memory allocated for the table
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        free(table->pages[i]);
    }
    free(table);
}

void* row_slot(Table* table, uint32_t row_number)
{
    // get a slot in the table 

    // rows are stored sequentially in pages, so we find the page
    // index based on the count of the row
    uint32_t page_number = row_number / ROWS_PER_PAGE;
    // initialize pointer to page memory
    void* page = table->pages[page_number];

    // in the event the page doesn't exist, we allocate it
    if (page == NULL)
    {
        page = table->pages[page_number] = malloc(PAGE_SIZE);
    }

    // find the "index" of the row in the page
    uint32_t row_offset = row_number % ROWS_PER_PAGE;
    // find the byte offset for the row
    uint32_t byte_offset = row_offset * ROW_SIZE;

    // return the exact memory location of the new row
    return page + byte_offset;
}

typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;

// utility to print the prompt
void print_prompt() { printf("MomoDB > ");}

// utility to print a row
void print_row(Row* row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

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
        // given that we're constrained to a single table, it's easy for us to
        // expect the input format and this write it to memory.
        int arg_count = sscanf(
            input_buffer->buffer,
            "insert %d %s %s",
            &(statement->row_to_insert.id),
            statement->row_to_insert.username,
            statement->row_to_insert.email
        );
        if (arg_count < 3)
        {
            // something is off
            return PREPARE_SYNTAX_ERROR;
        }
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
// serialization and deserialization for the rows
void serialize_row(Row* source, void* destination)
{
    // copy Y size data located at the second pointer to the first
    // pointer at a specific offset
    // printf("Before Copying (%d %s %s)\n", source->id, source->username, source->email);
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
    // at this point of time, all our struct data has been placed at the location
    // we want it to be at, which will be a page.
}

void deserialize_row(void* source, Row* destination)
{
    // same thing as serializer, just reverse - we move data from the 
    // page to the struct.
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
    // printf("Deserialized Row: (%d %s %s)\n", destination->id, destination->username, destination->email);
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

int main(int argc, char* argv[])
{
    // initialize the table
    Table* table = new_table();
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
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax Error: Could not Parse Statement\n");
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf(
                        "Unrecognized Keyword at the start of '%s'\n",
                        input_buffer->buffer
                        );
                continue;
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

