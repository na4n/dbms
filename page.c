#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "page.h"

const float VERSION = 0.01;
const int TUPLE_MAX = 10;

p_head* gethead(char *pname){
  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return NULL;
  }

  char buf[sizeof(p_head)];
  p_head *p = malloc(sizeof(p_head));
  fread(buf, sizeof(char), sizeof(p_head), fp);
  fclose(fp);
  memcpy(p, buf, sizeof(p_head));
  return p;
}

int init(char *pname){
  p_head p = {.tupct=0, .ver=VERSION, .nfree=4096};
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

int add(char *fmt, void **arg){
  int c = 10 - strlen(fmt);
  for(int i = 0 ; i < c; i++){
    strcat(fmt, "0");
  }

  //update header
  //  calculate next header pointer location
  //  calculate next insert location (add arg sizes)
  //  check if sufficient space
  //  insert if so and return 0, otherwise return 1

  for(int j = 0; j < strlen(fmt); j++){
    if(fmt[j] == 'f'){
      printf("%f\n", *(double*) arg[j]);
    }
    else if(fmt[j] == 'l'){
      printf("%ld\n", *(long*) arg[j]);
    }
    else if (fmt[j] == 's'){
      printf("%s\n", *(char**) arg[j]);
    }
  }
 
  printf("argument: %s\n", fmt);
  return 1;  
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

    add(argfrmt, buf);

    for(int i = 0; i < argc-2; i++){
      free(buf[i]);
    }
    free(buf);
  }

  return 0;
}
