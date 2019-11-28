#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAISE_ERROR(msg)     \
        printf(msg);         \
        exit(1)

#define MAX_LINE_LENGTH 1024 + 1
#define MAX_FILE_LINES 20000
#define NAME "name"
#define QUOTED_NAME "\"name\""
/*TODO notes
   314: Only strip 1 layer of quotes
*/
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
Tweet* findTweet(Tweet* tweets, char* name, int numTweets);
int getNameColumnPosition(FILE* fp);
bool isDelimiter(char c, char* delimiters);
bool matches(char* str);
TweetData* parseFile(FILE* fp);
void printTop10(FILE* fp);
char** tokenize(char* line, char* delimiters);
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


Tweet* findTweet(Tweet* tweets, char* name, int numTweets) {
    for (int i = 0; i < numTweets; i++) {
        if (!strcmp(tweets[i].name, name)) {
            return &tweets[i];
        }
    }
    return NULL;
}


int getNameColumnPosition(FILE* fp) {
    char line[MAX_LINE_LENGTH];
    bool nameFound;
    int namePos;
    char** tokenizedLine;

    // get header
    fgets(line, MAX_LINE_LENGTH, fp);
    tokenizedLine = tokenize(line, "\n,");
    
    nameFound = false;
    for (int col = 0; tokenizedLine[col] != NULL; col++) {
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
    }
    free(tokenizedLine);

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
    TweetData* tweetData;
    Tweet* tweet;
    char** tokenizedLine;

    // dynamic allocation
    tweetData = (TweetData*)malloc(sizeof(TweetData));
    tweetData->tweets = (Tweet*)malloc(MAX_FILE_LINES * MAX_LINE_LENGTH * sizeof(Tweet));

    // get position of name column in the header row
    namePos = getNameColumnPosition(fp);

    // convert file data into Tweet structs
    tweetData->numTweets = 0;
    for (int i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
        tokenizedLine = tokenize(line, "\n,");
        tweet = findTweet(tweetData->tweets, tokenizedLine[namePos], tweetData->numTweets);
        if (tweet) {
            tweet->count++;
        } else {
            strcpy(tweetData->tweets[tweetData->numTweets].name, tokenizedLine[namePos]);
            tweetData->tweets[tweetData->numTweets].count = 1;
            tweetData->numTweets += 1;
        }
        if (i % 1000 == 0) {
        printf("%d\n", i);}
    }

    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        free(tokenizedLine[i]);
    }
    free(tokenizedLine);

    bubbleSort(tweetData->tweets, tweetData->numTweets);

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
    free(tweetData);
}


char** tokenize(char* line, char* delimiters) {
    char** tokens;
    int numElements;
    char* element;
    char* trimmedElement;
    int size; // represents current size of element

    // dynamic allocation
    tokens = (char**)malloc((MAX_LINE_LENGTH) * sizeof(char*));
    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        tokens[i] = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
    }
    element = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));

    // tokenize line by delimiters
    size = numElements = 0;
    for (int i = 0; line[i] != '\0'; i++) { // loop until end of line
        // check if char is a delimiter and save 1 byte for null terminator
        if (!isDelimiter(line[i], delimiters) && size < MAX_LINE_LENGTH - 1) {
            element[size++] = line[i];
        } else {
            element[size] = '\0'; // TODO is this needed?
            trimmedElement = trim(element);
            memcpy(tokens[numElements++], trimmedElement, size + 1);
            size = 0;
        }
    }

    tokens[numElements] = '\0';
    free(element);

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
        fclose(fp);
        RAISE_ERROR("4Invalid Input Format\n");
    }

    printTop10(fp);

    return 0;
}
