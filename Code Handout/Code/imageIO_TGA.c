/*----------------------------------------------------------------------------------+
|	This is a modified version of the so-called "Lighthouse Library" for reading	|
|	images encoded in the *uncompressed, uncommented .tga (TARGA file) format. 		|
|	I had been using and modifying this code for years, simply mentioning			|
|	"Source Unknown" in my comments when I finally discovered, thanks to Web		|
|	searches, the origin of this code.  By then it had been adapted to work along	|
|	with reader/writer code for another image file format: the awful PPM/PBM/PGM	|
|	format of obvious Unix origin.													|
|	This is just to say that I am not claiming authorship of this code.  I am only	|
|	distributing it in this form instead of the original because (1) I have long	|
|	lost the original code, and (2) This version works with images organized		|
|	nicely into a struct.															|
|																					|
|	Jean-Yves Hervé		Dept. of Computer Science and Statistics, URI				|
|						2019-10-03													|
+----------------------------------------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>

#include "imageIO_TGA.h"


void swapRGBA(unsigned char *theData, unsigned int nbRows, unsigned int nbCols);


//----------------------------------------------------------------------
//	Utility function for memory swapping
//	Used because TGA stores the RGB data in reverse order (BGR)
//----------------------------------------------------------------------
void swapRGB(unsigned char *theData, unsigned int nbRows, unsigned int nbCols)
{
	unsigned int imgSize = nbRows*nbCols;

	for(unsigned int k = 0; k < imgSize; k++)
	{
		unsigned char tmp = theData[k*3+2];
		theData[k*3+2] = theData[k*3];
		theData[k*3] = tmp;
	}
}

void swapRGBA(unsigned char *theData, unsigned int nbRows, unsigned int nbCols)
{
    unsigned int imgSize = nbRows*nbCols;

    for(unsigned int k=0; k<imgSize; k++){
        unsigned char temp = theData[k*4+2];
        theData[k*4+2] = theData[k*4];
        theData[k*4] = temp;
    }
}



// ---------------------------------------------------------------------
//	Function : readTGA
//	Description :
//
//	This function reads an image of type TGA (8 or 24 bits, uncompressed
//
//	Parameters:
//
//	*fileName - Pointer to a string that contains the name of the file
//	*nbCols, *nbRows - Dimensions XxY of the image
//
//	Return value: pointer to the pixel data
//----------------------------------------------------------------------

unsigned char* readTGA(const char* fileName, unsigned int* nbRows, unsigned int* nbCols, ImageType* theType)
{
	unsigned char *theData, *ptr;
	unsigned int imgSize;
	char	head[18] ;
	FILE	*tga_in;
	ImageFileType fileType;

	printf("Opening\n");

	/* --- open TARGA input file ---------------------------------- */
	tga_in = fopen(fileName, "rb" );

	if (tga_in == NULL)
	{
		printf("Cannot open image file\n");
		exit(1);
	}

	/* --- Read the header (TARGA file) --- */
	fread( head, sizeof(char), 18, tga_in ) ;
	/* Get the size of the image */
	*nbCols = (((unsigned int)head[12]&0xFF) | (unsigned int)head[13]*256);
	*nbRows = (((unsigned int)head[14]&0xFF) | (unsigned int)head[15]*256);


	if((head[2] == 2) && (head[16] == 24))
		*theType = RGBA32_RASTER;
	else if((head[2] == 3) && (head[16] == 8))
		*theType = GRAY_RASTER;
	else
	{
		printf("Unsuported TGA image: ");
		printf("Its type is %d and it has %d bits per pixel.\n", head[2], head[16]);
		printf("The image must be uncompressed while having 8 or 24 bits per pixel.\n");
		fclose(tga_in);
		exit(2);
	}

	imgSize = *nbCols * *nbRows;
	/* Create the buffer for image */

	if (*theType == RGBA32_RASTER)
		theData = (unsigned char*) malloc(imgSize*4);
	else
		theData = (unsigned char*) malloc(imgSize);

	if(theData == NULL)
	{
		printf("Unable to allocate memory\n");
		fclose(tga_in);
		exit(3);
	}

	/* Check if the image is vertically mirrored */
	if (*theType == RGBA32_RASTER)
	{
		if(head[17]&0x20)
		{
			ptr = theData + imgSize*3 - ((*nbCols)*3);
			for(unsigned int i = 0; i < *nbRows; i++)
			{
				fread( ptr, 3*sizeof(char), *nbCols, tga_in ) ;
				ptr -= (*nbCols)*3;
			}
		}
		else
        {
            unsigned char* dest = theData;
            for (unsigned int i=0; i<*nbRows; i++)
            {
                for (unsigned int j=0; j<*nbCols; j++)
                {
                    fread(dest, 3*sizeof(char), 1, tga_in);
                    dest+=4;
                }
            }

        }

        //  tga files store color information in the order B-G-R
        //  we need to swap the Red and Blue components
    	swapRGBA(theData, *nbRows, *nbCols);
	}
	else
	{
		if(head[17]&0x20)
		{
			ptr = theData + imgSize - *nbCols;
			for(unsigned int i = 0; i < *nbRows; i++)
			{
				fread( ptr, sizeof(char), *nbCols, tga_in ) ;
				ptr -= *nbCols;
			}
		}
		else
		fread(theData, sizeof(char), imgSize, tga_in);
	}

	fclose( tga_in) ;
	return(theData);
}


