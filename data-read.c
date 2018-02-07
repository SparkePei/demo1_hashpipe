#include <stdio.h>
#include <stdlib.h>
main(){
FILE * file_t;
int * data;
data = (int *)malloc(100*sizeof(int));
file_t=fopen("./demo_file.txt","r");
int i;
i=0;
while(1){
fread(data,sizeof(int),1,file_t);
if(*data==0){break;}
printf("Data %d is: %d\n",i++,*data);
data++;
}
close(file_t);
}
