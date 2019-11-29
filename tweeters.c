#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAISE_ERROR(msg)     \
        printf(msg);         \
        exit(1)
#define MAX_LINE_CHARS 1024
#define MAX_LINE_LENGTH  MAX_LINE_CHARS + 2 // null terminator and newline/EOF
#define MAX_FILE_LINES 20000
//#define MAX_LINE_LENGTH 1024 + 1
//#define MAX_FILE_LINES 20000
#define NAME "name"
#define QUOTED_NAME "\"name\""
/*TODO notes
   311: "1,1" invalid
   314: Only strip 1 layer of quotes
*/
static int mallocs = 0;
static int frees = 0;

typedef struct {
    char name[MAX_LINE_LENGTH];
    int count;
} Tweet;

typedef struct {
    int numTweets;
    Tweet* tweets;
} TweetData;

// FILE DECLARATIONS
void bubbleSort(Tweet* tweets, int numTweets);
int countNumCols(char* header); 
Tweet* findTweet(Tweet* tweets, char* name, int numTweets);
int getNameColumnPosition(char* line, int numCols);
bool isDelimiter(char c, char* delimiters);
bool matches(char* str);
TweetData* parseFile(FILE* fp);
void printTop10(FILE* fp);
char** tokenize(char* line, char* delimiters, int numCols);
Tweet toTweet(char** data);
char* trim(char* str);

void bubbleSort(Tweet* tweets, int numTweets) {
    Tweet* tweetL;
    Tweet* tweetR;
    Tweet temp;

    if (numTweets < 1) {
        return;
    }

    for (int i = 0; i < numTweets - 1; i++) {
        for (int j = 0; j < numTweets - i - 1; j++) {
            tweetL = &tweets[j];
            tweetR = &tweets[j + 1];

            if (tweetR->count > tweetL->count) {
                temp = *tweetL;
                *tweetL = *tweetR;
                *tweetR = temp;
            }
        }
    }
}


int countNumCols(char* header) {
    int numCols;

    if (!header) {
       RAISE_ERROR("6Invalid File Format\n"); 
    }

    numCols = 0;
    for (int i = 0; header[i] != '\0'; i++) {
        if (header[i] == ',') {
            numCols++;
        }
    }

    return numCols + 1;
}

Tweet* findTweet(Tweet* tweets, char* name, int numTweets) {
    for (int i = 0; i < numTweets; i++) {
        if (!strcmp(tweets[i].name, name)) {
            return &tweets[i];
        }
    }
    return NULL;
}


int getNameColumnPosition(char* header, int numCols) {
    bool nameFound;
    int namePos;
    char** tokenizedLine;

    tokenizedLine = tokenize(header, "\n,", numCols);
    
    nameFound = false;
    for (int col = 0; col < numCols; col++) {
        // check for duplicate name columns
        if (matches(tokenizedLine[col]) && nameFound) {
            RAISE_ERROR("1Invalid Input Format\n");
        }

        if (matches(tokenizedLine[col])) {
            namePos = col;
            nameFound= true;
        }
    }

    if (!nameFound) {
        RAISE_ERROR("2Invalid Input Format\n");
    }

    // deallocate dynamic memory
    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        free(tokenizedLine[i]);
//printf("mallocs: %d frees: %d FREE:tokenizedLine %d %p\n",mallocs, ++frees, i, tokenizedLine[i]);
    }
    free(tokenizedLine);
//printf("mallocs: %d frees: %d FREE: tokenizedLine %p\n",mallocs, ++frees, tokenizedLine);
//printf("EXITING getNameColumnPosition\n");
    return namePos;
}


bool isDelimiter(char c, char* delimiters) {
    for (int i = 0; delimiters[i] != '\0'; i++) {
        if (c == delimiters[i]) {
            return true;
        }
    }
    return false;
}


bool matches(char* str) {
    return !strcmp(str, NAME) || !strcmp(str, QUOTED_NAME);
}