//---------------------------------------------------------------------*
//	Function : writeTGA
//	Description :
//
//	 This function write out an image of type TGA (24-bit color)
//
//	 Parameters:
//
//	  *fileName - Pointer to a string that contains the name of the file
//	  nbCols, nbRows - Dimensions XxY of the image
//	  *data - pointer to the array containing the pixels. This array
//	          is of type char {r, g, b, r, g, b, ... }
//
//	Return value: Error code (0 = no error)
//----------------------------------------------------------------------*/
int writeTGA(char* fileName, unsigned char* theData, unsigned int nbRows, unsigned int nbCols)
{
	long	offset;
	//int		swapflag = 1;
	char	head[18] ;
	FILE	*tga_out;

	/* --- open TARGA output file ---------------------------------- */

	tga_out = fopen(fileName, "wb" );

	if ( !tga_out )
	{
		printf("Cannot create pixel file %s \n", fileName);
		return 1;
	}

	// --- create the header (TARGA file) ---
	head[0]  = 0 ;		  					// ID field length.
	head[1]  = 0 ;		  					// Color map type.
	head[2]  = 2 ;		  					// Image type: true color, uncompressed.
	head[3]  = head[4] = 0 ;  				// First color map entry.
	head[5]  = head[6] = 0 ;  				// Color map lenght.
	head[7]  = 0 ;		  					// Color map entry size.
	head[8]  = head[9] = 0 ;  				// Image X origin.
	head[10] = head[11] = 0 ; 				// Image Y origin.
	head[13] = (char) (nbCols >> 8) ;		// Image width.
	head[12] = (char) (nbCols & 0x0FF) ;
	head[15] = (char) (nbRows >> 8) ;		// Image height.
	head[14] = (char) (nbRows & 0x0FF) ;
	head[16] = 24 ;		 					// Bits per pixel.
	head[17] = 0 ;		  					// Image descriptor bits ;
	fwrite( head, sizeof(char), 18, tga_out );

	for(unsigned int i = 0; i < nbRows; i++)
	{
		offset = i*4*nbCols;
		for(unsigned int j = 0; j < nbCols; j++)
		{
			fwrite(&theData[offset+2], sizeof(char), 1, tga_out);
			fwrite(&theData[offset+1], sizeof(char), 1, tga_out);
			fwrite(&theData[offset], sizeof(char), 1, tga_out);
			offset+=4;
		}
	}

	fclose( tga_out ) ;

	return 0;
}

