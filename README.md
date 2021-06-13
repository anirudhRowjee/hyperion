# MomoDB - A Simple, Limited SQLite Clone

[Anirudh Rowjee](https://github.com/anirudhRowjee)

## Aim
The aim of this project is to implement a simple, persistent database management system. The system will be able to handle a minimal subset of SQL (limited to a single table, `INSERT` and `SELECT`), and will persist data on to disk.

The Planning and Design on this database will mostly be inspired by SQLite, given the wealth of documentation / implementation available on said implementation.

This project will be implemented entirely in C, and version control (along with a Github Repository) will be used to keep track of all the work.

## Usage
* Ensure you have `gcc` and `make` installed
* Clone this Repository using `git clone`
* run `make` to build the binary file
* run `momodb` (or `momodb.exe` if you're on windows) to start the database

## Run commands
```shell
$ ./momodb <database file>
```

## Learning Outcome
The implementation of this project will include (but will not be limited to) learning the following concepts - 
1. Dynamic Memory Allocation / re-allocation (using `valgrind` to check for memory leaks)
2. Structures and Pointers
3. Minimal SQL parser and tokenizer implementation
4. Serializing/Deserializing Data Structures to bytes
5. Writing/reading serialized data to/from disk
6. BTree implementation


