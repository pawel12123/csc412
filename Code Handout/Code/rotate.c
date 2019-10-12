#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "imageIO_TGA.h"
#include "RasterImage.h"

#define LOG_PATH	"../Output/rotateLog.txt"

//Function Declarations
int checkFileExist(char* path);
char* generateOutputPath(char* input, char* output);
char* outputCropName(char* rootName, char* output);
void rotateLeft(unsigned char* input, unsigned char* output, unsigned int numRows,  unsigned int numCols);
void rotateRight(unsigned char* input, unsigned char* output, unsigned int numRows,  unsigned int numCols);
void flipImage(unsigned char* input, unsigned char* output, unsigned int numRows,  unsigned int numCols);

//--------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]){
    //LOG maintaining variables
    FILE * fp;
    if(checkFileExist(LOG_PATH) == 1){
        fp = fopen (LOG_PATH,"w");
    }
    else{
        fp = fopen (LOG_PATH,"a");
    }

    //Variables
    ImageType imgType;
    unsigned int numCols = 0;
    unsigned int numRows = 0;

    fprintf (fp, "Reading in Rasters..\n");
    unsigned char* input = readTGA(argv[1], &numRows, &numCols, &imgType);
    unsigned char* output = (unsigned char*)malloc(sizeof(unsigned char) * numRows * numCols * 4);

    //Error Checks
    fprintf (fp, "Checking Arguments...\n");
    if(checkFileExist(argv[1]) == 1){
        printf("Invalid input path.");
        exit(1);
    }
    if(checkFileExist(argv[2]) == 1){
        printf("Invalid output path.");
        exit(1);
    }
    if(argc > 4){
        printf("Too Many Arguments. Should be (path to file, path of output, x, y, width, height).");
        exit(1);
    }
    if(argc < 4){
    printf("Not Enough Arguments. Should be (path to file, path to output, Rotation specifier ['l', 'L', 'R', 'r', 'RR', 'LL').\n");
    exit(1);
    }

    //Determining which rotation to take.
    fprintf(fp, "Determining which rotation to make.\n");
    //Left Rotation
    if ((strcmp(argv[3], "l") == 0) || (strcmp(argv[3], "L") == 0)){
        fprintf (fp, "Rotating image left by 90 Degrees...\n");
        rotateLeft(input, output, numRows, numCols);

        if (writeTGA(generateOutputPath(argv[1], argv[2]), output, numCols, numRows)){
            fprintf (fp, "Writing out of the image failed.\n");
            printf("Writing out of the image failed.\n");
        }
        printf("Done! \n");
    }
    //Right rotation
    else if((strcmp(argv[3], "r") == 0) || (strcmp(argv[3], "R") == 0)){
        fprintf (fp, "Rotating image right by 90 Degrees...\n");
        rotateRight(input, output, numRows, numCols);

        if (writeTGA(generateOutputPath(argv[1], argv[2]), output, numCols, numRows)){
            fprintf (fp, "Writing out of the image failed.\n");
            printf("Writing out of the image failed.\n");
        }
        printf("Done! \n");
    }
    //180 Rotation
    else if((strcmp(argv[3], "ll") == 0) ||(strcmp(argv[3], "LL") == 0) || (strcmp(argv[3], "RR") == 0) || (strcmp(argv[3], "rr") == 0)){
        fprintf (fp, "Rotating image by 180 Degrees...\n");
        flipImage(input, output, numRows, numCols);

        if (writeTGA(generateOutputPath(argv[1], argv[2]), output, numRows, numCols)){
            fprintf (fp, "Writing out of the image failed.\n");
            printf("Writing out of the image failed.\n");
        }
        printf("Done! \n");
    }
    else{
         printf("Invalid rotation specifier.\n");
         exit(-1);
    }
    fprintf (fp, "\n----------EXECUTION DONE--------------\n\n");
    fclose(fp);
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------
char* generateOutputPath(char* input, char* output){
    //USE STRCAT
    int indexDot = 0;
    int indexSlash = 0;
    int index = 0;

    while(input[index] != '\0'){
        if(input[index] == '/'){
            indexSlash = index;
        }
        if(input[index] == '.'){
            indexDot = index;
        }
            index++;
    }

    int i;
    int rootNameLen = indexDot-indexSlash;

    char* rootName = (char*)malloc(sizeof(char)*rootNameLen+1);

    for(i = 1; i < rootNameLen; ++i){
        rootName[i-1] = input[indexSlash + i];
    }
    rootName[i-1] = '\0';

    //Check outputPath directory slash.
    int outputLen = strlen(output);
    if(output[outputLen-1] == '/'){
        output[outputLen-1] = '\0';
    }

    char* outputName = outputCropName(rootName, output);
    free(rootName);
    rootName = NULL;
    return outputName;
}

