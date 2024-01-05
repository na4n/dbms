#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int intpow(int a, int b){ //for historical purposes
  if(b == 0){
    return 1;
  }
  for(int i = 1; i < b; i++){
    a *= a;
  }
  return a;
}

int prompt_number(){
  char a;

  int n = 0;  
  while((a = getchar()) != EOF){
    if(a == '\n'){
      break;
    }
    else if(a < '0' || a > '9'){
      errno = ENONET;
      perror("NaN");
      return -1;
    }

    n = n*10 + (a-48); 
  }

  if(n < 0){
    // errno = ENONET;
    perror("number too large");
    return -1;
  }
  return n;
}

// int data_parse(){
//   printf("")
//   while()


//   void **buf = (void **)malloc(sizeof(void*)*(argc - 2));
//     char argfrmt[argc-2];
//     argfrmt[0] = '\0';
//     for(int i = 2; i < argc; i++){
//       if(strcmp(argv[i], "0") != 0 && atof(argv[i]) != 0.0 && atof(argv[i]) != atol(argv[i])){
//         strcat(argfrmt, "f");
//         buf[i-2] = malloc(sizeof(double*));
//         *(double*)buf[i-2] = atof(argv[i]);
//       }
//       else if(strcmp(argv[i], "0") != 0 && atol(argv[i]) != 0){
//         strcat(argfrmt, "l");
//         buf[i-2] = malloc(sizeof(long int*));
//         *(long int*)buf[i-2] = atol(argv[i]);
//       }
//       else{
//         strcat(argfrmt, "s");
//         buf[i-2] = malloc(sizeof(char*));
//         *(char**)buf[i-2] = argv[i];
//       }
//     }
// }

int add_row(char *tname){
  return 1;
}


int main(){
  // int a = prompt_number();
  // if(a != -1){
  //   printf("The number of arguments requested was: %d\n", a);
  // }
  // else{
  //   printf("invalid, not a number\n");
  // }

  for(int i = 0; i < 134; i++){
    printf("%d: %s\n", i, strerror(i));
  }

  return 1;
}

