#include <stdio.h>
#include <stdlib.h>

int create(char *void){
  
}

int main(int argc, char **argv){
  if(argc <= 2){
    printf("Usage: ./db.c <database>\n");
    return 1;
  }
  FILE *fp;
  if((fp = fopen("db")) == NULL){
    printf("DB doesn't exist\n");
    return 1;
  }

  return 0;
}
