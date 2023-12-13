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
  p.nfree -= l;

  int nexthead_end = sizeof(phead) + ((p.tupct) * (sizeof(thead)));
  int nexttuple_end = p.nfree;

  if(nexthead_end >= nexttuple_end){
    printf("terminating condition is met\n");
    return 2;
  }

  printf("adding %s ", fmt);
  printf("with size %d\n", l);

  //write new head
  if(hdwrite(pname, &p) == 1){ //MEGA PROBLEM: all three writes must happen or none happen
    return 1;
  }

  //write tuple head with offset
  FILE *fp;
  if((fp = fopen(pname, "r+")) == NULL){
    return 1;
  }

  thead t = {.fmt="", .loc=p.nfree, .tsize=l};
  strcpy(t.fmt, fmt);
  fseek(fp, nexthead_end-sizeof(thead), SEEK_SET);
  fwrite(&t, sizeof(thead), 1, fp);

  //write tuple at offset
  fseek(fp, p.nfree, SEEK_SET);
  fwrite(buf, l, 1, fp);

  fclose(fp);
  
  free(buf);
  return 0;
}

int decode(char *pname, int t){ //returns tuple t in page pname
  phead p = gethead(pname);
  if(p.tupct == -1 || p.ver == -1 || p.nfree == -1){
    return 1;
  }
  
  if(t > p.tupct || t <= 0){
    return 1;
  }

  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return 1;
  }

  int theadloc = sizeof(phead) + ((t-1) * sizeof(thead));
  fseek(fp, theadloc, SEEK_SET);

  thead th;
  fread(&th, sizeof(thead), 1, fp);
  printf("format: %s\nlocation: %d\ntsize: %d\n\n", th.fmt, th.loc, th.tsize);

  char buf[th.tsize];
  fseek(fp, th.loc, SEEK_SET);
  fread(buf, th.tsize, 1, fp);

  // char out[th.tsize+3];
  // strcat(out, "{");

  int bufloc = 0;
  // int outloc = 1;
  for(int i = 0; i < strlen(th.fmt); i++){
    if(th.fmt[i] == 'l'){
      long longch;
      memcpy(&longch, buf+bufloc, sizeof(long));
      bufloc += sizeof(long);
      printf("found a long in %s: %ld\n", th.fmt, longch);
    }
    else if(th.fmt[i] == 'f'){
      double floatch;
      memcpy(&floatch, buf+bufloc, sizeof(double));
      bufloc += sizeof(double);
      printf("found a double in %s: %f\n", th.fmt, floatch);
    }
    else if(th.fmt[i] == 's'){
      char *strng = NULL;
      int j = 0;
      while(1){
        if(*(buf+bufloc) != '\0'){
          strng = realloc(strng, j+1);
          strng[j] = *(buf+bufloc);
          j++;
          bufloc++;
        }
        else{
          strng = realloc(strng, j+1);
          strng[j] = *(buf+bufloc);
          j++;
          bufloc++;
          break;
        }
      }
      printf("found a string in %s: %s\n", th.fmt, strng);
      free(strng);
      continue;
    }
  }
  strcat(buf, "}");

  return 0;
}

int remove(char *pname, int t){
  FILE *fp;
  if(fp = fopen(pname, "r") == NULL){
    return 1;
  }
  phead p;
  fread(&p, 1, sizeof(phead), fp);

  if(t > p.tupct || t < 0){
    return 1;
  }
  
  int thead_loc = sizeof(phead) + (t-1) * sizeof(thead);
  thead t;
  fseek(fp, thead_loc, SEEK_SET);
  fread(&t, 1, sizeof(thead), fp);

  //fseek(fp, t.loc, SEEK_SET);   //delete data, rewrite others, update tuple pointers in database
  // char buf[]

  return 1;
}

int garbage_collect(char *pname){

}

int main(int argc, char **argv){
  if(argc == 1){
    decode("test", 2);
    //decode("test", 2);
    return 0;
  }

  init("test");
  
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
