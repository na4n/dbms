#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "page.h"

const float ver = 0.01;

int init(char *pname){
  p_head p = {.tupct=0, .ver=ver};
  char buf[sizeof(p_head)];
  memcpy(buf, &p, sizeof(p_head));
  
  FILE *fp;
  if((fp = fopen(pname, "w")) == NULL){
    return 1;
  }
  fwrite(buf, sizeof(p_head), 1, fp);
  fclose(fp);

  return 0;
}

int pgread(char *pname){
  // read header struct
  // iterate through tuple headers
  // print data

  return 1;
}

int pgwrite(char *pname){
  // update header struct
  // find next write
  // write

  return 1;
}

int main(){
  init("test");
  return 0;
}