TweetData* parseFile(FILE* fp) {
    char line[MAX_LINE_LENGTH];
    int namePos;
    char name[MAX_LINE_LENGTH];
    int numCols;
    TweetData* tweetData;
    Tweet* tweet;
    char** tokenizedLine;

    // dynamic allocation
    tweetData = (TweetData*)malloc(sizeof(TweetData));
//printf("mallocs: %d frees: %d MALLOC: tweetData %p\n",++mallocs, frees, tweetData);
    tweetData->tweets = (Tweet*)malloc(MAX_FILE_LINES * MAX_LINE_LENGTH * sizeof(Tweet));
//printf("mallocs: %d frees: %d MALLOC: tweetData->tweets %p\n", ++mallocs, frees, tweetData->tweets);

    // get position of name column in the header row
    fgets(line, MAX_LINE_LENGTH, fp);               // get header row
    numCols = countNumCols(line);                   // count number of columns
    namePos = getNameColumnPosition(line, numCols); // get pos of name column

    // convert file data into Tweet structs
    tweetData->numTweets = 0;
    for (int i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
        tokenizedLine = tokenize(line, "\n,", numCols);
        strcpy(name, tokenizedLine[namePos]);
    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        free(tokenizedLine[i]);
//printf("mallocs: %d frees: %d FREE:tokenizedLine[i] %p\n",mallocs, ++frees,tokenizedLine[i]);
    }
    free(tokenizedLine);
//printf("mallocs: %d frees:%d FREE:tokenizedLine %p\n",mallocs, ++frees,tokenizedLine);
        
        tweet = findTweet(tweetData->tweets, name, tweetData->numTweets);
        if (tweet) {
            tweet->count++;
        } else {
            strcpy(tweetData->tweets[tweetData->numTweets].name, name);
            tweetData->tweets[tweetData->numTweets].count = 1;
            tweetData->numTweets += 1;
        }
        if (i % 1000 == 0) {
        printf("%d\n", i);}
    }


    bubbleSort(tweetData->tweets, tweetData->numTweets);
//printf("EXITING parseFile\n");
    return tweetData;
}


void printTop10(FILE* fp) {
    TweetData* tweetData = parseFile(fp);

    for (int i = 0; i < tweetData->numTweets; i++) {
        if (i < 10) {
            printf("%s: %d\n", tweetData->tweets[i].name, tweetData->tweets[i].count);
        }
    }

    free(tweetData->tweets);
//printf("mallocs: %d frees: %d FREE:tweetData->tweets %p\n",mallocs, ++frees, tweetData->tweets);
    free(tweetData);
//printf("mallocs: %d frees: %d FREE:tweetData %p\n",mallocs, ++frees, tweetData);
}



char** tokenize(char* line, char* delimiters, int numCols) {
    char** tokens;
    int numTokens;
    char* token;
    char* trimmedElement;
    int size; // represents current size of token

    // dynamic allocation
    tokens = (char**)malloc((MAX_LINE_LENGTH) * sizeof(char*));
//printf("mallocs: %d frees: %d MALLOC: tokens %p\n",++mallocs, frees, tokens);
    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        tokens[i] = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
//printf("mallocs: %d frees: %d MALLOC: i: %d %p\n",++mallocs, frees, i,tokens[i]);
    }
    token = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
//printf("mallocs: %d frees: %d MALLOC: token: %p\n",++mallocs, frees, token);

    // tokenize line by delimiters
    size = numTokens = 0;
    for (int i = 0; line[i] != '\0'; i++) { // loop until end of line
        // check if char is a delimiter and save 1 byte for null terminator
        if (!isDelimiter(line[i], delimiters) && size < MAX_LINE_LENGTH - 1) {
            token[size++] = line[i];
        } else {
            token[size] = '\0'; // TODO is this needed?
            trimmedElement = trim(token);
            memcpy(tokens[numTokens++], trimmedElement, size + 1);
            size = 0;
        }
    }

    if (numTokens != numCols) {
        RAISE_ERROR("7Invalid File Format\n");
    }
//   tokens[numTokens] = '\0';
    free(token);
//printf("mallocs: %d frees: %d FREE:token %p\n",mallocs, ++frees, token);
//printf("EXITING tokenize\n");

    return tokens;
}


char* trim(char* str) {
    char* end;

    // trim leading spaces
    while (*str == ' ' || *str == '\r') str++;

    // trim trailing spaces
    end = str + strlen(str) - 1;
    
    while (end >= str && (*end == ' ' || *end == '\r')) end--;

    // null terminate
    end[1] = '\0';

    return str;
}


int main(int argc, char **argv) {
    FILE* fp;

    // check command line arguments
    if (argc != 2) {
        RAISE_ERROR("3Invalid Input Format\n");
    }
   
    // check if can open fp
    if ((fp = fopen(argv[1], "r")) == NULL) {
        RAISE_ERROR("4Invalid Input Format\n");
    }

    printTop10(fp);

    fclose(fp);
    return 0;
}
