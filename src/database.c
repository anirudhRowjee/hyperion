#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "pager.h"


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

