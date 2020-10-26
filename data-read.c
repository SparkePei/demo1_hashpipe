#include <stdio.h>
#include <stdlib.h>
int main(){
	int data;
	FILE * fd = fopen("./demo1_file.txt", "r");
	int i = 0;
	while(1){
		if(fread(&data, sizeof(int), 1, fd) == 0){break;}
		printf("Data %d is: %d\n", i++, data);
	}
	
	return fclose(fd);
}
