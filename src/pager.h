/*
 * PAGER
 * -----------
 *  This file contains utilities to handle memory I/O and other
 *  related abstractions, including but not limited to
 *  1. Serializing and De-serializing rows
 *  2. The Pager Abstraction to handle easy access to rows
 *  3. Function to get pages
 *  4. Write the cache to disk
 *  5. Get the latest row number / page to write to
 */
#ifndef pager_h
#define pager_h

#include "globals.h"

Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_number);
void pager_flush(Pager* pager, uint32_t page_num,  uint32_t size);
void* row_slot(Table* table, uint32_t row_number);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);

#endif