//--------------------------------------------------------------------------------------------------------------------------------
//This function handles everything in construction of the output path and name, returning the path and name of the file, handling the incrementing the [rotated x]
//and such.
char* outputCropName(char* rootName, char* output){
    int rootNameLen = strlen(rootName);
    char* temp = (char*)malloc(sizeof(char)*rootNameLen+20);

    char strIndex[3];
    int index = 0;
    while(1){
        strcpy(temp, output);
        temp[strlen(temp)] = '\0';
        strcat(temp, "/");
        temp[strlen(temp)] = '\0';
        if(index > 0){
            sprintf(strIndex, "%d", index);
        }
        strcat(temp, rootName);
        temp[strlen(temp)] = '\0';
        strcat(temp, "[rotated");
        temp[strlen(temp)] = '\0';
        if(index > 0){
            strcat(temp, " ");
            temp[strlen(temp)] = '\0';
            strcat(temp, strIndex);
            temp[strlen(temp)] = '\0';
        }
        strcat(temp, "].tga");
        temp[strlen(temp)] = '\0';
        if(checkFileExist(temp) == 1){
            break;
        }
        index++;
    }
    return temp;
}
//--------------------------------------------------------------------------------------------------------------------------------
//Functions which checks whether a file exists or not. Used for verifying input and output path/.
int checkFileExist(char* path){
    if(access(path, F_OK) != -1){
        //Exists
        return 0;
    }
    else{
        //Doesn't Exist
        return 1;
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
//Function for 90 Degree Left Rotation
void rotateLeft(unsigned char* input, unsigned char* output, unsigned int numRows,  unsigned int numCols){
    for (unsigned int i = 0; i < numRows; i++){
        for(unsigned int j = 0; j < numCols; j++){
            unsigned char* input_rgba = (unsigned char*) input + 4 * (i * numCols + j);
            unsigned char* output_rgba = (unsigned char*) output + 4 * (j * numRows + (numRows - i - 1));//rotate left 90
            output_rgba[0] = input_rgba[0];
            output_rgba[1] = input_rgba[1];
            output_rgba[2] = input_rgba[2];
            output_rgba[3] = input_rgba[3];
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
//Function for 90 Degree Right Rotation
void rotateRight(unsigned char* input, unsigned char* output, unsigned int numRows,  unsigned int numCols){
    for (unsigned int i = 0; i < numRows; i++){
        for(unsigned int j = 0; j < numCols; j++){
            //Create old and new rgba rasters
            unsigned char* input_rgba = (unsigned char*) input + 4 * (i * numCols + j);
            unsigned char* output_rgba = (unsigned char*) output + 4 * (j * numRows + i);//rotate right 90
            output_rgba[0] = input_rgba[0];
            output_rgba[1] = input_rgba[1];
            output_rgba[2] = input_rgba[2];
            output_rgba[3] = input_rgba[3];
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
//Function for 180 Degree Rotation
void flipImage(unsigned char* input, unsigned char* output, unsigned int numRows,  unsigned int numCols){
    for (unsigned int i = 0; i < numRows; i++){
        for(unsigned int j = 0; j < numCols; j++){
            unsigned char* input_rgba = (unsigned char*) input + 4 * (i * numCols + j);
            unsigned char* output_rgba = (unsigned char*) output + 4 * ((numRows * numCols) - numCols + j - i * numCols);
            output_rgba[0] = input_rgba[0];
            output_rgba[1] = input_rgba[1];
            output_rgba[2] = input_rgba[2];
            output_rgba[3] = input_rgba[3];
        }
    }
}
