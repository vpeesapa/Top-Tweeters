#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define MAX_FILE_LINES 20000

int numFields;
int numTweeters;
bool needsQuotes;

struct Tweeters {
	char* name;
	int count;
};

/*
 * Function that checks if the program has one additional command line argument
 * @numArgs: The number of command line arguments for the program
 */
void checkArgs(int numArgs)
{
	if(numArgs != 2) {
		printf("Invalid Input Format\n");
		exit(0);
	}
}

/*
 * Function that returns a list of all positions of commas in @fileHeader
 * @fileHeader: The string that needs to be traversed
 */
int* findCommaPositions(char* fileHeader)
{
	int* commaPositions = (int *)malloc(numFields * sizeof(int));
	int i;

	for(i = 0;fileHeader[i] != '\0';i++) {
		if(fileHeader[i] == ',') {
			numFields += 1;
			commaPositions = (int *)realloc(commaPositions,numFields * sizeof(int));
			commaPositions[numFields - 1] = i;
		}
	}

	return commaPositions;
}

/*
 * Function that allocates memory to create an array of strings
 */
char** create2DArray()
{
	int i;
	char** stringArray = (char **)malloc(MAX_LINE_LENGTH * sizeof(char *));

	for(i = 0;i < numFields + 1;i++) {
		stringArray[i] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
	}

	return stringArray;
}

/*
 * Function that deallocates memory allocated for @stringArray
 * @stringArray: The array of strings that needs to be deallocated
 */
void free2DArray(char** stringArray)
{
	int i;

	for(i = 0;i < numFields + 1;i++) {
		free(stringArray[i]);
	}
	free(stringArray);
}

/*
 * Function that returns the substring of a string in [start,end)
 * @string: The string whose substring needs to be returned
 * @start: The starting index of the substring within @string
 * @end: The ending index of the substring within @string
 */
char* getSubstring(char* string,int start,int end)
{
	int i;
	int substringIndex = 0;
	char* substring = (char *)malloc((end - start) * sizeof(char));

	for(i = start;i < end;i++) {
		substring[substringIndex++] = string[i];
	}

	return substring;
}

/*
 * Function to check if quotes are used correctly
 * @element: One cell in the file
 */
void validateQuotes(char* element) {
    int numQuoteChars = 0;
    int i;

    // count number of quote characters in element
    for(i = 0;element[i] != '\0';i++) {
        if(element[i] == '"') {
            numQuoteChars += 1;
        }
    }

    if(numQuoteChars % 2 != 0) {
        // If there is an odd number of quote characters
        printf("Invalid Input Format\n");
        exit(0);
    }
}

/*
 * Function that checks if cell has quotes
 * @element: One cell in the file
 */
bool hasQuotes(char* element) {
    if (!element) {
        printf("Invalid Input Format\n");
        exit(0);
    }

    return (element[0] == '"') && (element[strlen(element) - 1] == '"');
}

/* 
 * Function to handle valid quote usage of a given cell if needed
 * @element: One cell in the file
 */
void handleQuotes(char* element) {
    bool startIsQuote;
    bool endIsQuote;

    if (!element) {
        printf("Invalid Input Format\n");
        exit(0);
    }

    validateQuotes(element);

    startIsQuote = element[0] == '"';
    endIsQuote = element[strlen(element) - 1] == '"';

    if ((!startIsQuote && endIsQuote) || (startIsQuote && !endIsQuote)) {
        // If characters exist outside the most outer quotes
        printf("Invalid Input Format\n");
        exit(0);
    }

    if (hasQuotes(element) != needsQuotes) {
        printf("Invalid Input Format\n");
        exit(0);
    }
}

/*
 * Function that parses @fileHeader
 * @fileHeader: The header of the file
 */
char** parseHeader(char* fileHeader)
{
	int* commaPositions = findCommaPositions(fileHeader);
	char** headerColumns = create2DArray();
	int i;

	if(numFields != 0) {
		// Gets the first column
		strcpy(headerColumns[0],getSubstring(fileHeader,0,commaPositions[0]));

		// Gets every other column except for the last
		for(i = 1;i < numFields;i++) {
			strcpy(headerColumns[i],getSubstring(fileHeader,commaPositions[i - 1] + 1,commaPositions[i]));
		}

		// Gets the last column
		strcpy(headerColumns[numFields],getSubstring(fileHeader,commaPositions[numFields - 1] + 1,strlen(fileHeader)));
		headerColumns[numFields + 1] = '\0';
	} else {
		strcpy(headerColumns[0],getSubstring(fileHeader,0,strlen(fileHeader)));
		headerColumns[1] = '\0';
	}

	// Deallocates memory allocated for commaPositions
	free(commaPositions);

	return headerColumns;
}

