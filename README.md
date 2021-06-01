# UE20CS151 Project Proposal
[Anirudh Rowjee](https://www.github.com/anirudhRowjee) `PES2UG20CS050`

## Aim
The aim of this project is to implement a simple, persistent database management system. The system will be able to handle a minimal subset of SQL (limited to a single table, `INSERT` and `SELECT`), and data will be persisted onto disk. 

The Planning and Design on this database will mostly be inspired by SQLite, given the wealth of documentation / implementation available on said implementation.

This project will be implemented entirely in C, and version control (along with a Github Repository) will be used to keep track of all the work.

## Learning Outcome
The implementation of this project will include (but will not be limited to) learning the following concepts - 
1. Dynamic Memory Allocation / re-allocation (using `valgrind` to check for memory leaks)
2. Structures and Pointers
3. Minimal SQL parser and tokenizer implementation
4. Serializing/Deserializing Data Structures to bytes
5. Writing/reading serialized data to/from disk
6. BTrees/B+ Trees and implementation 
7. Prospective : Multithreading to ensure concurrent access
8. Prospective : Network Interfaces using raw Berkely Sockets to support non-local query execution
9. Unit Testing built binaries using `python3` and `subprocess`


