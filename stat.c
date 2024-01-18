#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINE_SIZE 60
#define MAX_WORD_SIZE 15
#define MAX_COLUMN_WIDTH 70
#define WORD_COLUMN_WIDTH 15
#define COUNT_COLUMN_WIDTH 5
#define GRAPH_COLUMN_WIDTH (MAX_COLUMN_WIDTH - WORD_COLUMN_WIDTH - COUNT_COLUMN_WIDTH - 5)
#define READ 0
#define WRITE 1


struct WordCount{
    char word[MAX_WORD_SIZE];
    int count;
};
void printWordCountTable(const struct WordCount wordCounts[], int lineCount);
void parseInputString(const char* input, struct WordCount wordCounts[], int lineCount);

/*
Take 2 param
1. File name
2. Count of lines

Count of uniq words in text and make a graph
*/

int main(int argc, char *argv[]) {
    // Check count of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s param1 param2\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int lineCount = atoi(argv[2]);

    // Use shell script
    char *scriptPath = "./stat.sh";
    char *scriptArgs[] = {scriptPath, argv[1], argv[2], NULL};

    // Initialize an array to store word count struct by count of lines
    struct WordCount wordTable[lineCount];

    // Pipe for child and parent processes
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork
    pid_t child_pid = fork();

    // Error
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Child
    if (child_pid == 0) {
        // Close read end of the pipe
        close(pipe_fd[READ]);    

        // Redirect stdout to the write end of the pipe
        dup2(pipe_fd[WRITE], STDOUT_FILENO);

        // Close the original write end of the pipe
        close(pipe_fd[WRITE]); 

        // Use execvp to run shell
        execvp(scriptPath, scriptArgs); 

        // If execvp fails
        perror("execvp");
        exit(EXIT_FAILURE);
    } 
    // Parent
    else {
        // Close write end of the pipe
        close(pipe_fd[WRITE]); 

        // Calculate max output
        char output[MAX_LINE_SIZE * lineCount];
        ssize_t bytesRead = read(pipe_fd[0], output, sizeof(output));

        if (bytesRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Null-terminate the string
        output[bytesRead] = '\0'; 

        //Make a table
        parseInputString(output, wordTable,lineCount);
        printWordCountTable(wordTable ,lineCount);

        int status;
        waitpid(child_pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child process exited with status %d\n", WEXITSTATUS(status));
        } else {
            fprintf(stderr, "Child process exited abnormally\n");
        }

        close(pipe_fd[0]); // Close read end of the pipe
    }

    return 0;
}


// from shell script to array
void parseInputString(const char* input, struct WordCount wordCounts[], int lineCount) {
    // Tokenize the input string using space and newline as delimiters
    char* lines[lineCount * 2];  // Double the size to store count and word pairs
    char* token = strtok((char*)input, " \n");
    int i = 0;

    while (token != NULL && i < lineCount * 2) {
        lines[i++] = token;
        token = strtok(NULL, " \n");
    }

    // Parse each pair of count and word
    for (int j = 0; j < lineCount; j++) {
        if (j * 2 < i) {
            sscanf(lines[j * 2], "%d", &wordCounts[j].count);
            strcpy(wordCounts[j].word, lines[j * 2 + 1]);
        } else {
            fprintf(stderr, "Error: Insufficient data for line %d\n", j + 1);
        }
    }
}

// print array
void printWordCountTable(const struct WordCount wordCounts[], int lineCount) {
    printf("%-*s | %-*s | %-*s\n", WORD_COLUMN_WIDTH, "Word", COUNT_COLUMN_WIDTH, "Count", GRAPH_COLUMN_WIDTH, "Graph");
    printf("%-*s-|-%-*s-|-%-*s\n", WORD_COLUMN_WIDTH, " ", COUNT_COLUMN_WIDTH, " ", GRAPH_COLUMN_WIDTH, " ");
    for (int k = 0; k < lineCount; k++) {
        int graphLength = (int)((float)GRAPH_COLUMN_WIDTH * ((float)wordCounts[k].count / wordCounts[0].count));
        printf("%-*s | %-*d | ", WORD_COLUMN_WIDTH, wordCounts[k].word, COUNT_COLUMN_WIDTH, wordCounts[k].count);
        for (int i = 0; i < graphLength; i++) {
            putchar('|');
        }
        printf("\n");
    }
}