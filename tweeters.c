#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Function that checks if the program has one additional command line argument
 * @numArgs: The number of command line arguments for the program
 */
void checkArgs(int numArgs)
{

	if(numArgs != 2) {
		printf("Invalid number of arguments.\n");
		exit(0);
	}

}

int main(int numArgs,char** args)
{
	char* fileName = (char *)malloc(sizeof(char));
	
	checkArgs(numArgs);

	// Only gets to this part of the code if there are only two command line arguments
	strcpy(fileName,args[1]);
	// FIXME: Omit following line
	printf("The name of the file is: %s\n",fileName);

	return 0;
}