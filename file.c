#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "file.h"

int indir(const char *dir, const char *fname, const unsigned char type){
  DIR *d;
  if((d = opendir(dir)) == NULL){
    perror("failed to open dir");
    return -1;
  }

  struct dirent *dp;
  while((dp = readdir(d)) != NULL){
    if(dp->d_type == type && strcmp(dp->d_name, fname) == 0){
      return 1;
    }
  }

  return 0;
}

