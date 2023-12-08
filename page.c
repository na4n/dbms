#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "page.h"

const float VERSION = 0.01;
const int TUPLE_MAX = 10;
const int PAGE_SIZE = 4096;

phead gethead(char *pname){
  phead p = PAGE_HEAD_INITIALIZER;

  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return p;
  }

  fread(&p, sizeof(char), sizeof(phead), fp);
  fclose(fp);
  return p;
}

int hdwrite(char *pname, phead *p){
  FILE *fp;
  if((fp = fopen(pname, "r+")) == NULL){
    return 1;
  }

  fwrite(p, sizeof(phead), 1, fp);
  fclose(fp);
  return 0;
}

int init(char *pname){
  FILE *fp;
  
  if((fp = fopen(pname, "r")) != NULL){
    fclose(fp);
    return 2;
  }

  if((fp = fopen(pname, "w+")) == NULL){
    printf("failed to open\n");
    return 1;
  }
  fclose(fp);

  phead p = {.tupct=0, .ver=VERSION, .nfree=PAGE_SIZE};
  return hdwrite(pname, &p);
}

  //update header
  //  calculate next header pointer location
  //  calculate next insert location (add arg sizes)
  //  check if sufficient space
  //  insert if so and return 0, otherwise return 1

int add(char *pname, char *fmt, void **arg) {
  void *buf = NULL;
  int l = 0;
  int asize;

  for (int j = 0; j < strlen(fmt); j++) {
    if(fmt[j] == 'f'){
      asize = sizeof(double);
      buf = realloc(buf, l + asize);
      memcpy(buf + l, (double *)arg[j], asize);
      l += asize;
    } 
    else if(fmt[j] == 'l'){
      asize = sizeof(long);
      buf = realloc(buf, l + asize);
      memcpy(buf + l, (long *)arg[j], asize);
      l += asize;
    }
    else if(fmt[j] == 's'){
      asize = strlen(*(char **)arg[j]) + 1;
      buf = realloc(buf, l + asize);
      memcpy(buf + l, *(char **)arg[j], asize);
      l += asize;
    }
  }

  phead p = gethead(pname);
  p.tupct += 1;
  p.nfree -= asize;

  int nexthead_end = sizeof(phead) + ((p.tupct) * (sizeof(thead)));
  int nexttuple_end = p.nfree;
  if(nexthead_end >= nexttuple_end){
    return 2;
  }

  //write new head                                                //MEGA PROBLEM: all three writes must happen or none happen
  if(hdwrite(pname, &p) == 1){
    return 1;
  }

  //write tuple head with offset
  FILE *fp;
  if((fp = fopen(pname, "r+")) == NULL){
    return 1;
  }

  thead t = {.fmt=*fmt, .loc=p.nfree};
  fseek(fp, nexthead_end-sizeof(thead), SEEK_SET);
  fwrite(&t, sizeof(thead), 1, fp);

  //write tuple at offset
  fseek(fp, p.nfree, SEEK_SET);
  fwrite(buf, sizeof(buf), 1, fp);

  fclose(fp);
  
  free(buf);
  return 0;
}

int main(int argc, char **argv){
  if(argc == 1)
    return 0;

  if(init("test") == 2)
    printf("specified db already exists\n");

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
