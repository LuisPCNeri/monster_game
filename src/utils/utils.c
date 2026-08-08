#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

// Takes in the path to a file and reads it to a string
// Returns a pointer to the string or NULL if it fails
// The caller is responsible for freeing the string
char* LoadFileToString(const char* file_path){
    FILE* fptr;
    if(!(fptr = fopen(file_path, "r"))){
        perror("Open JSON Monster File: ");
        return NULL;
    }

    // Count file size
    fseek(fptr, 0 , SEEK_END);
    long file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char* buffer = (char*) malloc((size_t) file_size + 1);
    size_t out = fread(buffer, 1, file_size, fptr);
    if(out == 0) perror("fread");
    buffer[file_size] = '\0';

    fclose(fptr);
    return buffer;
}
