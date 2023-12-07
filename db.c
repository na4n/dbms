#include <stdio.h>
#include <stdlib.h>

#include "file.h"

int create(char *name){
  FILE *fp;
  if((fp = fopen(name, "w")) == NULL){
    return 1;
  }

  fclose(fp);
  return 0;
}

int parheader(char *pname){
  FILE *fp;
  if((fp = fopen(pname, "r")) == NULL){
    return 1;
  }

}

int main(int argc, char **argv){
  if(argc < 2){
    printf("Usage: ./db.c <database>\n");
    printf("\t- subsequent arguments will be ignored\n");
    return 1;
  }

  if(indir(".", argv[1], DT_REG) == 0){
    if(create(argv[1]) == 1){
      perror("could not find db and could not make");
      return 1;
    }
  }
    
  return 0;
}
