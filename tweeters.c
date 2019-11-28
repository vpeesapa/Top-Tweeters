#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAISE_ERROR(msg)     \
        printf(msg);         \
        exit(1)

#define MAX_LINE_LENGTH 1024
#define MAX_FILE_LINES 20000
#define NAME "name"
#define QUOTED_NAME "\"name\""
/*TODO notes
   314: Only strip 1 layer of quotes
*/

typedef struct {
    char* name;
    int count;
} Tweet;

// FILE DECLARATIONS
Tweet** bubbleSort(Tweet** tweets, int numTweets);
Tweet* findTweet(Tweet** tweets, char* name);
int getNameColumnPosition(FILE* fp);
bool isDelimiter(char c, char* delimiters);
bool matches(char* str);
Tweet** parseFile(FILE* fp);
void printTop10(FILE* fp);
char** tokenize(char* line, char* delimiters);
Tweet toTweet(char** data);

Tweet* findTweet(Tweet** tweets, char* name) {
    for (int i = 0; tweets[i] != NULL; i++) {
        if (!strcmp(tweets[i]->name, name)) {
            return tweets[i];
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


Tweet** parseFile(FILE* fp) {
    char** fileData;
    char line[MAX_LINE_LENGTH];
    bool nameFound;
    int namePos;
    char* name;
    int numTweets;
    Tweet** tweets;
    Tweet* tweet;
    char** tokenizedLine;

    // dynamic allocation
    tweets = (Tweet**)malloc(MAX_FILE_LINES * MAX_LINE_LENGTH * sizeof(Tweet*));

    // get position of name column in the header row
    namePos = getNameColumnPosition(fp);

    // convert file data into Tweet structs
    numTweets = 0;
    for (int i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
        tokenizedLine = tokenize(line, "\n,");
        name = tokenizedLine[namePos];
        tweet = findTweet(tweets, name);
        if (tweet) {
            tweet->count++;
        } else {
            tweet = (Tweet*)malloc(sizeof(Tweet));
            tweet->name = name;
            tweet->count = 1;
            tweets[numTweets++] = tweet;
        }
        if (i % 1000 == 0) {
        printf("%d\n", i);}
    }

    return bubbleSort(tweets, numTweets);
}



// TODO NOTE: don't use strlen use memcpy, etc
void printTop10(FILE* fp) {
    Tweet** tweets = parseFile(fp);

    for (int i = 0; tweets[i] != NULL && i < 10; i++) {
        printf("%s: %d\n", tweets[i]->name, tweets[i]->count);
    }
    // TODO get position of tweeter column from header on line 1
    // TODO get rest of lines of fp, strtok for ',', check if tweeterCol
    // exists, then count into a hashtable
    // TODO bubbleSort and print
}


Tweet** bubbleSort(Tweet** tweets, int numTweets) {
    Tweet* tweetL;
    Tweet* tweetR;
    Tweet temp;

    if (numTweets < 1) {
        return tweets;
    }

    for (int i = 0; i < numTweets - 1; i++) {
        for (int j = 0; j < numTweets - i - 1; j++) {
            tweetL = tweets[j];
            tweetR = tweets[j + 1];

            if (tweetR->count > tweetL->count) {
                temp = *tweetL;
                *tweetL = *tweetR;
                *tweetR = temp;
            }
        }
    }
    return tweets;
}


char* trim(char* str) {
    char* ptr;

    // remove leading spaces
    while (*str == ' ' || *str == '\r') {
        str++;
    }

    // remove trailing spaces from end
    ptr = str;
    while (*ptr != '\0') { // find last character
        ptr++;
    }
    ptr--;
    while ((*ptr == ' ' || *ptr == '\r') && ptr != str) { // remove trailing spaces
        ptr--;
    }

    // null terminate
    *(ptr + 1) = '\0';

    return str;
}


char** tokenize(char* line, char* delimiters) {
    char** elements;
    int numElements;
    char* element;
    int size; // represents current size of element

    // dynamic allocation
    elements = (char**)malloc((MAX_LINE_LENGTH + 1) * sizeof(char*));
    for (int i = 0; i < MAX_LINE_LENGTH + 1; i++) {
        elements[i] = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
    }
    element = (char*)malloc((MAX_LINE_LENGTH + 1) * sizeof(char)); // + 1 for null terminator

    // tokenize line by delimiters
    size = numElements = 0;
        for (int i = 0; line[i] != '\0'; i++) { // loop until end of line
        // check if char is a delimiter and save 1 byte for null terminator
        if (!isDelimiter(line[i], delimiters) && size < MAX_LINE_LENGTH - 1) {
            element[size++] = line[i];
        } else {
            element[size] = '\0'; // TODO is this needed?
            element = trim(element);
            memcpy(elements[numElements++], element, size + 1);
            size = 0;
        }
    }

    elements[numElements] =  '\0';

    return elements;
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
