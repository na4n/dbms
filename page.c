#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const float VERSION = 0.01;
const int TUPLE_MAX = 10;
const int PAGE_SIZE = 4096;

struct page_header{
  int tupct;
  float ver;
  int nfree;
} typedef phead;

struct tuple_header{
  char fmt[10];
  int loc;
  int tsize;
} typedef thead;

phead PAGE_HEAD_INITIALIZER = {.tupct=-1, .ver=-1, .nfree=-1};

phead head_ret(char *pname){
  phead p = PAGE_HEAD_INITIALIZER;    //TODO: Change this to NULL

  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return p;
  }

  fread(&p, sizeof(char), sizeof(phead), fp);
  fclose(fp);
  return p;
}

int head_wrt(char *pname, phead *p){
  FILE *fp;
  if((fp = fopen(pname, "r+")) == NULL){
    return 1;
  }

  fwrite(p, sizeof(phead), 1, fp);
  fclose(fp);
  return 0;
}

int page_init(char *pname){
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
  return head_wrt(pname, &p);
}

int add(char *pname, char *fmt, void **arg) { //CHANGE NAME
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

  phead p = head_ret(pname);
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
  if(head_wrt(pname, &p) == 1){ //TODO: perform all three writes or none (backup)
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

int decode(char *pname, int t){
  phead p = head_ret(pname);
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

  int bufloc = 0;
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

  return 0;
}

int removetuple(char *pname, int n){ //TODO: find loc, copy all above down until end, renumber tuple numbers, renumber db file
  FILE *fp = fopen(pname, "r");
  if(fp == NULL){
    return 1;
  }

  phead p;
  fread(&p, 1, sizeof(phead), fp);
  printf("file has %d tuples\n", p.tupct);
  if(n > p.tupct || n <= 0){
    fclose(fp);
    return 1;
  }
  thead t;
  fseek(fp, (n-1)*sizeof(thead)+sizeof(phead), SEEK_SET);
  fread(&t, 1, sizeof(thead), fp);

  printf("format: %s\nloc: %d\nsize: %d\n", t.fmt, t.loc, t.tsize); //FP is not at 0 at this point

  thead lt;
  // fseek(fp, )

  fclose(fp);
  return 1;
}

int main(int argc, char **argv){
  if(argc == 1){ //TODO: remove default test case
    removetuple("test", 1);
    return 0;
  }

  page_init("test");
  
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
  else if(strcmp(argv[1], "decode") == 0){
    if(!(argc >= 4 && (atoi(argv[3]) != 0 || strcmp(argv[3], "0") == 0))){
      printf("usage: ./a.out decode [page name] [tuple]");
      return 1;
    }
    decode(argv[2], atoi(argv[3]));
  }

  return 0;
}
