#include <stdio.h>
#include <stdlib.h>

int createtab(char *tname){
  FILE *fp = fopen(tname, "w+");
  if(fp != NULL){
    fclose(fp);
    return 0;
  }
  return 1;
}

int main(){
  //parse to verify SQL command
  //handle I/O to create/write/delete table/to table
  //add efficiency logic for file storage and I/O operations
  //profit??
}
