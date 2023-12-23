#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const float VERSION = 0.01;
const int TUPLE_MAX = 10;
const int PAGE_SIZE = 4096;

struct page_header{
  int tupct;
  float ver;
  int nfreedat;
  int nfreetup;
} typedef phead;

struct tuple_header{
  unsigned char size;
  short loc;
} typedef thead;

phead PAGE_HEAD_INITIALIZER = {.tupct=-1, .ver=-1, .nfreedat=-1, .nfreetup=-1};

int phead_null(phead p){
  return p.tupct == -1 && p.ver == -1 && p.nfreedat == -1 && p.nfreetup == -1;
}

phead phead_ret(char *pname){
  phead p = PAGE_HEAD_INITIALIZER;
  
  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return p;
  }

  fread(&p, sizeof(char), sizeof(phead), fp);
  fclose(fp);
  return p;
}

int phead_wrt(char *pname, phead *p){
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

  phead p = {.tupct=0, .ver=VERSION, .nfreedat=PAGE_SIZE, .nfreetup=(int)sizeof(phead)};
  return phead_wrt(pname, &p);
}

int tuple_add(char *pname, char *fmt, void **arg) {
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

  phead p = phead_ret(pname);
  if(phead_null(p)){
    return 1;
  }

  int curr_thead_loc = p.nfreetup;

  p.tupct += 1;
  p.nfreedat -= l;
  p.nfreetup += sizeof(thead);

  if(p.nfreetup >= p.nfreedat){
    printf("page is full\n");
    return 2;
  }
  
  if(phead_wrt(pname, &p) == 1){
    return 1;
  }

  //write tuple head with offset
  FILE *fp;
  if((fp = fopen(pname, "r+")) == NULL){
    return 1;
  }
  
  thead t = {.loc=p.nfreedat, .size=l};

  fseek(fp, curr_thead_loc, SEEK_SET);
  fwrite(&t, sizeof(thead), 1, fp);
  
  //write tuple at offset
  fseek(fp, p.nfreedat, SEEK_SET);
  fwrite(buf, l, 1, fp);
  
  fclose(fp);
  free(buf);
  return 0;
}

int tuple_decode(char *pname, int t, char *fmt){
  phead p = phead_ret(pname);
  if(phead_null(p)){
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
  printf("tuple size: %d\ntuple location: %d\n\n", th.size, th.loc);

  char buf[th.size];
  fseek(fp, th.loc, SEEK_SET);
  fread(buf, th.size, 1, fp);

  int bufloc = 0;
  for(int i = 0; i < strlen(fmt); i++){
    if(fmt[i] == 'l'){
      long longch;
      memcpy(&longch, buf+bufloc, sizeof(long));
      bufloc += sizeof(long);
      printf("found a long in %s: %ld\n", fmt, longch);
    }
    else if(fmt[i] == 'f'){
      double floatch;
      memcpy(&floatch, buf+bufloc, sizeof(double));
      bufloc += sizeof(double);
      printf("found a double in %s: %f\n", fmt, floatch);
    }
    else if(fmt[i] == 's'){
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
      printf("found a string in %s: %s\n", fmt, strng);
      free(strng);
      continue;
    }
  }

  return 0;
}

int tuple_remove(char *pname, int n){
  FILE *fp = fopen(pname, "r+");
  if(fp == NULL){
    return 1;
  }

  phead p = phead_ret(pname);
  if(n > p.tupct || n <= 0){
    fclose(fp);
    printf("invalid tuple number\n");
    return 1;
  }

  int th_loc = sizeof(phead)+(n-1)*sizeof(thead);
  thead th;
  fseek(fp, th_loc, SEEK_SET);
  fread(&th, sizeof(thead), 1, fp);
  
  int t_loc = th.loc;
  fseek(fp, t_loc, SEEK_SET);
  char b = '\0';
  long s;
  
  for(int i = 0; i < th.size; i++){
    s = fwrite(&b, 1, 1, fp);
    if(s == 0){
      printf("something is going wrong\n");
    }
  }
  fseek(fp, th_loc, SEEK_SET);
  for(int i = 0; i < sizeof(thead); i++){
    s = fwrite(&b, 1, 1, fp);
    if(s == 0){
      printf("something is going wrong\n");
    }
  }
  fclose(fp);

  FILE *garbage = fopen(".garbage", "a+");
  fwrite(&n, sizeof(int), 1, garbage);
  fclose(garbage);

  return 0;
}

int main(int argc, char **argv){
  if(argc == 1){  //DEFAULT TEST CASE
    tuple_remove("test", 1);
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

    tuple_add("test", argfrmt, buf);

    for(int i = 0; i < argc-2; i++){
      free(buf[i]);
    }
    free(buf);

    tuple_decode("test", 1, argfrmt);
    tuple_decode("test", 2, argfrmt);
  }
  else if(strcmp(argv[1], "decode") == 0){
    if(!(argc >= 4 && (atoi(argv[3]) != 0 || strcmp(argv[3], "0") == 0))){
      printf("usage: ./a.out decode [page name] [tuple]");
      return 1;
    }
    tuple_decode(argv[2], atoi(argv[3]), "quick");
  }

  return 0;
}
