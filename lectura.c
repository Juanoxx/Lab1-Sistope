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



matrixF *grayScale(int **pixels, int height, int width) {
  matrixF *mf = createMF(height, width);
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
	  float prom = pixels[y][x*3]*0.299+pixels[y][x*3 + 1]*0.587+pixels[y][x*3 + 2]*0.114;
	  mf = setDateMF(mf, y, x, prom);
    }
  }
  return mf;
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void)
my_error_exit (j_common_ptr cinfo){
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}
GLOBAL(matrixF*)
leerJPG(char *nombre, matrixF *mf){
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE * imagen;
	JSAMPARRAY buffer;
	int row_stride;
	if ((imagen = fopen(nombre, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", nombre);
		return 0;
	}
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(imagen);
		return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, imagen);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	unsigned char* filaPixel = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	int acum = 0;
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		for(int i = 0;i < cinfo.image_width*cinfo.num_components;i++){
			filaPixel[i + acum] = buffer[0][i];
			if (i == cinfo.image_width*cinfo.num_components - 1){
				acum = acum + i + 1;
			}
		}
	}
	int **pixels=(int**)malloc(cinfo.image_height*sizeof(int*));
	for (int k = 0; k < cinfo.image_height; k++){
		int *row=(int*)malloc(cinfo.image_width*3*sizeof(int));
		pixels[k] = row;
	}
	for (int i = 0; i < cinfo.image_height; i++){
		for(int j = 0; j < cinfo.image_width; j++)
		{
			pixels[i][j*3]=(int)filaPixel[(i*cinfo.image_width*3)+(j*3)+0];
			pixels[i][j*3 + 1]=(int)filaPixel[(i*cinfo.image_width*3)+(j*3)+1];
			pixels[i][j*3 + 2]=(int)filaPixel[(i*cinfo.image_width*3)+(j*3)+2];
			//printf("(%d,%d,%d) ",pixels[i][j*3],pixels[i][j*3 + 1],pixels[i][j*3 + 2]);
		}
		//printf("\n\n");
	}
	mf = grayScale(pixels, cinfo.image_height, cinfo.image_width);
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(imagen);
	return mf;
}
/*matrixF* leerPNG(char *nombre, matrixF *mf, int width, int height, png_byte color_type,
  png_byte bit_depth, png_bytep *row_pointers) {
  FILE *archivo = fopen(nombre, "rb");
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info = png_create_info_struct(png);
  png_init_io(png, archivo);
  png_read_info(png, info);
  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);
  png_read_update_info(png, info);
  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }
  png_read_image(png, row_pointers);
  fclose(archivo);
  png_destroy_read_struct(&png, &info, NULL);
  mf = grayScale(row_pointers, height, width);
  return mf;
}*/

int main(int argc, char *argv[]){
	matrixF *filter;
	matrixF *salida;  
	int fil, col;
	float date;
	/*png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers = NULL;*/

	char imagenArchivo[40]; /*Nombre del archivo imagen_1.png*/
	char nombreFiltroConvolucion[40]; /*filtro.txt*/
	int umbralClasificacion[1]; /*numero del umbral*/

	pid_t pid;
	int status;
	  
	int pDateMatrix[2];
	int pFilMatrix[2];
	int pColMatrix[2];
	int pDateFilter[2];
	int pFilFilter[2];
	int pColFilter[2];

	int pUmbral[2]; /*para pasar el umbral para clasificacion*/
	int pNombre[2]; /*Para pasar nombre imagen_1.png*/
	int pImagen[2]; /*para pasar la imagen de lectura*/
	/*Se crean los pipes*/
	pipe(pNombre);
	pipe(pUmbral);
	pipe(pImagen);
	pipe(pDateMatrix);
	pipe(pFilMatrix);
	pipe(pColMatrix);
	pipe(pDateFilter);
	pipe(pFilFilter);
	pipe(pColFilter);
	  
	/*Se crea el proceso hijo.*/
	pid = fork();
	  
	/*Es el padre*/
	if(pid>0){
		read(3,imagenArchivo,sizeof(imagenArchivo));
		read(4,umbralClasificacion,sizeof(umbralClasificacion));
		read(8, &fil, sizeof(fil));
		read(9, &col, sizeof(col));
		filter = createMF(fil, col);
		for (int y = 0; y < countFil(filter); y++){
			for (int x = 0; x < countColumn(filter); x++){
				read(7, &date, sizeof(date));
				filter = setDateMF( filter, y, x, date);
			}
		}
		/*read(5, &filter,2000*sizeof(filter));*/
		
		salida=leerJPG(imagenArchivo, salida);
		
		close(pNombre[0]);
		write(pNombre[1],imagenArchivo,(strlen(imagenArchivo)+1));

		close(pUmbral[0]);
		write(pUmbral[1],umbralClasificacion,sizeof(umbralClasificacion));
		
		close(pDateFilter[0]);
		close(pFilFilter[0]);
		close(pColFilter[0]);
		int filfilter = countFil(filter);
		int colfilter = countColumn(filter);
		write(pFilFilter[1], &filfilter, sizeof(filfilter));
		write(pColFilter[1], &colfilter, sizeof(colfilter));
		for (int y2 = 0; y2 < countFil(filter); y2++){
			for (int x2 = 0; x2 < countColumn(filter); x2++){
				float datefilter = getDateMF(filter, y2, x2);
				write(pDateFilter[1], &datefilter, sizeof(datefilter));
			}
		}		
		/*close(pImagen[0]);
		write(pImagen[1],salida,sizeof(matrixF));*/		
		close(pDateMatrix[0]);
		close(pFilMatrix[0]);
		close(pColMatrix[0]);
		int filmatrix = countFil(salida);
		int colmatrix = countColumn(salida);
		write(pFilMatrix[1], &filmatrix, sizeof(filmatrix));
		write(pColMatrix[1], &colmatrix, sizeof(colmatrix));
		for (int y3 = 0; y3 < countFil(salida); y3++){
			for (int x3 = 0; x3 < countColumn(salida); x3++){
				float datematrix = getDateMF(salida, y3, x3);
				write(pDateMatrix[1], &datematrix, sizeof(datematrix));
			}
		}
		waitpid(pid,&status,0);
	}
	else{ /*Es el hijo*/
		close(pNombre[1]);
		dup2(pNombre[0],3);

		close(pImagen[1]); /*Imagen resultantes de lectura*/
		dup2(pImagen[0],4);

		close(pUmbral[1]);
		dup2(pUmbral[0],5);

		close(pDateFilter[1]);
		dup2(pDateFilter[0], 7);
		close(pFilFilter[1]);
		dup2(pFilFilter[0], 8);
		close(pColFilter[1]);
		dup2(pColFilter[0], 9);
		
		close(pDateMatrix[1]);
		dup2(pDateMatrix[0], 10);
		close(pFilMatrix[1]);
		dup2(pFilMatrix[0], 11);
		close(pColMatrix[1]);
		dup2(pColMatrix[0], 12);

		char *argvHijo[] = {"bidireccionalConvolution",NULL};
		execv(argvHijo[0],argvHijo);
	}
    return 0;

}

