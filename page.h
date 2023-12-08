#ifndef PAGE_H
#define PAGE_H
struct page_header{
  int tupct;
  float ver;
  int nfree;
} typedef phead;

struct tuple_header{
  char fmt[10];
  int loc;
} typedef thead;

phead PAGE_HEAD_INITIALIZER = {.tupct=-1, .ver=-1, .nfree=-1};

phead gethead(char *pname);
int hdwrite(char *pname, phead *p);
int init(char *pname);
int add(char *pname, char *fmt, void **arg);

#endif

