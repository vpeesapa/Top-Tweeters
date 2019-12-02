#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
   319: Check line lengths and # of lines
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
void CHECK_HAS_COMMA(char* token);
void CHECK_LENGTH(char* line);
void HANDLE_QUOTES(char* token);
bool IS_EMPTY_LINE(char* line);
void RAISE_ERROR(char msg[]);
int comparator(const void *p, const void *q); 
int countNumCols(char* header); 
Tweet* findTweet(Tweet* tweets, char* name, int numTweets);
void free2DArray(char** arr, int size);
int getNameColumnPosition(char* line, int numCols, bool isLastLine);
bool hasQuotes(char* str);
bool isDelimiter(char c, char* delimiters);
TweetData* parseFile(FILE* fp);
void printTop10(FILE* fp);
void stripQuotes(char* str); 
char** tokenize(char* line, char* delimiters, int numCols, bool isLastLine);
TweetData* toTweetData(FILE* fp, int numCols, int namePos);
//char* trim(char* str);

bool needQuotes = false;

void CHECK_HAS_COMMA(char* token) {
    while (*token != '\0') {
        if (*token == ',') {
            RAISE_ERROR("0Invalid File Format\n");
        }
        token++;
    }
}


void CHECK_LENGTH(char* line) {
    if (!line || strlen(line) > MAX_LINE_LENGTH) {
        RAISE_ERROR("1Invalid File Format\n");
    }
}


void HANDLE_QUOTES(char* token) {
    int numQuoteChars = 0;
    int i;

    if (!token) {
        printf("Invalid File Format\n");
        exit(1);
    }

    // count number of quote characters in token
    for(i = 0; token[i] != '\0'; i++) {
        if(token[i] == '"') {
            numQuoteChars += 1;
        }
    }

    if(numQuoteChars % 2 != 0) {
        // If there is an odd number of quote characters
        printf("Invalid Input Format\n");
        exit(0);
    }

    if (needQuotes != hasQuotes(token)) {
        RAISE_ERROR("1AAInvalid File Format\n");
    }

    if (needQuotes) {
        stripQuotes(token);
    }
}


bool IS_EMPTY_LINE(char* line) {
    return strlen(line) == 1 && line[0] == '\n';
}


bool hasQuotes(char* str) {
    bool endIsQuote;
    int length;
    bool startIsQuote;

    length = strlen(str);

    if (length < 2) {
        return false;
    }

    startIsQuote = str[0] == '"';
    endIsQuote = str[length - 1] == '"';

    if ((!startIsQuote && endIsQuote) ||
      (startIsQuote && !endIsQuote)) {
        RAISE_ERROR("1AInvalid File Format\n");
    }

    return startIsQuote && endIsQuote;
}


void RAISE_ERROR(char msg[]) {
    printf("%s", msg);
    exit(1);
}


int comparator(const void *p, const void *q) 
{
    // Get the values at given addresses 
    const Tweet* l = (const Tweet*) p;
    const Tweet* r = (const Tweet*) q;

    return l->count < r->count;
} 


