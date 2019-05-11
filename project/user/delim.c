#include <stdio.h>
#include <string.h>
#include <ctype.h>
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
int isNumber(char str[]){
	for (int i = 0; i < number_of_bits(str); ++i)
	{
		if (!isdigit(str[i])) return 0;
	}
	return 1;
}
int is_valid_cronstr(char str[])
{
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
int main()
{
	
		char lol[]="1 * 3 4 *";
		if(is_valid_cronstr(lol)) printf("cool\n");
		else printf("not cool\n");
	

	return 0;
}