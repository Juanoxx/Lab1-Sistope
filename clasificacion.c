#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "matrixf.h"
#include "jpeglib.h"
#include <setjmp.h>

// Funcion: EScribe un archivo en formato png, resultante
// 
// Entrada: en nombre del archivo y lamatriz resultante.
// Salida: void
GLOBAL(void)
escribirJPG(char *nombre, matrixF *mf, int fil, int col){
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	if ((outfile = fopen(nombre, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", nombre);
		exit(1);
	}
	float **matrix = (float**)malloc(cinfo.jpeg_width*sizeof(float*));
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = col; 	/* image width and height, in pixels */
	cinfo.image_height = fil;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 10, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = col * 3;	/* JSAMPLEs per row in image_buffer */
	JSAMPLE *buffer = (JSAMPLE*)malloc(15*15*3*sizeof(JSAMPLE));
	unsigned char* pixel_row = (unsigned char*)(buffer);
	for (int i = 0; i < cinfo.jpeg_height; i++){
		for(int j = 0; j < cinfo.jpeg_width; j++)
		{
			pixel_row[(i*cinfo.jpeg_width*3)+(j*3)+0]=(unsigned char)((int)getDateMF(mf, i, j));
			pixel_row[(i*cinfo.jpeg_width*3)+(j*3)+1]=(unsigned char)((int)getDateMF(mf, i, j));
			pixel_row[(i*cinfo.jpeg_width*3)+(j*3)+2]=(unsigned char)((int)getDateMF(mf, i, j));
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

// Funcion: Permite clasificar una imagene de acuerdo a un umbral
// 
// Entrada: Matriz resultante desde etapa de pooling, umbral ingresado por usuario y el nombre d ela imagen.
// Salida: void

void clasificacion(matrixF *mf, int umbral, char *namefile){
	int maxBlack = 0;
	for (int y = 0; y < countFil(mf); y++){
		for (int x = 0; x < countColumn(mf); x++){
			if (getDateMF(mf, y, x) == 0.0000){
				maxBlack = maxBlack + 1;
			}
		}
	}
	float porcentBlack = (maxBlack * 100.0000)/(countFil(mf) * countColumn(mf));
	if (porcentBlack >= umbral){
		printf("|   %s   |         yes        |\n",namefile);
	}
	if (porcentBlack < umbral){
		printf("|   %s   |         no         |\n",namefile);
	}
	strcat(namefile,"R.jpg");
	escribirJPG(namefile, mf,countFil(mf),countColumn(mf));
}


int main(int argc, char *argv[]){
  /* matrixf clasfication;
  aqui iria la matriz para guardar el clasification*/ 
  matrixF *entrada;
  matrixF *salida;
  
  int fil, col;
  float date;

  char imagenArchivo[40]; /*Nombre del archivo imagen_1.png*/
  int umbralClasificacion[1]; /*numero del umbral*/

  pid_t pid;
  int status;


  int pUmbral[2]; /*para pasar el umbral para clasificacion*/
  int pNombre[2]; /*Para pasar nombre imagen_1.png*/
  //int pFiltroConvolucion[2]; /*para pasar filtro.txt*/
  int pImagen[2]; /*para pasar la imagen de pooling*/
  /*Se crean los pipes*/
  //pipe(pFiltroConvolucion);
  pipe(pUmbral);
  pipe(pNombre);
  pipe(pImagen);

  read(3,imagenArchivo,sizeof(imagenArchivo));
  read(4,umbralClasificacion,sizeof(umbralClasificacion));
  /*falta aqui read de la imagen desde pooling*/
  /*read(5, entrada,sizeof(matrixF) );*/
  read(8, &fil, sizeof(fil));
  read(9, &col, sizeof(col));
  entrada = createMF(fil, col);
  for (int y = 0; y < countFil(entrada); y++){
	for (int x = 0; x < countColumn(entrada); x++){
		read(7, &date, sizeof(date));
		entrada = setDateMF( entrada, y, x, date);
	}
  }
  char *imagefile = (char *)malloc(1000*sizeof(char));
  strncpy(imagefile, imagenArchivo, strlen(imagenArchivo) - 4);
  clasificacion(entrada, umbralClasificacion[0],imagefile);
  return 0;
}