int countNumCols(char* header) {
    int numCols;

    if (!header) {
       RAISE_ERROR("2Invalid File Format\n"); 
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


void free2DArray(char** arr, int size) {
    for (int i = 0; i < size; i++) {
        free(arr[i]);
    }
    free(arr);
}


int getNameColumnPosition(char* header, int numCols, bool isLastLine) {
    bool nameFound;
    int namePos;
    char** tokenizedLine;

    tokenizedLine = tokenize(header, "\n,", numCols, isLastLine);
    
    nameFound = false;
    for (int col = 0; col < numCols; col++) {
        // check for duplicate name columns
        if (nameFound && (!strcmp(tokenizedLine[col], NAME) ||
!strcmp(tokenizedLine[col], QUOTED_NAME))) {
            RAISE_ERROR("2AInvalid File Format\n");
        }

        if (!strcmp(tokenizedLine[col], NAME)) {
            needQuotes = false;
            namePos = col;
            nameFound= true;
        }

        if (!strcmp(tokenizedLine[col], QUOTED_NAME)) {
            needQuotes = true;
            stripQuotes(tokenizedLine[col]);
            namePos = col;
            nameFound = true;
        }
    }

    if (!nameFound) {
        RAISE_ERROR("4Invalid Input Format\n");
    }

    // deallocate dynamic memory
    free2DArray(tokenizedLine, MAX_LINE_LENGTH);

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


TweetData* parseFile(FILE* fp) {
    bool isLastLine;
    char line[MAX_LINE_LENGTH];
    int namePos;
    int numCols;
    TweetData* tweetData;

    // get position of name column in the header row
    fgets(line, MAX_LINE_LENGTH, fp);               // get header row

    // check if empty first row
    if (IS_EMPTY_LINE(line)) {
        printf("Invalid File Format");
        exit(1);
    }

    CHECK_LENGTH(line);
    isLastLine = feof(fp);
    numCols = countNumCols(line);                   // count number of columns
    namePos = getNameColumnPosition(line, numCols, isLastLine); // get pos of name column

    // read file into tweetData struct
    tweetData = toTweetData(fp, numCols, namePos);

    if (tweetData->numTweets < 2) {
        return tweetData;
    }

    qsort(tweetData->tweets, tweetData->numTweets, sizeof(tweetData->tweets[0]),
comparator);

    return tweetData;
}


void printTop10(FILE* fp) {
    TweetData* tweetData = parseFile(fp);

    for (int i = 0; i < tweetData->numTweets; i++) {
        if (i < 10) {
            printf("%s: %d\n", tweetData->tweets[i].name,
tweetData->tweets[i].count);
        }
    }

    free(tweetData->tweets);
    free(tweetData);
}


void stripQuotes(char* str) {
    int length;

    length = strlen(str);

    for (int i = 0; i < length; i++) {
        str[i] = str[i + 1];
    }
    str[length - 2] = str[length - 1];
}


TweetData* toTweetData(FILE* fp, int numCols, int namePos) {
    bool isLastLine;
    char line[MAX_LINE_LENGTH];
    char name[MAX_LINE_LENGTH];
    char** tokenizedLine;
    Tweet* tweet;
    TweetData* tweetData;

    // dynamic allocation
    tweetData = (TweetData*)malloc(sizeof(TweetData));
    tweetData->tweets = (Tweet*)malloc(MAX_FILE_LINES * MAX_LINE_LENGTH * sizeof(Tweet));

    tweetData->numTweets = 0;
    for (int i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
        if (IS_EMPTY_LINE(line)) {
            continue;
        }
        CHECK_LENGTH(line);
        isLastLine = feof(fp);
        tokenizedLine = tokenize(line, "\n,", numCols, isLastLine);
        HANDLE_QUOTES(tokenizedLine[namePos]);
        strcpy(name, tokenizedLine[namePos]);
        free2DArray(tokenizedLine, MAX_LINE_LENGTH);

        tweet = findTweet(tweetData->tweets, name, tweetData->numTweets);
        if (tweet) {
            tweet->count++;
        } else {
            strcpy(tweetData->tweets[tweetData->numTweets].name, name);
            tweetData->tweets[tweetData->numTweets].count = 1;
            tweetData->numTweets += 1;
        }
    }

    return tweetData;
}


char** tokenize(char* line, char* delimiters, int numCols, bool isLastLine) {
    char** tokens;
    int numTokens;
    char* token;
    int size; // represents current size of token
    int i;

    if (strlen(line) == 1) {
        return NULL;
    }

    // dynamic allocation
    tokens = (char**)malloc((MAX_LINE_LENGTH) * sizeof(char*));

    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        tokens[i] = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
    }
    token = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));

    // tokenize line by delimiters
    size = numTokens = 0;
    for (i = 0; line[i] != '\0'; i++) { // loop until end of line
        if (!isDelimiter(line[i], delimiters) && size < MAX_LINE_LENGTH - 1) {
            // terminate at delimiters and add to token array
            token[size++] = line[i];
        } else {
            // found delimiter, add this token to tokens array
            token[size] = '\0';
            CHECK_HAS_COMMA(token);
            memcpy(tokens[numTokens++], token, size + 1);
            size = 0;
        }
    }

    // this is the last line of the file, read last token
    if (isLastLine && strlen(token) > 0) {
        token[size] = '\0';
        CHECK_HAS_COMMA(token);
        memcpy(tokens[numTokens++], token, size + 1);
        size = 0;
    }

    free(token);

    if (numTokens != numCols) {
        RAISE_ERROR("5Invalid File Format\n");
    }

    return tokens;
}


int main(int argc, char **argv) {
    FILE* fp;

    // check command line arguments
    if (argc != 2) {
        RAISE_ERROR("6Invalid Input Format\n");
    }
   
    // check if can open fp
    if ((fp = fopen(argv[1], "r")) == NULL) {
        RAISE_ERROR("7Invalid Input Format\n");
    }

    printTop10(fp);

    fclose(fp);

    return 0;
}
