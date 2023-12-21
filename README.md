# DBMS
- a database management system

# Tasks
    - parse tree
    - track structure
        - track rows
    - delete tuple
    - garbage collect

# Program Outline
## Functions
    - int phead_null(phead p): checks if phead is null
    - phead phead_ret(char \*pname): returns a page header for associated page
    - int phead_wrt(char \*pname, phead \*p): writes page header in specified page
    - int page_init(char \*pname): creates and initializes page with default page header
    - int tuple_add(char \*pname, char \*fmt, void \*\*arg): adds tuple to page
    - int tuple_decode(char \*pname, int t): decodes tuple t in pname: decodes tuple t in page
    - int tuple_remove(char \*pname, int n): removes tuple n in page

