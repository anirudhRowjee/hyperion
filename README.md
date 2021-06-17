# Hyperion - A Lightweight, Minimal SQLite Clone

Hyperion is minimal, lightweight implementation of a relational database management system written in Pure C.
At the moment, the following features are supported.

* [x] Single Static Table
* [x] `SELECT` queries
* [x] `INSERT` queries
* [x] In-Memory Storage
* [x] Persistance to disk
* [x] Minimal SQL Parsing and SQLite Meta-Command Support
* [ ] B-Tree Support

## Installation
1. Clone this repository using `git clone`
2. `cd` into the directory you cloned into
3. Run `make` (ensure you have `make` and `gcc` installed)
4. Run the executable built (i.e. `./hyperion`)

## Project Structure
```
.
├── Makefile
├── proposal.pdf
├── test.py               // rudimentary testing script to mock Rspec
├── README.md
└── src
    ├── database.c        // Loads the Database and Table
    ├── database.h
    ├── executor.c        // accepts compiled statements and executes them
    ├── executor.h
    ├── pager.c           // Memory IO and the Cursor Abstraction
    ├── pager.h
    ├── parser.c          // parses the text input into internal statement representation
    ├── parser.h
    ├── utils.c          // General Utilities - Input Buffer, Prompt, etc
    ├── utils.h
    ├── globals.h         // important macros, typdefs and structs
    └── main.c            // driver code

1 directory, 13 files
```

## Contributing
1. Fork this repository
2. Clone the forked repository
3. Switch to a new branch within the repository using `git checkout -b <your branch name>`
4. Make all your changes within your branch - don't forget to commit often!
5. Once you're done, push your changes to the origin using `git push -u origin <your branch name>`
6. Open a Pull Request, and await code review!

## About
This Project drew inspiration from [this](https://cstack.github.io/db_tutorial/) tutorial.
