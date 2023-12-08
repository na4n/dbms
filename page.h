#ifndef PAGE_H
#define PAGE_H

struct page_header{
  int tupct;
  float ver;
  int nfree;
} typedef phead;

typedef int tptr;

phead PAGE_HEAD_INITIALIZER = {.tupct=-1, .ver=-1, .nfree=-1};

#endif

