//  bestWordle.c
//  Find the best one or two wordle starting words.
// 
//  Author: Dale Reed, 10/8/22
//  System: CLion and XCode
//
//  Links to wordle dictionary words at:
//    https://www.reddit.com/r/wordle/comments/s4tcw8/a_note_on_wordles_word_list/
//

#include <stdio.h>    // for printf, scanf
#include <stdlib.h>   // for exit( -1)
#include <string.h>   // for strcpy

// Declare globals
#define WORD_LENGTH 5     // All words have 5 letters, + 1 NULL at the end when stored
#define ANSWERS_FILE_NAME "answersLarge.txt"
#define GUESSES_FILE_NAME "guessesLarge.txt"
const int DebugOn = 0;    // Set to 1 to display debug info

typedef struct wordCount wordCountStruct;
struct wordCount{
    char word[ WORD_LENGTH + 1];   // The word length plus NULL
    int score;                     // Score for the word
};


//-----------------------------------------------------------------------------------------
// Count the number of words in an input file and return it.
int countWordsInFile( char fileName[])
{
    FILE *inFilePtr  = fopen(fileName, "r");  // Connect logical name to filename
    char inputString[ 81];

    // Ensure file open worked correctly
    if( inFilePtr == NULL ) {
        printf("Error: could not open %s for reading\n", fileName);
        exit(-1);    // must include stdlib.h
    }

    // Read from file.  For validation display only the first group of words.
    int wordsCounter = 0;
    while( fscanf( inFilePtr, "%s", inputString) != EOF) {
        wordsCounter++;
    }

    // Close the file
    fclose( inFilePtr);

    return wordsCounter;
} // end countWordsInFile(..)


//-----------------------------------------------------------------------------------------
// Read file and one at a time append all the words from that file into the array.
void appendWordsFromFileToArray(
            char fileName[],        // Filename we'll read from
            wordCountStruct *words, // Array of words where we'll store words we read from file
            int startingIndex)      // Starting index into which we will store words.  This is
                                    // needed when we are appending the guesses words to the
                                    // array of already stored answer words, giving all words.
{
    FILE *inFilePtr  = fopen(fileName, "r");  // Connect logical name to filename
    char inputString[ 81];

    // Sanity check: ensure file open worked correctly
    if( inFilePtr == NULL ) {
        printf("Error: could not open %s for reading\n", fileName);
        exit(-1);    // must include stdlib.h
    }

    // Read each word from file and store into array, initializing the score for that word to 0.
    int index = startingIndex;
    while( fscanf( inFilePtr, "%s", inputString) != EOF) {
        strcpy( words[ index].word, inputString);
        words[ index].score = 0;
        index++;
    }

    // Close the file
    fclose( inFilePtr);
} // end appendWordsFromFileToArray(..)


//-----------------------------------------------------------------------------------------
// Read in words from files into arrays, displaying how many words there are in each file.
// The number of words in each file was previously calculated, and space was dynamically
// allocated for each of the arrays.
void readInWordsAndDisplayNumbers(
        wordCountStruct *answerWords,   // Array of the answer words
        int answersWordCount,           // How many words there are in answerWords
        wordCountStruct *allWords,      // Array of all the words
        int totalWordCount,             // How many words there are in allWords
        int guessesWordCount,           // How many words there are in the guesses
        char answersFileName[],         // Name of the answers file
        char guessesFileName[])         // Name of the guesses file
{
    char fileName[ 81];         // Will be reused for different file names.

    // Read all the words from the answers and guesses files, this time storing them
    // in our newly allocated arrays. As we go display the total number of words in each file.
    strcpy( fileName, answersFileName);
    // Add the answers words to both arrays, starting at index 0
    appendWordsFromFileToArray( fileName, answerWords, 0);
    appendWordsFromFileToArray( fileName, allWords, 0);
    printf("%s has %d words\n", fileName, answersWordCount);    // Display word counts

    // Append the guesses words to the allWords array as well, starting at index answersWordCount,
    // which should be the next open spot into which the next word can be stored.
    strcpy( fileName, guessesFileName);
    appendWordsFromFileToArray( fileName, allWords, answersWordCount);
    printf("%s has %d words\n", fileName, guessesWordCount);    // Display word counts
} //end readInWordsAndDisplayNumbers(..)


