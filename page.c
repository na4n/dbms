#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const float VERSION = 0.01;
const int TUPLE_MAX = 10;
const int PAGE_SIZE = 4096;

struct __attribute__((packed)) page_header{
  int tupct;
  float ver;
  int nfreedat;
  int nfreetup;
} typedef phead;

struct __attribute__((packed)) tuple_header{
  unsigned char size;
  short loc;
} typedef thead;

// struct garbage_header{
//   unsigned char tuplen;
//   int foffset;
// }

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

int gbc_removed_tuple(char *pname, int n, int delsize){  
  FILE *fp;
  if((fp = fopen(pname, "r+")) == NULL){
    return 1;
  }

  phead p = phead_ret(pname);
  if(n > p.tupct || n <= 0){
    fclose(fp);
    printf("invalid tuple number\n");
    return 1;
  }

  if(n == p.tupct){
    printf("no change necessary");
    return 0;
  }
  
  int datend;
  int datstart = p.nfreedat;
  thead currth;
  fseek(fp, sizeof(phead)+sizeof(thead)*(n), SEEK_SET);
  fread(&currth, sizeof(thead), 1, fp);
  datend = currth.loc+currth.size;
  
  for(int i = 0; i < p.tupct-n; i++){ //
    fseek(fp, sizeof(phead)+sizeof(thead)*(n+i), SEEK_SET);
    fread(&currth, sizeof(thead), 1, fp);
    currth.loc += delsize;
    fseek(fp, sizeof(phead)+sizeof(thead)*(n+i-1), SEEK_SET);
    fwrite(&currth, sizeof(thead), 1, fp);
  }

  fseek(fp, sizeof(phead)+sizeof(thead)*(p.tupct-1), SEEK_SET);
  char b = '\0';
  for(int i = 0; i < sizeof(thead); i++){
    fwrite(&b, 1, 1, fp);
  }

  char buf[datend-datstart];
  fseek(fp, p.nfreedat, SEEK_SET);
  fread(buf, 1, datend-datstart, fp);
  
  fseek(fp, p.nfreedat+delsize, SEEK_SET);
  fwrite(buf, 1, datend-datstart, fp);
  
  fseek(fp, p.nfreedat, SEEK_SET);
  for(int i = 0; i < delsize; i++){
    fwrite(&b, 1, 1, fp);
  }

  fclose(fp);
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
  
  for(int i = 0; i < th.size; i++){
    if(fwrite(&b, 1, 1, fp) == 0){
      printf("tuple removal (null overwrite) is failing");
    }
  }

  fseek(fp, th_loc, SEEK_SET);
  for(int i = 0; i < sizeof(thead); i++){
    if(fwrite(&b, 1, 1, fp) == 0){
      printf("tuple removal (null overwrite) is failing");
    }
  }
  fclose(fp);

  gbc_removed_tuple(pname, n, th.size);
  p.tupct -= 1;
  p.nfreetup = sizeof(phead) + (p.tupct) * sizeof(thead);
  p.nfreedat += th.size;
  phead_wrt(pname, &p);

  return 0;
}

int debug_tuple(char *pname, int n, char *fmt){
  return 1;
}

int debug_page(char *pname){
  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return 1;
  }

  phead p = phead_ret(pname);
  printf("Page Header\n");
  printf("\tTuple Count: %d\n", p.tupct);
  printf("\tVersion: %f\n", p.ver);
  printf("\tNext Free Data Location: %d\n", p.nfreedat);
  printf("\tNext Free Tuple Location: %d\n", p.nfreetup);

  thead th;
  for(int i = 1; i <= p.tupct; i++){
    fseek(fp, sizeof(phead)+sizeof(thead)*(i-1), SEEK_SET);
    fread(&th, sizeof(thead), 1, fp);
    printf("Tuple %d\n", i);
    printf("\tLocation: %d\n", th.loc);
    printf("\tSize: %d\n", th.size);
    // fseek(fp, th.loc, SEEK_SET);
    // char buf[th.size];
    // fread(buf, 1, th.size, fp);
  }

  fclose(fp);
  return 0;
}

int main(int argc, char **argv){

  if(argc == 1){  //DEFAULT TEST CASE
    tuple_remove("test", 2);
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
    tuple_decode(argv[2], atoi(argv[3]), "s");    //decode formatter is set to a single string
  }
  else if(strcmp(argv[1], "debug") == 0){
    if(argc < 3){
      printf("Usage: ./a.out debug page_name");
      return 1;
    }
    debug_page(argv[2]);
  }

  return 0;
}
