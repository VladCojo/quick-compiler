#include <stdio.h>
#include <stdlib.h>

#include "../include/lexer.h"
#include "../include/utils.h"

#define MAX_INPUT_SIZE 4096

int main(int argc, char* argv[]){
    
        if(argc < 1){
        err("You must provide a path");
    }

    char *buf = loadFile(argv[1]);
    
    tokenize(buf);

    printf("Tokens: \n");
    showTokens();


    return EXIT_SUCCESS;
}