//-----------------------------------------------------------------------------------------
// Calculate the word comparison score, where it gets:
//   - One point for each correct letter in an incorrect position
//   - Three points for each correct letter in the correct position
int getSingleWordComparisonScore( char originalWordParameter[], char comparisonWordParameter[])
{
    // Make copies of words, to use in blanking out letters that have already contributed
    // to scoring.
    char originalWord[ WORD_LENGTH + 1];
    char comparisonWord[ WORD_LENGTH + 1];
    strcpy( originalWord, originalWordParameter);
    strcpy( comparisonWord, comparisonWordParameter);

    int score = 0;    // Accumulates word score

    // Find exact matches
    for( int i=0; i<WORD_LENGTH; i++) {
        // Accumulate score for exact letter match, and blank out letter so it is
        // not reused.
        if( originalWord[ i] == comparisonWord[ i] ) {
            score += 3;     // Points for a matching letter in the same position
            originalWord[ i] = ' ';
            comparisonWord[ i] = ' ';
        }
    }

    // Find matching letter in a different position.  Letters that were exact
    // matches have already been blanked out.
    for( int i=0; i<WORD_LENGTH; i++) {
        char c = originalWord[ i];
        // Only consider non-blanks
        if( c != ' ') {
            // Step through each possible matching character.
            for( int j=0; j<WORD_LENGTH; j++) {
                // Accumulate score for exact letter match, and blank out letter so it is
                // not reused. After a match is found, break out of comparison loop so that
                // letter does not count for scoring more than once.
                if( c == comparisonWord[ j]) {
                    score += 1;
                    originalWord[ i] = ' ';
                    comparisonWord[ j] = ' ';
                    break;
                }
            } //end for( int j ...)
        } //end if( c...
    } //end for( int i...

    return score;
} //end getSingleWordComparisonScore(..)


//-----------------------------------------------------------------------------------------
// For the given word, calculate its score by comparing how well it matches each answer word.
int getScore( char theGuess[],              // Word being evaluated as a guess
              wordCountStruct *answerWords, // All the answer words, which theGuess is compared against
              int answersWordCount)         // Number of words in answers file
{
    int score = 0;
    for( int i=0; i<answersWordCount; i++) {
        score += getSingleWordComparisonScore( theGuess, answerWords[ i].word);
    }
    return score;
}


//-----------------------------------------------------------------------------------------
// Comparator for use in built-in qsort(..) function.  Parameters are declared to be a
// generic type, so they will match with anything.
// This is a two-part comparison.  First the scores are compared.  If they are the same,
// then the words themselves are also compared, so that the results are in descending
// order by score, and within score they are in alphabetic order.
int compareFunction( const void * a, const void * b) {
    // Before using parameters we have cast them into the actual type they are in our program
    // and then extract the numerical value used in comparison
    int firstScore = ((wordCountStruct *) a)->score;
    int secondScore = ((wordCountStruct *) b)->score;

    // If scores are different, then that's all we need for our comparison.
    if (firstScore != secondScore) {
        // We reverse the values, so the result is in descending vs. the otherwise ascending order
        // return firstScore - secondScore;   // ascending order
        return secondScore - firstScore;      // descending order
    }
    else {
        // Scores are equal, so check words themselves, to put them in alphabetical order
        return strcmp( ((wordCountStruct *)a)->word,  ((wordCountStruct *)b)->word );
    }
} //end compareFunction(..)


// -----------------------------------------------------------------------------------------
// Find the score for each word in the allWords array by comparing it against all the words
// in answerWords and accumulating values for matching letters.
// Find the number of top-scoring words, returning this value through the reference parameter.
void findScoresAndTopWords(
        wordCountStruct *answerWords,   // Array of the answer words
        int answersWordCount,           // How many words there are in answerWords
        wordCountStruct *allWords,      // Array of all the words
        int totalWordCount,             // How many words there are in allWords
        wordCountStruct * *bestWords,   // Array to be allocated to store best words
        int *numberOfTopScoringWords)   // How many words are tied with top score value
{
    // For each word in the allWords array, calculate its score to represent how good of a job
    // it does on average at matching letters from the answer words.  The struct used to
    // store words has space for the 5-letter word as well as for that word's score.
    for( int i=0; i<totalWordCount; i++) {
        allWords[ i].score =  getScore( allWords[ i].word, answerWords, answersWordCount);
    }

    // Sort the allWords array in descending order by score, and within score they should also
    // be sorted into ascending order alphabetically.  Use the built-in C quick sort qsort(...).
    // The last parameter is the name of the comparison function we use (a function pointer).
    qsort( allWords, totalWordCount, sizeof( wordCountStruct), compareFunction);

    // Retrieve the top score and count the number of words that all share that top score.
    int topScore = allWords[ 0].score;
    int index = 0;
    while( allWords[ index].score == topScore) {
        index++;
    }
    *numberOfTopScoringWords = index;

    // Allocate memory for the best words array and store words into it.
    *bestWords = (wordCountStruct *) malloc( sizeof( wordCountStruct) * *numberOfTopScoringWords);
    for( int i=0; i < *numberOfTopScoringWords; i++) {
        strcpy( (*bestWords)[ i].word, allWords[ i].word);   // Store
        (*bestWords)[ i].score = topScore;
    }

    // For debugging set global value DebugOn to 1
    if( DebugOn) {
        // For debugging display all words in descending order
        printf("All words in descending order by score:\n");
        for (int i = 0; i < totalWordCount; i++) {
            printf("%d %s\n", allWords[ i].score, allWords[ i].word);   // Display
        }
    }
    
    if( DebugOn) {
        // For debugging display the top scoring words
        printf("Top scoring words:\n");
        for (int i = 0; i < *numberOfTopScoringWords; i++) {
            printf("%s %d\n", (*bestWords)[i].word, (*bestWords)[i].score);   // Display
        }
    }
} //end findScoresAndTopWords(..)


