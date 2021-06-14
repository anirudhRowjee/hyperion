#ifndef globals_h
#define globals_h
/*
 * Hyperion : A Minimal, Lightweight SQLite Clone
 * ----------------------------------------------
 * This header File Contains:
 * 1. Macros
 * 2. Structures
 * 3. Constant Variables
 * 4. Small Utility Functions
 */
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

// define some constants
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100


/*
 *  ---------------- ENUMS FOR ERROR HANDLING AND RETURN GUARANTEES ----
 */

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

/*
 *  ---------------- BASIC STRUCTURES AND TYPES -------------------------
 */

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

typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;

/*
 * ----------------- CONSTANT VALUES -----------------------------------
 */

extern const uint32_t ID_SIZE;
extern const uint32_t USERNAME_SIZE;
extern const uint32_t EMAIL_SIZE;
extern const uint32_t ID_OFFSET;
extern const uint32_t USERNAME_OFFSET;
extern const uint32_t EMAIL_OFFSET;
extern const uint32_t ROW_SIZE;
extern const uint32_t PAGE_SIZE; 
extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;

#endif