/*
 * Function that searches for a column called name or "name" and returns its index
 * in @headerColumns.
 * @fp: The file that's being read
 */
int findName(FILE* fp,char** headerColumns)
{
	int nameIndex = -1;
	int i;

	for(i = 0;i < numFields + 1;i++) {
		if(strcmp(headerColumns[i],"name") == 0 || strcmp(headerColumns[i],"\"name\"") == 0) {
			if(nameIndex == -1) {
				nameIndex = i;
			} else {
				// If there is more than one column called name or "name"
				printf("Invalid Input Format\n");
				fclose(fp);
				exit(0);
			}
		}
	}

	if(nameIndex == -1) {
		// If there is no column called name or "name"
		printf("Invalid Input Format\n");
		fclose(fp);
		exit(0);
	}

	return nameIndex;
}

/*
 * Function that checks whether the number of fields remain consistent in the
 * remaining portion of the file
 * @fp: The file that's being read
 * @fileContents: The line that's been read from the file
 */
void getNumCommas(FILE* fp,char* fileContents)
{
	int numCommas = 0;
	int i;

	for(i = 0;fileContents[i] != '\0';i++) {
		if(fileContents[i] == ',') {
			numCommas += 1;
		}
	}

	if(numCommas != numFields) {
		// If the number of fields in the row does not match with the number of fields in the header
		printf("Invalid Input Format\n");
		fclose(fp);
		exit(0);
	}

}

/*
 * Function that gets the information of the tweeter from @fileContents
 */
char** getTweeterInfo(char* fileContents)
{
	char** tweeterInfo = create2DArray();
	int i;
	int infoIndex = 0;
	int startIndex = 0;
	char* info = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));

	// Gets all the information about the tweeter except the last column
	for(i = 0;fileContents[i] != '\0';i++) {
		if(fileContents[i] != ',') {
			info[infoIndex++] = fileContents[i];
		} else {
			info[infoIndex] = '\0';
			memcpy(tweeterInfo[startIndex++],info,infoIndex + 1);
			infoIndex = 0;
		}
	}
	// Gets the information stored in the last column
	info[infoIndex] = '\0';
	memcpy(tweeterInfo[startIndex++],info,infoIndex + 1);
	tweeterInfo[startIndex] = '\0';

	free(info);

	return tweeterInfo;
}

/*
 * Function that returns the index of the tweeter with @tweeterName
 * Returns -1 when there is no such tweeter
 */
int findTweeter(struct Tweeters* tweeters,char* tweeterName)
{
	int i;

	if(tweeters == NULL) {
		// If the list is empty
		return -1;
	}

	for(i = 0;i < numTweeters;i++) {
		if(strcmp(tweeters[i].name,tweeterName) == 0) {
			return i;
		}
	}

	return -1;
}

/*
 * Function that sorts the list of tweeters, @tweeters, in descending order
 */
void sort(struct Tweeters** tweeters)
{
	int i,j;
	struct Tweeters temp;

	for(i = 0;i < numTweeters - 1;i++) {
		for(j = 0;j < numTweeters - i - 1;j++) {
			if((*tweeters)[j].count < (*tweeters)[j + 1].count) {
				// Sort in descending order
				temp = (*tweeters)[j];
				(*tweeters)[j] = (*tweeters)[j + 1];
				(*tweeters)[j + 1] = temp;
			}
		}
	}
}

/*
 * Function that prints a string without its surrounding quotes
 * @str: the string to print without its quotes (assumes surrounding quotes)
 */
void printWithoutQuotes(char* str) {
    for (int i = 1; i < strlen(str) - 1; i++) {
        printf("%c", str[i]);
    }
}

/*
 * Function that prints the top tweeters found in the file
 */
