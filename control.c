#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>

#include <linux/limits.h>

DIR *currdb = NULL;
char root[PATH_MAX];
char cdb[PATH_MAX];

int closedb(){
  if(currdb != NULL){
    closedir(currdb);
    return 0;
  }

  return 1;
}

int indir(char *d, char *c){
  DIR *dir = opendir(d);
  struct dirent *dp;

  while((dp = readdir(dir)) != NULL){
    if(strcmp(dp->d_name, c) == 0){
      closedir(dir);
      return 1;
    }
  }

  closedir(dir);
  return 0;
}

int usedb(char *db){
  if(indir(".", db) == 0){
    if(mkdir(db, 0755) != 0){
      perror("could not make directory");
      return 1;
    }
  }

  if((currdb = opendir(db)) == NULL){
    perror("could not open directory");
    return 1;
  }  

  strcpy(cdb, db);
  return 0;
}

int create(char *table){
  if(currdb == NULL){
    perror("no db open");
    return -1;  
  }

  if(indir(cdb, table) == 1)
    return 2;

  char pathbuf[PATH_MAX];
  sprintf(pathbuf, "%s/%s", cdb, table);
  FILE *fp = fopen(pathbuf, "w");
  fclose(fp);

  return 0;  
}

int deletedb(char *db){
  if(currdb == NULL)
    return 1;
  
  struct dirent *dp;
  DIR *rootdirp = opendir(cdb);
  while((dp = readdir(rootdirp)) != NULL){
    if(dp->d_name[0] != '.' && dp->d_type == DT_DIR){
      if(strcmp(dp->d_name, db) == 0){
        char pbuf[PATH_MAX];
        sprintf(pbuf, "%s/%s", cdb, db);
        rmdir(pbuf);
        closedir(rootdirp);
        return 0;
      }
    }
  }

  closedir(rootdirp);
  return 1;
}

int lsdb(char *dir, int d){
  DIR *rdirp = opendir(dir);
  struct dirent *dp;
  while((dp = readdir(rdirp)) != NULL){
    if(dp->d_type == DT_DIR && dp->d_name[0] != '.'){
      for(int i = 0; i < d; i++){
        printf("\t");
      }
      printf("%s\n", dp->d_name);
      char p[PATH_MAX];
      sprintf(p, "%s/%s", dir, dp->d_name);
      lsdb(p, d+1);
    }
  }

  return 0;
}

int main(int argc, char **argv){
  getcwd(root, PATH_MAX);
  
  lsdb(root, 0);

  // if(usedb("testdb") == 1){
  //   fflush(stderr);
  //   return 1;
  // }
  
  // if(create("table") == -1){
  //   fflush(stderr);
  //   return 1;
  // }

  //deletedb("fake");
  
  
  closedb();

  /* HAS: make, create table, delete, close
     NEEDS: 
      store data --> column (table metadata for columns)
      copy commands
      interactive view
      better directory management
  */
}
