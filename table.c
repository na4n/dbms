#include <stdio.h>

struct __attribute__((packed)) table_entry{
  char *fmt;
  int pnum;
  short tnum;
  int idx;
} typedef tnode;

//int layout_struct(tnode t){
//  for(int i = 0; i < strl
//}

int main(){
  tnode t;
  t.fmt = "a";
  t.pnum = 2;
  t.tnum = 3;
  t.idx = 4;

  tnode p;
  p.fmt = "1234567890";
  p.pnum = 2;
  p.tnum = 3;
  p.idx = 4;

  printf("Size of tnode %ld and all members %ld\n", sizeof(t), sizeof(char*)+sizeof(int)*2+sizeof(short));
  printf("Hello World\n");
  return 1;
}
