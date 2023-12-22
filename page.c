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

int phead_null(phead p){
  return p.tupct == -1 && p.ver == -1 && p.nfree == -1;
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

  phead p = {.tupct=0, .ver=VERSION, .nfree=PAGE_SIZE};
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
  if(phead_wrt(pname, &p) == 1){
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

int tuple_decode(char *pname, int t){
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

int tuple_remove(char *pname, int n){
  FILE *fp = fopen(pname, "r");
  if(fp == NULL){
    return 1;
  }

  phead p = phead_ret(pname);
  printf("file has %d tuples\n", p.tupct);
  if(n > p.tupct || n <= 0){
    fclose(fp);
    return 1;
  }
  
  thead t;
  fseek(fp, (n-1)*sizeof(thead)+sizeof(phead), SEEK_SET);
  fread(&t, 1, sizeof(thead), fp);

  printf("format: %s\nloc: %d\nsize: %d\n", t.fmt, t.loc, t.tsize); //FP is not at 0 at this point

  // thead lt;
  // if(n < p.tupct){
  //   fseek(fp, (p.tupct-1)*sizeof(thead)+sizeof(phead), SEEK_SET);
  //   fread(&lt, 1, sizeof(thead), fp);
  // }
  if(n == p.tupct){ //just delete
    fclose(fp);
    return 1;
  }
  else if(n <= p.tupct){ //delete then move tuples and data 
    fclose(fp);
    return 1;
  }


  fclose(fp);
  return 1;
}

int prev_main_logic(int argc, char **argv){
  if(argc == 1){  //DEFAULT TEST CASE
    //tuple_remove("test", 1);
    tuple_decode("test", 1);
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
  }
  else if(strcmp(argv[1], "decode") == 0){
    if(!(argc >= 4 && (atoi(argv[3]) != 0 || strcmp(argv[3], "0") == 0))){
      printf("usage: ./a.out decode [page name] [tuple]");
      return 1;
    }
    tuple_decode(argv[2], atoi(argv[3]));
  }

  return 0;
}

int create_db(char *dname, char *fmt){
  FILE *dir_fp;
  FILE *mtda_fp;

  int dmtd_len = strlen(dname)+strlen(".metadata")+1;
  int ddir_len = strlen(dname)+strlen(".dir")+1;
  char dname_meatadata[dmtd_len];
  char dname_dir[ddir_len];
  bzero(dname_meatadata, ddir_len);
  bzero(dname_dir, dmtd_len);

  strcat(dname_meatadata, dname);
  strcat(dname_meatadata, ".metadata");
  strcat(dname_dir, dname);
  strcat(dname_dir, ".dir");

  printf("dname metadata: %s\n", dname_meatadata);
  printf("dname dir: %s\n", dname_dir);

  if((dir_fp = fopen(dname_dir, "r")) != NULL){
    fclose(dir_fp);
    return 1;
  }

  printf("testing format\n");
  for(int i = 0; i < strlen(fmt); i++){
    if(!(fmt[i] == 's' || fmt[i] == 'l' || fmt[i] == 'f')){
      printf("failing character '%c' at %d\n", fmt[i], i);
      return 1;
    }
  }
  printf("done testing format\n");

  dir_fp = fopen(dname_dir, "w");
  mtda_fp = fopen(dname_meatadata, "w");

  if(!(dir_fp == NULL && mtda_fp == NULL) && dir_fp == NULL || mtda_fp == NULL){
    if(dir_fp == NULL){
      fclose(mtda_fp);
    }
    else{
      fclose(dir_fp);
    }
  }
  if(dir_fp == NULL && mtda_fp == NULL){
    return 1;
  }

  fwrite(fmt, sizeof(char), strlen(fmt), mtda_fp);

  fclose(dir_fp);
  fclose(mtda_fp);

  return 0;
}

int main(int argc, char **argv){
  create_db("testdname", "sslfs");
  return 1; 
  // prev_main_logic(argc, argv);
}
