#ifndef utils_h
#define utils_h

#include "globals.h"

/*
 * UTILS
 * -----------
 *  This file contains the functions for general utilities such as 
 *  1. The Input Buffer for Queries
 *  2. Printing Utilities (print a row, the prompt, etc)
 */

InputBuffer* new_input_buffer();
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);
void print_prompt();
void print_row(Row* row);

#endif
