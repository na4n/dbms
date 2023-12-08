#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "page.h"

const float VERSION = 0.01;
const int TUPLE_MAX = 10;

phead gethead(char *pname){
  phead p = PAGE_HEAD_INITIALIZER;

  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return p;
  }

  char buf[sizeof(phead)];
  fread(buf, sizeof(char), sizeof(phead), fp);
  fclose(fp);
  memcpy(&p, buf, sizeof(phead));
  return p;
}

int hdwrte(char *pname, phead *p){
  FILE *fp;
  if((fp = fopen(pname, "w")) == NULL){
    return 1;
  }

  char buf[sizeof(phead)];
  memcpy(buf, p, sizeof(phead));
  fwrite(buf, sizeof(phead), 1, fp);
  fclose(fp);

  return 0;
}

int init(char *pname){
  phead p = {.tupct=0, .ver=VERSION, .nfree=4096};
  if(hdwrte(pname, &p) == 1){
    return 1;
  }

  return 0;
}

  //update header
  //  calculate next header pointer location
  //  calculate next insert location (add arg sizes)
  //  check if sufficient space
  //  insert if so and return 0, otherwise return 1

int add(char *fname, char *fmt, void **arg) {
  void *buf = NULL;
  int l = 0;
  int asize;

  for (int j = 0; j < strlen(fmt); j++) {
    if (fmt[j] == 'f') {
      asize = sizeof(double);
      buf = realloc(buf, l + asize);
      memcpy(buf + l, (double *)arg[j], asize);
      l += asize;
    } else if (fmt[j] == 'l') {
      asize = sizeof(long);
      buf = realloc(buf, l + asize);
      memcpy(buf + l, (long *)arg[j], asize);
      l += asize;
    } else if (fmt[j] == 's') {
      asize = strlen(*(char **)arg[j]) + 1; // +1 for null terminator
      buf = realloc(buf, l + asize);
      memcpy(buf + l, *(char **)arg[j], asize);
      l += asize;
    }
  }

  // double *d = malloc(sizeof(double));
  // long *a = malloc(sizeof(long));
  // char *s = malloc(100);
  // int loc = 0;

  // for (int i = 0; i < strlen(fmt); i++) {
  //   if (fmt[i] == 'f') {
  //     memcpy(d, buf + loc, sizeof(double));
  //     loc += sizeof(double);
  //     printf("Argument %d: %f\n", i, *d);
  //   } else if (fmt[i] == 'l') {
  //     memcpy(a, buf + loc, sizeof(long));
  //     loc += sizeof(long);
  //     printf("Argument %d: %ld\n", i, *a);
  //   } else if (fmt[i] == 's') {
  //     strcpy(s, buf + loc);
  //     loc += strlen(s) + 1; // +1 for null terminator
  //     printf("Argument %d: %s\n", i, s);
  //   }
  // }

  // free(d);
  // free(a);
  // free(s);
  free(buf);

  // int c = 10 - strlen(fmt);
  // for(int i = 0 ; i < c; i++){
  //   strcat(fmt, "0");
  // }

  // phead p = gethead(fname);
  // if(p.tupct == -1 && p.ver == -1 && p.nfree == -1){
  //   return 1;
  // }
  
  // int hloc = sizeof(phead) + 10 * p.tupct;

  // printf("argument: %s\n", fmt);
  // return 1;  

  return 0;
}

int main(int argc, char **argv){
  //init("test");
  //add("test");
  if(argc == 1){
    return 0;
  }

  if(strcmp(argv[1], "add") == 0){
    if(argc > TUPLE_MAX+2){
      perror("too many arguments");
      return 1;
    }

    void **buf = (void **)malloc(sizeof(void*)*(argc - 2));
    char argfrmt[argc-2];
    argfrmt[0] = '\0';
    for(int i = 2; i < argc; i++){
      if(strcmp(argv[i], "0") != 0 && atof(argv[i]) != 0.0 && atof(argv[i]) != atol(argv[i])){
        strcat(argfrmt, "f");
        buf[i-2] = malloc(sizeof(double*));
        *(double*)buf[i-2] = atof(argv[i]);
      }
      else if(strcmp(argv[i], "0") != 0 && atol(argv[i]) != 0){
        strcat(argfrmt, "l");
        buf[i-2] = malloc(sizeof(long int*));
        *(long int*)buf[i-2] = atol(argv[i]);
      }
      else{
        strcat(argfrmt, "s");
        buf[i-2] = malloc(sizeof(char*));
        *(char**)buf[i-2] = argv[i];
      }
    }

    add("test", argfrmt, buf);

    for(int i = 0; i < argc-2; i++){
      free(buf[i]);
    }
    free(buf);
  }

  return 0;
}