// -----------------------------------------------------------------------------------------
// Go through the array of answerWordsCopy.  For each word, remove the letters that were already
// handled with the bestWord, so we are only scoring on letters from the second move and
// not (again) from first move letters represented by bestWord.
void removeMatchingLetters(
        wordCountStruct *answerWordsCopy, // Copy of the answerWords
        int answersWordCount,             // How many words in the allWords array
        char bestWord[ ])                 // The best word
{
    // Go through each word in answerWordsCopy
    for( int i=0; i<answersWordCount; i++) {
        // For the current (ith) answerWordsCopy word, step through each (jth) letter in bestWord
        
        // Make a copy of the best word
        char bestWordCopy[ 6];
        strcpy( bestWordCopy, bestWord);
        
        // First blank out matching letters in the same position.
        for( int j=0; j<WORD_LENGTH; j++) {
            // Compare the bestWord letter[ j] to the answersWordsCopy[ j] letter.
            if( bestWordCopy[ j] == answerWordsCopy[ i].word[ j] ) {
                // Blank out matching letters, so they can't be reused
                bestWordCopy[ j] = ' ';
                answerWordsCopy[ i].word[ j] = ' ';
            }
        }
                
        // Next blank out matching letters in different positions.
        for( int j=0; j<WORD_LENGTH; j++) {
            // Compare the current (jth) bestWord letter to each answersWordsCopy (kth) letter.
            for( int k=0; k<WORD_LENGTH; k++) {
                // If a match is found, blank out the answersWordsCopy letter so it will not contribute to scoring
               if( bestWordCopy[ j] == answerWordsCopy[ i].word[ k] ) {
                   answerWordsCopy[ i].word[ k] = ' ';
                   break;   // Go on to next (jth) letter in the bestWordCopy
               }
            } //end for( int k...
        } //end for( int j...
    } //end for( int i...

} //end removeMatchingLetters(..)


// -----------------------------------------------------------------------------------------
// Find the set of best second words, once the letters from the first words are taken out
// of the way.
void findAndDisplayBestSecondWords(
        wordCountStruct *answerWords,   // The set of answer words
        int answersWordCount,           // How many answer words there are
        wordCountStruct *allWords,      // The set of all words
        int totalWordCount,             // How many allWords there are
        wordCountStruct *bestWords,     // The set of best first words
        int bestWordIndex)              // Index of current best word being used to find best
                                        //   second words.
{
    char bestWord[ 81];        // The best first word used this round to find best second words.
    strcpy( bestWord, bestWords[ bestWordIndex].word);
    int bestWordScore = bestWords[ bestWordIndex].score;

    // Make a copy of answerWords, zeroing out its scores and eliminating the first occurrence
    // of all characters found in the current top-scoring word.
    // First allocate space for the copy.
    wordCountStruct *answerWordsCopy = (wordCountStruct *) malloc( sizeof( wordCountStruct) * answersWordCount);

    // Copy the original words into answerWordsCopy, and zero-out scores in the copy
    for( int j=0; j<answersWordCount; j++) {
        strcpy( answerWordsCopy[ j].word, answerWords[ j].word);
        answerWordsCopy[ j].score = 0;
    }

    // Remove single letters matching those in the current best word under consideration
    removeMatchingLetters( answerWordsCopy, answersWordCount, bestWord);

    // For each word in allWords find its score by comparing to all answerWordsCopy.
    // Sort and find top scoring words.
    int numberOfTopScoringSecondWords = 0;
    wordCountStruct *bestSecondWords = NULL;  // Will be allocated in function below
    findScoresAndTopWords( answerWordsCopy, answersWordCount,
                           allWords, totalWordCount,
                           &bestSecondWords, &numberOfTopScoringSecondWords);

    if( DebugOn) {
        // For debugging display allWordsCopy, with letters from bestWord removed
        printf("answerWordsCopy after letters from %s removed:\n", bestWord);
        for (int j = 0; j < answersWordCount; j++) {
            printf("%2d. %s\n", j, answerWordsCopy[j].word);
        }

        // For debugging display all words and scores
        printf("    Words and scores:\n");
        for (int i = 0; i < totalWordCount; i++) {
            printf("    %s %d \n", allWords[i].word, allWords[i].score);
        }
    } //end if( DebugOn)

    // Display the top scoring first and second words
    printf("%s %d\n", bestWord, bestWordScore);
    for (int i = 0; i < numberOfTopScoringSecondWords; i++) {
        printf("   %s %d", bestSecondWords[i].word, bestSecondWords[i].score);
    }
    printf("\n");
} //end findAndDisplayBestSecondWords(..)


