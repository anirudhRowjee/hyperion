#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include "globals.h"
#include "utils.h"


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



void print_prompt()
{
    printf("Hy > ");
}



