#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUFFER_LENGTH 256		         ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];	  ///< The receive buffer from the LKM


int number_of_bits(char str[]){
	int j=0;
	int k=0;

	while(str[j]!='\0'){
		k++;
		j++;
	}

	return k;
}
int main(int argc, char *argv[]) {

	int ret;
	int fd;
	int i;
	int j;
	int k=0;
	char stringToSend[BUFFER_LENGTH]="";

	fd = open("/dev/cronk_scd", O_RDWR);	    

	if (fd < 0){

		perror("Failed to open the device...");
		return errno;

	}
	
	for(i=1;i<argc;i++){
	

		strcat(stringToSend,argv[i]);
		if(i!=argc-1){ 
			strcat(stringToSend," ");
		}
		
	}
		ret = write(fd, stringToSend, string_length); 
	if (ret < 0){
		perror("Failed to write the message to the device.");
		return errno;
	}


	if(strcpy(stringToSend,"-l")==0){


	}
	else if(strcmp(stringToSend,"-r")==0){

	}
	else if(strcmp(stringToSend,"-e")==0){

	}

	else if(strcmp(stringToSend,"-v")==0){

	}
	
	int string_length = number_of_bits(stringToSend);
	printf("Writing message to the device [%s].\n", stringToSend);


	getchar();
	printf("%s",stringToSend);
	
	ret = read(fd, receive, BUFFER_LENGTH);	     // Read the response from the LKM
	if (ret < 0){
		perror("Failed to read the message from the device.");
		return errno;
	}
	printf("The received message is: [%s]\n", receive);
	printf("End of the program\n");
	return 0;
}
