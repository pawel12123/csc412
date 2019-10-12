#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "imageIO_TGA.h"

#define LOG_PATH	"../Output/splitLog.txt"

//Function Declarations
int checkFileExist(char* path);
char* generateOutputPath(char* input, char* output, char* color);
char* outputCropName(char* rootName, char* output, char* color);

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
char* generateOutputPath(char* input, char* output, char* color){
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

    char* outputName = outputCropName(rootName, output, color);
    free(rootName);
    rootName = NULL;
    return outputName;
}

//This function handles everything output, returning the path and name of the file, handling the incrementing the  [cropped x] and such.
char* outputCropName(char* rootName, char* output, char* color){
    int rootNameLen = strlen(rootName);
    int outputLen = strlen(output);
    char* temp = (char*)malloc(sizeof(char)*rootNameLen+ outputLen+22);

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
        strcat(temp, color);
        temp[strlen(temp)] = '\0';
        strcat(temp, "[cropped");
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

int main(int argc, char** argv){
    //LOG maintaining variables
    FILE * fp;
    if(checkFileExist(LOG_PATH) == 1){
        fp = fopen (LOG_PATH,"w");
    }
    else{
        fp = fopen (LOG_PATH,"a");
    }

    unsigned int numRows, numCols;
	ImageType imgType;
	unsigned char bytesPerPixel;
	unsigned int bytesPerRow;

	fprintf (fp, "Reading in Rasters..\n");
    unsigned char* rasterR = readTGA(argv[1], &numRows, &numCols, &imgType);
    unsigned char* rasterG = readTGA(argv[1], &numRows, &numCols, &imgType);
    unsigned char* rasterB = readTGA(argv[1], &numRows, &numCols, &imgType);

    //Error checks
    fprintf (fp, "Checking For Errors...\n");
    if(argc < 3){
        printf("Not Enough Arguments. Arguments Should be (Input Path, Output Path)");
        exit(-1);
    }
    if(argc > 3){
        printf("Too Many Arguments. Arguments Should be (Input Path, Output Path)");
        exit(-1);
    }
    if (rasterR != NULL && rasterG != NULL && rasterB != NULL)
	{
		switch (imgType)
		{
			case RGBA32_RASTER:
			case FLOAT_RASTER:
				bytesPerPixel = 4;
				break;

			case GRAY_RASTER:
				bytesPerPixel = 1;
			break;
		}

		bytesPerRow = bytesPerPixel * numCols;
	}
	else
	{
		printf("Reading of image file failed.\n");
		exit(-1);
	}

	fprintf (fp, "Killing color channels..\n");
	//Loop that kills color channels
    for (unsigned int i=0; i< numRows; i++)
	{
		for (unsigned int j=0; j< numCols; j++)
		{
                //Kill Green and Blue
				rasterR[i*bytesPerRow + 4*j + 1] = 0x00;
				rasterR[i*bytesPerRow + 4*j + 2] = 0x00;
				//Kill Red and Blue
				rasterG[i*bytesPerRow + 4*j + 0] = 0x00;
				rasterG[i*bytesPerRow + 4*j + 2] = 0x00;
				//Kill Red and Green
				rasterB[i*bytesPerRow + 4*j + 0] = 0x00;
				rasterB[i*bytesPerRow + 4*j + 1] = 0x00;
		}
	}

    //Get Output Path
	fprintf (fp, "Generating output paths.\n");

	if (writeTGA(generateOutputPath(argv[1], argv[2], "_r"), rasterR, numRows, numCols))
	{
		printf("Writing out of the image failed.\n");
	}
	if (writeTGA(generateOutputPath(argv[1], argv[2], "_g"), rasterG, numRows, numCols))
	{
		printf("Writing out of the image failed.\n");
	}
	if (writeTGA(generateOutputPath(argv[1], argv[2], "_b"), rasterB, numRows, numCols))
	{
		printf("Writing out of the image failed.\n");
	}

    fprintf (fp, "--------------DONE EXECUTION:---------------\n");
    printf("Done!\n");
    fclose(fp);
    return 0;
}
