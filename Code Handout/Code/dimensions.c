#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
//
#include "imageIO_TGA.h"

#define LOG_PATH	"../Output/dimensionsLog.txt"

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

int main(int argc, char** argv){
    //LOG Maintaining variables
    FILE * fp;
    if(checkFileExist(LOG_PATH) == 1){
        fp = fopen (LOG_PATH,"w");
    }
    else{
        fp = fopen (LOG_PATH,"a");
    }
    unsigned int numRows, numCols;
    ImageType imgType;

    //Check Errors
    fprintf (fp, "Checking For Errors..\n");
    if(argc > 4){
        printf("Too many arguments. At most 3.\n");
        fprintf (fp, "Too many arguments provided: %d..\n", argc-1);
        exit(-1);
    }
    if(argc < 3){
        printf("Too few arguments. At least 2.\n");
        fprintf (fp, "Too few arguments provided: %d..\n", argc-1);
        exit(-1);
    }
    fprintf (fp, "Checking argument format..\n");

    if((strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "-h") == 0) && strcmp(argv[2], "-v") == 0){
        fprintf (fp, "File path being looked at: %s\n", argv[3]);
        fprintf (fp, "Found arguments of a verbose dimension..\n");
        unsigned char* raster = readTGA(argv[3], &numRows, &numCols, &imgType);
    }
    else if((strcmp(argv[1], "-v") == 0) && ((strcmp(argv[2], "-w") == 0) || strcmp(argv[2], "-h") == 0)){
        fprintf (fp, "File path being looked at: %s\n", argv[3]);
        fprintf (fp, "Found arguments of a verbose dimension..\n");
        unsigned char* raster = readTGA(argv[3], &numRows, &numCols, &imgType);
    }
    else if((strcmp(argv[1], "-w") == 0) || (strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "-h") == 0)){
        fprintf (fp, "File path being looked at: %s\n", argv[2]);
        fprintf (fp, "found argument of only a dimension or verbose..\n");
        unsigned char* raster = readTGA(argv[2], &numRows, &numCols, &imgType);
    }
    else{
        fprintf (fp, "File path being looked at: %s\n", argv[1]);
        fprintf (fp, "One argument provided. Either Path to file or error..\n");
        unsigned char* raster = readTGA(argv[1], &numRows, &numCols, &imgType);
    }

    fprintf (fp, "Outputting proper response.\n");
    if(strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-h") == 0){
        printf("height: %d\n", numRows);
    }
    else if(strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-w") == 0){
        printf("width: %d\n", numCols);
    }
    else if(strcmp(argv[1], "-w") == 0 && strcmp(argv[2], "-v") == 0){
        printf("width: %d\n", numCols);
    }
    else if(strcmp(argv[1], "-h") == 0 && strcmp(argv[2], "-v") == 0){
        printf("height: %d\n", numRows);
    }
    else if(strcmp(argv[1], "-h") == 0){
        printf("%d\n", numRows);
    }
    else if(strcmp(argv[1], "-w") == 0){
        printf("%d\n", numCols);
    }
    else if(strcmp(argv[1], "-v") == 0){
        printf("width: %d, height: %d\n", numCols, numRows);
    }
    else{
        printf("%d %d", numCols, numRows);
    }
    fprintf (fp, "\n--------------DONE EXECUTION:---------------\n");
    printf("Done!\n");
    fclose(fp);

    return 0;
}
