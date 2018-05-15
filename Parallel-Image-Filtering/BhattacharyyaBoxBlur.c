////////////////////////////////////////////////////////////////////////////////
//INCLUDES
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

#pragma warning(disable: 4996)

//problem assumptions
#define BMP_HEADER_SIZE_BYTES 14
#define BMP_DIB_HEADER_SIZE_BYTES 40
#define MAXIMUM_IMAGE_SIZE 256

//bmp compression methods
//none:
#define BI_RGB 0

////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTURES

struct BMP_Header {
	char signature[2];		//ID field
	int size;		//Size of the BMP file
	short reserved1;		//Application specific
	short reserved2;		//Application specific
	int offset_pixel_array;  //Offset where the pixel array (bitmap data) can be found
};

//40 bytes information header
typedef struct information_header
{
    int size;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    int compression;
    int imgSizeBytes;
    int pixPerMeterHorizontal;
    int pixPerMeterVertcal;
    int colorUsed;
    int colorImp;
}BM_INFO_HEADER;

//Structure to load bmp file colour pixels
typedef struct 
{
   unsigned char r,g,b;
}COLOURINDEX;

COLOURINDEX colourindex[MAXIMUM_IMAGE_SIZE][MAXIMUM_IMAGE_SIZE];
COLOURINDEX blur_colourindex[MAXIMUM_IMAGE_SIZE][MAXIMUM_IMAGE_SIZE];

typedef enum colours
{
	BLUE = 0,
	GREEN = 1,
	RED = 2
}COLOR;

unsigned char blur_algo(int,int,COLOR);		//Blur algorithm based on Stencil operation

//global data to hold height & width of the image data
int g_height = 0;
int g_width = 0;

//Thread functions

void* l_upper_blur (void* unused)
{
	int i,j;
	int height = (g_height + 1)/2;
	int width = (g_width + 1)/2;
	for (j = 0;j < height;j++) 
	{
      	for (i = 0;i < width;i++)
		{
			blur_colourindex[i][j].b = blur_algo(i,j,BLUE);
			blur_colourindex[i][j].g = blur_algo(i,j,GREEN);
			blur_colourindex[i][j].r = blur_algo(i,j,RED);
		}
	}
}

void* l_lower_blur (void* unused)
{
	int i,j;
	int height = g_height;
	int width = (g_width + 1)/2;
	for (j = (g_height/2);j < height;j++) 
	{
      	for (i = 0;i < width;i++)
		{
			blur_colourindex[i][j].b = blur_algo(i,j,BLUE);
			blur_colourindex[i][j].g = blur_algo(i,j,GREEN);
			blur_colourindex[i][j].r = blur_algo(i,j,RED);
		}
	}
}

void* r_upper_blur (void* unused)
{
	int i,j;
	int height = (g_height + 1)/2;
	int width = g_width;
	for (j = 0;j < height;j++) 
	{
      	for (i = (g_width/2);i < width; i++)
		{
			blur_colourindex[i][j].b = blur_algo(i,j,BLUE);
			blur_colourindex[i][j].g = blur_algo(i,j,GREEN);
			blur_colourindex[i][j].r = blur_algo(i,j,RED);
		}
	}
}

