#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID
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
    char username[COLUMN_USERNAME_SIZE+1];
    char email[COLUMN_EMAIL_SIZE+1];
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


// create a Pager
// the pager is an abstraction that allows us to access
// blocks of memory more easily. This will be our primary interface
// with the file.
typedef struct {
    int file_desc;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    uint32_t row_count;
    Pager* pager;
} Table;

Pager* pager_open(const char* filename)
{
    // printf("Opening the Pager!\n");

    // function to create a new pager
    // int fd = open(filename, O_RDWR | O_CREAT | S_IWUSR | S_IRUSR);
    int fd = open(filename,
            O_RDWR |      // Read/Write mode
            O_CREAT,  // Create file if it does not exist
            S_IWUSR |     // User write permission
            S_IRUSR   // User read permission
            );

    if (fd == -1)
    {
        // file opening failed
        printf("Unable to Open file\n");
        exit(EXIT_FAILURE);
    }

    // find the size of the file
    off_t file_length = lseek(fd, 0, SEEK_END);

    Pager* pager = malloc(sizeof(Pager));

    pager->file_desc = fd;
    pager->file_length = file_length;

    // initialize all pages to null
    // do this so we prevent segfaults
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pager->pages[i] = NULL;
    }

    return pager;
}


// function to create a new table
Table* db_open(const char* filename)
{
    // printf("Opening the Database\n");
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_length / ROW_SIZE;

    Table* table = malloc(sizeof(Table));

    // initialize all values to zero or null
    table->pager = pager;
    table->row_count = num_rows;

    return table;
}


void* get_page(Pager* pager, uint32_t page_number)
{
    // function to get a page based on the row number
    // handle CACHE MISSES

    // ensure we don't breach pre-set limits for page numbers
    if (page_number > TABLE_MAX_PAGES)
    {
        printf("Page Number Out of Bounds (%d > %d)\n", page_number, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    // If we want a page number that hasn't been used yet
    // i.e. a page number that's out of bounds of the current
    // set of pages, we'll allocate new memory for that page.
    // As and when we flush the cache to disk (i.e. the in-memory cache),
    // the page will be written to file.

    if (pager->pages[page_number] == NULL)
    {
        // enter badlands
        // allocate new memory for the fresh page
        void* page = malloc(PAGE_SIZE);

        // find the number of pages already present
        // ----
        // pager->file_length is initialized as soon as the database is
        // opened, and it tells us the length of the file at start in 
        // bytes.
        uint32_t num_pages = pager->file_length / PAGE_SIZE;

        // if there's already more data (i.e. a partial page), we 
        // will save it, too
        if (pager->file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }

        // this is to ensure we don't over-allocate for a partial write
        // if we've got a page that we want that isn't the a fresh page,
        // or if we want to load a page that's there previously 
        // (i.e. assume you have 10 pages, and want to read page 6)
        // we use this function to also load pages from the file into
        // memory
        if (page_number <= num_pages)
        {
            // change the file descriptor to start reading from the latest
            // page
            lseek(pager->file_desc, page_number*PAGE_SIZE, SEEK_SET);
            // read bytes from the file into the page 
            // doing this takes care of the partial page problem, 
            // given that the latest page have space to accomodate more rows
            ssize_t bytes_read = read(pager->file_desc, page, PAGE_SIZE);
            // sanity check
            if (bytes_read == -1)
            {
                // TODO figure out what "errno" is 
                printf("Error reading File: %d\n", 0);
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_number] = page;
    }
    return pager->pages[page_number];
}


// this function writes a page number to the file
void pager_flush(Pager* pager, uint32_t page_num,  uint32_t size)
{

    // sanity check
    if (pager->pages[page_num] == NULL)
    {
        printf("Tried to Flush Null Page.\n");
        exit(EXIT_FAILURE);
    }

    // find where we need to start writing from
    off_t offset = lseek(pager->file_desc, page_num * PAGE_SIZE, SEEK_SET);

    if (offset == -1)
    {
        printf("Error seeking start\n");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager->file_desc, pager->pages[page_num], size);

    if (bytes_written == -1)
    {
        printf("Error Writing to File\n");
        exit(EXIT_FAILURE);
    }
}

void db_close(Table* table)
{
    // function to flush the page cache to disk, free all memory
    // and close the file

    Pager* pager = table->pager;
    uint32_t num_full_pages = table->row_count / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < num_full_pages; i++)
    {
        // prevent double-free 
        if (pager->pages[i] == NULL)
        {
            continue;
        }

        // flush the page to file
        pager_flush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    // ensure there isn't a partial page to write
    uint32_t additional_rows = table->row_count % ROWS_PER_PAGE;

    if (additional_rows > 0)
    {
        uint32_t page_num = num_full_pages;
        if (pager->pages[page_num] != NULL)
        {
            pager_flush(pager, page_num, additional_rows * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }

    int result = close(pager->file_desc);
    if (result == -1)
    {
        printf("Error closing the database file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        void* page = pager->pages[i];
        if (page)
        {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}


void* row_slot(Table* table, uint32_t row_number)
{
    // get a slot in the table 

    // rows are stored sequentially in pages, so we find the page
    // index based on the count of the row
    uint32_t page_number = row_number / ROWS_PER_PAGE;

    void* page = get_page(table->pager, page_number);

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

