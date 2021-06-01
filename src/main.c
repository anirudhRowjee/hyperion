#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
 * MomoDB - A Simple SQLite clone
 */


// Input Buffer to persistently accept user input
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

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

        if (strcmp(input_buffer->buffer, ".exit") == 0)
        {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Unrecognized Command '%s'. \n", input_buffer->buffer);
        }
    }

    return 0;
}