void* r_lower_blur (void* unused)
{
	int i,j;
	int height = g_height;
	int width = g_width;
	for (j = (g_height/2);j < height;j++) 
	{
      	for (i = (g_width/2);i < width;i++)
		{
			blur_colourindex[i][j].b = blur_algo(i,j,BLUE);
			blur_colourindex[i][j].g = blur_algo(i,j,GREEN);
			blur_colourindex[i][j].r = blur_algo(i,j,RED);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE


void main(int argc, char* argv[]) {


	//sample code to read first 14 bytes of BMP file format
	FILE* file = fopen("test1.bmp", "rb");
	if(NULL == file)
	{
		printf("\nCannot open the file\n");
		exit(1);
	}
	struct BMP_Header header;
	BM_INFO_HEADER infoheader;
        int i,j;

	//read bitmap file header (14 bytes)
	fread(&header.signature, sizeof(char)*2, 1, file);
	fread(&header.size, sizeof(int), 1, file);
	fread(&header.reserved1, sizeof(short), 1, file);
	fread(&header.reserved2, sizeof(short), 1, file);
	fread(&header.offset_pixel_array, sizeof(int), 1, file);

	if('B' != header.signature[0] || 'M' != header.signature[1])
	{
		printf("\nUnsupported file format\nSupported format is 24 bits per pixel bmp image\n");
		exit(1);
	}

	printf("signature: %c%c\n", header.signature[0], header.signature[1]);
	printf("size: %d\n", header.size);
	printf("reserved1: %d\n", header.reserved1);
	printf("reserved2: %d\n", header.reserved2);
	printf("offset_pixel_array: %d\n", header.offset_pixel_array);

	fread(&infoheader,sizeof(BM_INFO_HEADER),1,file);
	
	if(24 != infoheader.bitsPerPixel)
	{
		printf("\nUnsupported BMP file. \nSupported version 24bit per pixel bmp image\n");
		exit(1);
	}
	
	printf("Image size = %d x %d\n",infoheader.width,infoheader.height);
   
	g_height = infoheader.height;
	g_width = infoheader.width;
   //read image data
   fseek(file,header.offset_pixel_array,SEEK_SET);
   
    for (j=0;j<infoheader.height;j++) 
	{
      for (i=0;i<infoheader.width;i++)
	  {
			fread(&colourindex[i][j].b,sizeof(unsigned char),1,file);
 	        fread(&colourindex[i][j].g,sizeof(unsigned char),1,file);
        	fread(&colourindex[i][j].r,sizeof(unsigned char),1,file);
	  }
	}
	fclose(file);
	
	//Geometric decomposition -> 4 threads created to act on data
	pthread_t l_upper_tid,l_lower_tid,r_upper_tid,r_lower_tid;
	
	pthread_create(&l_upper_tid, NULL, &l_upper_blur, NULL);
        pthread_join(l_upper_tid, NULL);
	
	pthread_create(&l_lower_tid, NULL, &l_lower_blur, NULL);
        pthread_join(l_lower_tid, NULL);
	
	pthread_create(&r_upper_tid, NULL, &r_upper_blur, NULL);
        pthread_join(r_upper_tid, NULL);
	
	pthread_create(&r_lower_tid, NULL, &r_lower_blur, NULL);
        pthread_join(r_lower_tid, NULL);
	
	FILE* fp = fopen("out.bmp", "wb");
	if(NULL == fp)
	{
		printf("\nCannot create the file\n");
		exit(1);
	}
	fwrite(&header.signature, sizeof(char)*2, 1, fp);
	fwrite(&header.size, sizeof(int), 1, fp);
	fwrite(&header.reserved1, sizeof(short), 1, fp);
	fwrite(&header.reserved2, sizeof(short), 1, fp);
	fwrite(&header.offset_pixel_array, sizeof(int), 1, fp);
    fwrite(&infoheader,sizeof(BM_INFO_HEADER),1,fp);
	for (j=0;j < infoheader.height;j++) 
	{
	  for (i=0;i<infoheader.width;i++)
	  {
		fwrite(&blur_colourindex[i][j].b,sizeof(unsigned char),1,fp);
 	    fwrite(&blur_colourindex[i][j].g,sizeof(unsigned char),1,fp);
       	fwrite(&blur_colourindex[i][j].r,sizeof(unsigned char),1,fp);
	  }
	}
	fclose(fp);
	printf("\nBlurred image created..... \nCheck out.bmp file in the same folder\n");
}

unsigned char blur_algo(int i, int j, COLOR colour)
{
	int q,r;
	switch(colour)
	{
		case BLUE:
		{
			int sum = 0;
			int count = 0;
			int avg = 0;
			for(q = i-1; q <= i+1 ; q++)
			{
				if(q < 0 || q >= g_height)
				{
					continue;
				}
				for(r = j-1; r <= j+1; r++)
				{
					if(r < 0 || r >= g_width)
					{
						continue;
					}
					sum = sum + colourindex[q][r].b;
					count++;
				}
			}
			if(count)
			{
				avg = sum/count;
			}
			return avg;
		}
		break;
			
		case GREEN:
		{
			int sum = 0;
			int count = 0;
		    int avg = 0;
			for(q = i-1; q <= i+1 ; q++)
			{
				if(q < 0 || q >= g_height)
					continue;
				for(r = j-1; r<= j+1; r++)
				{
					if(r < 0 || r >= g_width)
					{
						continue;
					}
					sum = sum + colourindex[q][r].g;
					count++;
				}
			}
			if(count)
			{
				avg = sum/count;
			}
			return avg;
		}
		break;
			
		case RED:
		{
			int sum = 0;
			int count = 0;
			int avg = 0;
			for(q = i-1; q <= i+1 ; q++)
			{
				if(q < 0 || q >= g_height)
				{
					continue;
				}
				for(r = j-1; r<= j+1; r++)
				{
					if(r < 0 || r >= g_width)
					{
						continue;
					}
					sum = sum + colourindex[q][r].r;
					count++;
				}
			}
			if(count)
			{
				avg = sum/count;
			}
			return avg;
		}
		break;
			
		default:
		{
			//do nothing
		}
	}
}
