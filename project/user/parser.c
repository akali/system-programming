#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define BUFFER_LENGTH 256                ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];   ///< The receive buffer from the LKM

int number_of_bits(char str[])
{
	int j=0;
	int k=0;

	while(str[j]!='\0')
	{
		k++;
		j++;
	}

	return k;
}
int str_to_int(char a[]) {
  int c, sign, offset, n;
 
  if (a[0] == '-') {  // Handle negative integers
	sign = -1;
  }
 
  if (sign == -1) {  // Set starting position to convert
	offset = 1;
  }
  else {
	offset = 0;
  }
 
  n = 0;
 
  for (c = offset; a[c] != '\0'; c++) {
	n = n * 10 + a[c] - '0';
  }
 
  if (sign == -1) {
	n = -n;
  }
 
  return n;
}

int isNumber(char str[]) {
	for (int i = 0; i < number_of_bits(str); ++i) {
		if (!isdigit(str[i])) return 0;
	}
	return 1;
}

int is_valid_cronstr(char str[]) {
	int init_size = strlen(str);
	char delim[] = " ";
	char *res_tokens[100];
	char *ptr = strtok(str, delim);
	int i = 1;
	
	
		res_tokens[0] = ptr;
	do
	{
		
		if (!ptr) break;
		ptr = strtok(NULL, delim);
		res_tokens[i] = ptr;
		i++;
	} while(ptr);
	
	int j=0;
	int valid_params=0;
	for (int j = 0;j<5; ++j)
	{
		if (strcmp(res_tokens[j],"*")==0)
		{
			
			valid_params++;
		}
		else {
		
			
			if (isNumber(res_tokens[j]))
			{
				
				if(j==0 && str_to_int(res_tokens[j])<60) {
					valid_params++;
				}
				else if (j==1 && str_to_int(res_tokens[j])<24)
				{
					valid_params++;
				}
				else if (j==2 && str_to_int(res_tokens[j])<32 && str_to_int(res_tokens[j])>0)
				{
					valid_params++;
				}
				else if (j==3 && str_to_int(res_tokens[j])<13 && str_to_int(res_tokens[j])>0)
				{
					valid_params++;
				}
				else if (j==4 && str_to_int(res_tokens[j])<8 && str_to_int(res_tokens[j])>-1)
				{
					valid_params++;
				}
			}
		}
		//printf("%s\n",res_tokens[j] );
	}
	if (valid_params==5) return 1;
	return 0;
}

int is_valid_filename(char filename[]){
	FILE * fp;
	char * buffer = NULL;
	size_t len = 0;
	ssize_t read;
	int num_of_valid_cronstrings=0;
	int num_of_lines=0;
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&buffer, &len, fp)) != -1) {
		   if(is_valid_cronstr(buffer)){
			num_of_valid_cronstrings++;
		   }
		   
			num_of_lines++;
	   
	  }

		fclose(fp);
  
	  if (buffer)
	  {
		free(buffer); 
	  }
		//printf("%d %d\n", num_of_valid_cronstrings, num_of_lines);
	  if( num_of_valid_cronstrings==num_of_lines)
	  {
		return 1;
	  }
	  return 0;
}

int main(int argc, char *argv[]) {

	int ret;
	int fd;
	int i;
	int j;
	int k=0;
	char stringToSend[BUFFER_LENGTH]="";

	fd = open("/dev/cronk_scd", O_RDWR);        

	if (fd < 0)
	{
		perror("Failed to open the device...");
		return errno;
	}
	
	for(i=1;i<argc;i++)
	{
		strcat(stringToSend,argv[i]);
		if(i!=argc-1)
		{ 
			strcat(stringToSend," ");
		}
	}

	int strlength = number_of_bits(stringToSend);

	if(strcpy(stringToSend,"crontab -l")==0)
	{
		ret = write(fd, stringToSend,strlength);
		if (ret < 0)
		{
			perror("Failed to write the message to the device.");
			return errno;
		}
	}
	else if(strcmp(stringToSend,"crontab -r")==0)
	{
		ret = write(fd, stringToSend,strlength);
		if (ret < 0)
		{
			perror("Failed to write the message to the device.");
			return errno;
		}
	}
	else if(strcmp(stringToSend,"crontab -e")==0)
	{
		ret = write(fd, stringToSend,strlength);
		if (ret < 0)
		{
			perror("Failed to write the message to the device.");
			return errno;
		}
	}
	else if(strcmp(stringToSend,"crontab -s")==0)
	{
		ret = write(fd, stringToSend,strlength);
		if (ret < 0)
		{
			perror("Failed to write the message to the device.");
			return errno;
		}
	}

	else if(strcmp(stringToSend,"crontab -v")==0)
	{
		ret = write(fd, argv[3], strlength);
		if (ret < 0)
		{
			perror("Failed to write the message to the device.");
			return errno;
		}
	}
	else if(strcmp(argv[1],"crontab")==0 &&  //скидываю название файла
			strcmp(argv[2],"-a")==0 &&
			strcmp(argv[3],"")!=0)
	{
		int length = number_of_bits(argv[3]);

		if(is_valid_filename(argv[3])==1)
		{
			char *path = realpath(argv[3], NULL);
			ret = write(fd, path, strlen(path));
			if (ret < 0)
			{
				perror("Failed to write the message to the device.");
				return errno;
			}
		}
	}
	else if(strcmp(argv[1],"crontab")==0 && //скидываю юзера
			strcmp(argv[2],"-u")==0 &&
			strcmp(argv[3],"")!=0)
	{
		int length = number_of_bits(argv[3]);


		ret = write(fd, argv[3], length);
		if (ret < 0)
		{
			perror("Failed to write the message to the device.");
			return errno;
		}
	}
	else{
		printf("Yhis string cannot be written to device [%s].\n", stringToSend);

	}

	// ret = read(fd, receive, BUFFER_LENGTH);      // Read the response from the Linux Kernel Module
	// if (ret < 0)
	// {
	// 	perror("Failed to read the message from the device.");
	// 	return errno;
	// }
	// printf("The received message is: [%s]\n", receive);
	// printf("End of the program\n");
	return 0;
}
