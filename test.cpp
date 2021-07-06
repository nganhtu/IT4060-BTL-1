/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
   char c;
   for(int i=0;i<266;i++){
      c = (char)i;
      printf("%d %c\n",c,c);
   }
   return(0);
}
*/
#include<iostream>
#include<stdio.h>
#include <string.h>
#include <string>
using namespace std;


/*append string at the end of Log file
return 1 if seccess and something else is fail*/
bool appendFile(const char file_path[],const char src[], int src_len) {
   FILE *fp;
   fp = fopen(file_path, "ab");
   if (fp == NULL) return false;
   fwrite(src, src_len, sizeof(char), fp);
   fclose(fp);
   return true;
}

int main() {

}