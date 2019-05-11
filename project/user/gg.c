#include <stdio.h>

#include <string.h>




#define BUFFER_LENGTH 256		         ///< The buffer length (crude but fine)
	  ///< The receive buffer from the LKM

int main(int argc, char *argv[]) {

	int ret, fd,i,j,k=0;
	char stringToSend[BUFFER_LENGTH];
	
for(i=1;i<argc;i++){
		j=0;
		 strcat(argv[i]," ");
		strcat(stringToSend,argv[i]);
		k++;
	}
	
	printf("Writing message to the device [%s].\n", stringToSend);
	
	return 0;
}