void printTopTweeters(struct Tweeters* tweeters)
{
    int i;

    if(numTweeters >= 10) {
        for(i = 0;i < 10;i++) {
            if (needsQuotes) {
                printWithoutQuotes(tweeters[i].name);
            }
            else {
                printf("%s", tweeters[i].name);
            }
            printf(": %d\n",tweeters[i].count);
        }
    } else {
        for(i = 0;i < numTweeters;i++) {
            if (needsQuotes) {
                printWithoutQuotes(tweeters[i].name);
            }
            else {
                printf("%s", tweeters[i].name);
            }
            printf(": %d\n",tweeters[i].count);
        }
    }
}

/*
 * Function that parses the remaining part of the file @fp
 * @fp: The file that needs to be parsed
 * @nameIndex: The index corresponding to the name column
 */
void parseRemaining(FILE* fp,int nameIndex)
{
    struct Tweeters* tweeters = (struct Tweeters *)malloc(numTweeters * sizeof(struct Tweeters));
    char fileContents[MAX_LINE_LENGTH];
    char* tweeterName;
    char** tweeterInfo;
    int tweeterIndex;

    while(fgets(fileContents,MAX_LINE_LENGTH,fp)) {
        
        if(feof(fp)) {
        	fileContents[strlen(fileContents)] = '\0';	
        } else {
        	fileContents[strlen(fileContents) - 1] = '\0';
        }

        getNumCommas(fp,fileContents);
        tweeterInfo = getTweeterInfo(fileContents);
        tweeterName = (char *)malloc(sizeof(char));
        strcpy(tweeterName,tweeterInfo[nameIndex]);

        tweeterIndex = findTweeter(tweeters,tweeterName);
        if(tweeterIndex == -1) {
            handleQuotes(tweeterName);

            // Adding a new tweeter to the list
            numTweeters += 1;
            tweeters = (struct Tweeters *)realloc(tweeters,numTweeters * sizeof(struct Tweeters));
            tweeters[numTweeters - 1].name = (char *)malloc(sizeof(char));
            strcpy(tweeters[numTweeters - 1].name,tweeterName);
            tweeters[numTweeters - 1].count = 1;
        } else {
            // The tweeter was found in the list
            tweeters[tweeterIndex].count += 1;
        }

        free(tweeterName);
    }

    if(numTweeters != 0) {
    	sort(&tweeters);
	    printTopTweeters(tweeters);

	    free2DArray(tweeterInfo);
    }

    free(tweeters);
}

/*
 * Function that checks if the header @headerColumn has any duplicate columns in the
 * file @fp.
 */
void checkDuplicates(FILE* fp,char** headerColumn,int headerColumnIndex)
{
	int i;

	for(i = 0;i < numFields + 1;i++) {
		if(strcmp(headerColumn[i],headerColumn[headerColumnIndex]) == 0 && headerColumnIndex != i) {
			printf("Invalid Input Format\n");
			fclose(fp);
			exit(0);
		}
	}
}

/*
 * Function that opens and reads a file with name @fileName, if it exists
 * @fileName: Name of the file
 */
void readFile(char* fileName)
{
	FILE* fp;
	int fileSize,nameIndex,i;
	char fileHeader[MAX_LINE_LENGTH];
	char** headerColumns;

	// Opening the file
	fp = fopen(fileName,"r");

	if(fp == NULL) {
		// If the file doesn't exist
		printf("Invalid Input Format\n");
		exit(0);
	}

	fseek(fp,0,SEEK_END);
	fileSize = ftell(fp);
	if(fileSize == 0) {
		// If the file is empty
		printf("Invalid Input Format\n");
		fclose(fp);
		exit(0);
	}
	fseek(fp,0,SEEK_SET);

	// If the non-empty file exists, need to parse it
	fgets(fileHeader,MAX_LINE_LENGTH,fp);
	if(feof(fp)) {
		fileHeader[strlen(fileHeader)] = '\0';
	} else {
		fileHeader[strlen(fileHeader) - 1] = '\0';
	}

	headerColumns = parseHeader(fileHeader);
	nameIndex = findName(fp,headerColumns);
	for(i = 0;i < numFields + 1;i++) {
		checkDuplicates(fp,headerColumns,i);
	}
	needsQuotes = hasQuotes(headerColumns[nameIndex]);
	parseRemaining(fp,nameIndex);

	free2DArray(headerColumns);
	fclose(fp);
}

int main(int numArgs,char** args)
{
	checkArgs(numArgs);
	// Only gets to this part of the code if there are only two command line arguments
	// Opens and reads file, if exists
	readFile(args[1]);

	return 0;
}
