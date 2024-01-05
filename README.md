# Database Management System
- A relational database management system (dbms) made from scratch in C. Currently strings, floats, and ints can be added throught the command line.
- The dbms uses heapfile pages where each row is stored as a tuple in a given page and is accessible by accessing an offset within that page.
- Support for tables, i.e. the ability to group and track tuples is being added

# Installation Instructions
- Clone the repository into a given folder and navigate to that folder
- Compile page.c using [gcc](https://gcc.gnu.org/), [clang](https://clang.llvm.org/), or any other C compiler (or any other compiler that supports the C99 standard)

# Usage
- run the executable (default executable name is a.out, ran with the command ./a.out) and supply one of the following arguments
- add \[page_name\] \[series of strings, floats, and ints to be added\]
- debug \[page_name\] to see the locations of tuples (rows) in a page
- decode \[page_name\] \[tuple_number\] to see specific tuple data

# Contact Information
- Contact me at the following email
[![Firefox Relay Alias](/assets/email.png "No spam please")]

# Documentation
- Documentation for current development work is on Github Pages: [https://na4n.github.io/dbms/](https://na4n.github.io/dbms/)

