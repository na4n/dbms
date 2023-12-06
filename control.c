#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

char *currdb = NULL;

int alterdb(char *db, int option){
  FILE *fp;
  switch (option){
    case 0:
      fp = fopen(db, "r");
      currdb = db;
      break;
    case 1:
      fp = fopen(db, "w");
      break;
  } 
 
  if(fp == NULL){
    return 1;
  }

  fclose(fp);
  return 0;
}

int create(char *table){
  if(currdb == NULL){
    perror("no db open");
    return 1;  
  }

  FILE *fp;
  fp = fopen(currdb, "w+");
  fprintf(fp, "%s\n", table);
  fclose(fp);
  
  fp = fopen(table, "w+");
  fclose(fp);
  
  return 0;  
}

int main(int argc, char **argv){
  //-->parse to verify SQL command
  //handle I/O to create/write/delete table/to table --> handle db as folder? 
  //add efficiency logic for file storage and I/O operations
  //profit??

  alterdb("testdb", 0); 
  create("table"); 
}