// -----------------------------------------------------------------------------------------
int main() {
    int answersWordCount = 0;   // Counter for number of words in answers file
    int guessesWordCount = 0;   // Counter for number of words in answers file
    char answersFileName[81];  // Stores the answers file name
    char guessesFileName[81];  // Stores the guesses file name
    // Global macros for filenames are provided at program top, for convenience in changing.
    strcpy(answersFileName, ANSWERS_FILE_NAME);
    strcpy(guessesFileName, GUESSES_FILE_NAME);
    printf("Default file names are %s and %s\n", answersFileName, guessesFileName);

    // Display menu, to allow partial credit for different program components
    int menuOption = 0;
    do {
        printf("\n");
        printf("Menu Options:\n");
        printf("  1. Display best first words only\n");
        printf("  2. Display best first and best second words\n");
        printf("  3. Change answers and guesses filenames\n");
        printf("  4. Exit\n");
        printf("Your choice: ");
        scanf("%d", &menuOption);

        // Handle menu option 3 to exit the program
        if (menuOption == 4) {
            exit(1); // Exit the program
        } else if( menuOption == 3) {
            // Change file names.  Menu will then be redisplayed.
            printf("Enter new answers and guesses filenames: ");
            scanf("%s %s", answersFileName, guessesFileName);
        } else if( menuOption == 5) {
            // Hidden menu option to choose large files
            strcpy( answersFileName, "answersLarge.txt");
            strcpy( guessesFileName, "guessesLarge.txt");
            // Set menu option to display first and second words
            menuOption = 2;
            continue;
        }
    } while( menuOption == 3);

    // Count the total number of words in the answers and guesses files.
    answersWordCount = countWordsInFile(answersFileName);
    guessesWordCount = countWordsInFile(guessesFileName);
    int totalWordCount = answersWordCount + guessesWordCount;

    // Allocate space for an array of answer words, and for an array of all the words.
    wordCountStruct *answerWords = (wordCountStruct *) malloc(sizeof(wordCountStruct) * answersWordCount);
    wordCountStruct *allWords = (wordCountStruct *) malloc(sizeof(wordCountStruct) * totalWordCount);

    // Read in words from files into arrays, displaying how many words there are in each file
    readInWordsAndDisplayNumbers(answerWords, answersWordCount, allWords, totalWordCount, guessesWordCount, answersFileName, guessesFileName);

    // For each word find its score by comparing to all answerWords.  Sort and find top scoring words.
    int numberOfTopScoringWords = 0;
    wordCountStruct *bestWords = NULL;  // Will be allocated in function below
    findScoresAndTopWords( answerWords, answersWordCount, allWords, totalWordCount, &bestWords, &numberOfTopScoringWords);

    // If we got to this point, menuOption is 1 or 2
    printf("\n");
    if( menuOption == 1) {
        // Display best first-guess words.  There could be multiples if there was a tie.
        printf("Words and scores for top first words:\n");
        for (int i = 0; i < numberOfTopScoringWords; i++) {
            printf("%s %d\n", bestWords[i].word, bestWords[i].score);   // Display
        }
    }
    else if( menuOption == 2) {
        // For each top-scoring word, find the best second word.
        printf("Words and scores for top first words and second words:\n");
        for( int i=0; i<numberOfTopScoringWords; i++) {
            findAndDisplayBestSecondWords( answerWords, answersWordCount, allWords, totalWordCount, bestWords, i);
        } //end for( int i...
    }

    printf("Done\n");
    return 0;
} // end main()