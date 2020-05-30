#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <png.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jpeglib.h"
#include <setjmp.h>

GLOBAL(void)
write_JPEG_file (char * filename, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	float **matrix = (float**)malloc(cinfo.jpeg_width*sizeof(float*));
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = 15; 	/* image width and height, in pixels */
	cinfo.image_height = 15;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = 15 * 3;	/* JSAMPLEs per row in image_buffer */
	JSAMPLE *buffer = (JSAMPLE*)malloc(15*15*3*sizeof(JSAMPLE));
	int r = 255;
	int g = 125;
	int b = 63;
	unsigned char* pixel_row = (unsigned char*)(buffer);
	for (int i = 0; i < cinfo.jpeg_height; i++){
		for(int j = 0; j < cinfo.jpeg_width; j++)
		{
			pixel_row[(i*cinfo.jpeg_width*3)+(j*3)+0]=(unsigned char)r;
			pixel_row[(i*cinfo.jpeg_width*3)+(j*3)+1]=(unsigned char)g;
			pixel_row[(i*cinfo.jpeg_width*3)+(j*3)+2]=(unsigned char)b;
		}
	}
	buffer = pixel_row;
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}
GLOBAL(int)
read_JPEG_file (char * filename)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE * infile;		/* source file */
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */
	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return 0;
	}
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	unsigned char* pixel_row = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	int acum = 0;
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		for(int i = 0;i < cinfo.image_width*cinfo.num_components;i++){
			pixel_row[i + acum] = buffer[0][i];
			if (i == cinfo.image_width*cinfo.num_components - 1){
				acum = acum + i + 1;
			}
		}
	}
	int **matrix=(int**)malloc(cinfo.image_height*sizeof(int*));
	for (int k = 0; k < cinfo.image_height; k++){
		int *row=(int*)malloc(cinfo.image_width*3*sizeof(int));
		matrix[k] = row;
	}
	for (int i = 0; i < cinfo.image_height; i++){
		for(int j = 0; j < cinfo.image_width; j++)
		{
			matrix[i][j*3]=(int)pixel_row[(i*cinfo.image_width*3)+(j*3)+0];
			matrix[i][j*3 + 1]=(int)pixel_row[(i*cinfo.image_width*3)+(j*3)+1];
			matrix[i][j*3 + 2]=(int)pixel_row[(i*cinfo.image_width*3)+(j*3)+2];
			printf("(%d,%d,%d) ",matrix[i][j*3],matrix[i][j*3 + 1],matrix[i][j*3 + 2]);
		}
		printf("\n\n");
	}
	
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	return 1;
}
int main(){
	char *name = "test2.jpg";
	int quality = 15;
	write_JPEG_file (name, quality);
	char *name2 = "test.jpg";
	read_JPEG_file (name2);
	return 0;
}