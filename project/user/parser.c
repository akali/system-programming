#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUFFER_LENGTH 256		         ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];	  ///< The receive buffer from the LKM


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

int is_valid_filename(char filename[]){
	FILE * fp;
    char * buffer = NULL;
    size_t len = 0;
    ssize_t read;
	int num_of_valid_cronstrings=0;
	int num_of_lines=0;
    fp = fopen("textfile1.txt", "r");
    if (fp == NULL)
    {
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&buffer, &len, fp)) != -1) 
    {
   	       if(strcmp(buffer,"crontab -r\n")==0)
            {
              num_of_valid_cronstrings++;
            }
            else if(strcmp(buffer,"crontab -l\n")==0)
            {
              num_of_valid_cronstrings++;
            }
            else if(strcmp(buffer,"crontab -v\n")==0)
            {
              num_of_valid_cronstrings++;
            }
            else if(strcmp(buffer,"crontab -e\n")==0)
            {
              num_of_valid_cronstrings++;
            }
            else if(strcmp(buffer,"crontab -i\n")==0)
            {
              num_of_valid_cronstrings++;
            }
            else if(strcmp(buffer,"crontab -s\n")==0)
            {
              num_of_valid_cronstrings++;
            }

            num_of_lines++;
            printf("%s",buffer );
        }

    	fclose(fp);
  
      if (buffer)
      {
        free(buffer); 
      }
		printf("%d %d\n", num_of_valid_cronstrings, num_of_lines);
      if( num_of_valid_cronstrings==num_of_lines)
      {
        return 1;
      }
      return 0;
}
int main(int argc, char *argv[])
 {

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
			ret = write(fd, argv[3], length);
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
	
	printf("Writing message to the device [%s].\n", stringToSend);


	getchar();
	printf("%s",stringToSend);
	
	ret = read(fd, receive, BUFFER_LENGTH);	     // Read the response from the LKM
	if (ret < 0)
	{
		perror("Failed to read the message from the device.");
		return errno;
	}
	printf("The received message is: [%s]\n", receive);
	printf("End of the program\n");
	return 0;
}
