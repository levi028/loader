#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int GetFileSize(char *filename);
void read_src(char *path, unsigned char *buf, unsigned int size);
void make_dstfile(char *outpath, unsigned char *buf, unsigned int size);

//#define ARRAY_FILE_NAME "ali_pmu_bin_3505.h"
//#define ARRAY_FILE_NAME "ali_pmu_bin_3922.h"
//#define ARRAY_FILE_NAME "ali_pmu_bin_3921.h"
#define ARRAY_FILE_NAME "ali_pmu_bin.h"

int main(int argc,char *argv[])
{
	unsigned char *buf = NULL;
	unsigned int size;
	if(argc < 2)
		printf("Please input src file name\nC file path name\n");
	size = GetFileSize(argv[1]);
	buf = (unsigned char *)malloc(sizeof(unsigned char)*size);
	read_src(argv[1], buf, size);
	make_dstfile(ARRAY_FILE_NAME, buf, size);
	return 0;
}
unsigned int GetFileSize(char *filename)
{   
	unsigned int  siz = 0;   
	FILE  *fp = fopen(filename, "rb");   
	if (fp) 
	{      
		fseek(fp, 0, SEEK_END);      
		siz = ftell(fp);      
		fclose(fp);   
	}   
	return siz;
}

void read_src(char *path, unsigned char *buf, unsigned int size)
{
	FILE *infile;
	
	if((infile=fopen(path,"rb"))==NULL)
	{
		printf( "\nCan not open the path: %s \n", path);
		exit(-1);
	}

	fread(buf, sizeof(unsigned char), size, infile);
	
	fclose(infile);
}
void make_dstfile(char *outpath, unsigned char *buf, unsigned int size)
{
	FILE *infile;
	int i,j,k,n;
	char pbuf[10]={0};
	if((infile=fopen(outpath,"wa+"))==NULL)
	{
		printf( "\nCan not open the path: %s \n", outpath);
		exit(-1);
	}
	k=0;
	/*fwrite("unsigned char str_ali_pmu_bin[] = {\n",strlen("unsigned char str_ali_pmu_bin[] = {\n"),1,infile);*/
	fwrite("unsigned char g_ali_pmu_bin[] = {\n",strlen("unsigned char g_ali_pmu_bin[] = {\n"),1,infile);
	for(i = 0; i < size; i++)
	{
			k++;
			sprintf(pbuf,"0x%02x",buf[i]);
			fwrite(pbuf,strlen(pbuf),1,infile);
			if(k != 8)
			fwrite(", ",strlen(", "),1,infile);
			else
			fwrite(",",strlen(","),1,infile);	
		
			if(k==8)
			{
				k=0;
				fwrite("\n",strlen("\n"),1,infile);

			}
		
	}

	fseek(infile,0,SEEK_END);
	if(k == 0)
		fwrite("};",strlen("};"),1,infile);
	else
		fwrite("\n};",strlen("\n};"),1,infile);


	fclose(infile);
}

