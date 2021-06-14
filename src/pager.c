#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "globals.h"
#include "pager.h"

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
