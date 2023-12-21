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
- phead phead_ret(char *pname): returns a page header for associated page
- int phead_wrt(char *pname, phead *p): writes the page header in the specified page
- int page_init(char *pname): creates and initializes a page with the default page header
- int tuple_add(char *pname, char *fmt, void **arg): adds a tuple to the page
- int tuple_decode(char *pname, int t): decodes tuple t on the page in pname
- int tuple_remove(char *pname, int n): removes tuple n on the page in pname
