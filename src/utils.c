#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "utils.h"


// declare some constant sizes here to link as external variables
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


// utility to print the prompt
void print_prompt() { printf("H > ");}

// utility to print a row
void print_row(Row* row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

