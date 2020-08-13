/*Laboratorio número uno de Sistemas operativos - 1 - 2020*/
/*Integrantes: Hugo Arenas - Juan Arredondo*/
/*Profesor: Fernando Rannou*/


/*Se importan las librerías*/
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

matrixF *convertFilter(char **datefilter, int cont);

matrixF *grayScale(int **pixels, int height, int width);

matrixF *escalaGris(matrixF *mf);

matrixF *filtracion(matrixF *mf, matrixF *filter);

matrixF *binarizacion(matrixF *mf, int umbral);

GLOBAL(void) escribirJPG(char *nombre, matrixF *mf, int fil, int col);
void clasificacion(matrixF *mf, int umbral, char *namefile, int aux);

METHODDEF(void) my_error_exit (j_common_ptr cinfo);

GLOBAL(matrixF*) leerJPG(char *nombre);

/*Estructura para manejar imágenes tipo .jpg*/
struct my_error_mgr {
  struct jpeg_error_mgr pub;	
  jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr * my_error_ptr;


// Funcion main: Funcion que toma por parametros los datos entrantes y pasa a la etapa de lectura,
// La matriz del filtro para convulocion y el nombre de las imagenes.
// Entrada: los parametros ingresados por el usuario.
// Salida: Entero que representa fin de su ejecucion.

int main(int argc, char *argv[])
{
	/*Se definen los flags para que tomarán los valores ingresados por consola*/

    char *cflag = (char*)malloc(100*sizeof(char));
    char *mflag = (char*)malloc(100*sizeof(char));
    char *nflag = (char*)malloc(100*sizeof(char));
    char *uflag = (char*)malloc(100*sizeof(char));
    int cantidadImagenes=0;
	int umbralBinarizacion[1];
    int umbralClasificacion[1];
    int aux[1];
    aux[0] = 0;

    int caso;

    //c: Cantidad de imágenes
	//u: Umbral de binarización de la imágen
	//n: Umbral para clasificación
	//m: NOMBRE del archivo que contiene la máscara a utilizar

    while((caso=getopt(argc,argv, "c:u:n:m:b"))!= -1){
        switch(caso){
            case 'c':
        
                strcpy(cflag, optarg); /*Numero Cantidad imagenes*/
        		
                break;    
            case 'u':
                strcpy(uflag, optarg); /*Umbral de binarizacion*/
                break;  
            
            case 'n':
                strcpy(nflag, optarg); /*Umbral de de clasificacion*/
                break;

            case 'm':
            	strcpy(mflag,optarg);  /*Archivo del filtro laplaciano*/
        		break;

            case 'b': /*Se muestra o no por pantalla*/

                aux[0] = 1; /*Si ingresa el parámetro -b por consola, significa que muestra el resultado. Cambiando aux = 1*/
                break;
             default:
                abort(); /*En caso de ingresar mal los datos, el programa se finaliza*/
            
            
        }   
    }

	char **datefilter = (char **)malloc(2000*sizeof(char *));
	char *date = (char *)malloc(2000*sizeof(char));
	FILE *filefilter = fopen(mflag,"r");
	int error = 0, cont = 0;

	/*Se verifica que el archivo ingresado exista y sea correcto*/
	while(error == 0){
		fseek(filefilter, 0, SEEK_END);
		if ((filefilter == NULL) || (ftell(filefilter) == 0)){
			perror("Error en lectura. Ingrese el nombre de un archivo existente.\n");
			error = 1;
		}
		else{
			date=(char*)malloc(2000*sizeof(char));
			rewind(filefilter);
			while(feof(filefilter) == 0){
				date = fgets(date, 1000, filefilter);
				datefilter[cont] = date;
				date = (char*)malloc(1000*sizeof(char));
				cont = cont + 1;
			}
			error = 1;
		}
	}
	rewind(filefilter);
	fclose(filefilter);
	
	/*Se asignan valores ingresados por consola*/
    cantidadImagenes = atoi(cflag);
	umbralBinarizacion[0] = atoi(uflag);
  	umbralClasificacion[0] = atoi(nflag);

	int image = 1;

	if (aux[0])
	{
		printf("\n|     Imagen     |     Nearly Black     |\n");
	}
  	matrixF *filter = convertFilter(datefilter, cont); 
  	while(image <= cantidadImagenes){ /*Se ejecuta while miestras sea cantidadImagenes>0*/
	    char cantidadImg[10];
	    sprintf(cantidadImg,"%d",image); 
	    char imagenArchivo[] = "imagen_"; /*Archivo de entrada imagenes*/
	    char extension[] = ".jpg"; /*Extension de imagen*/
	    strcat(imagenArchivo,cantidadImg); /*imagen_1*/
	    strcat(imagenArchivo,extension); /*imagen_1.jpg*/
		matrixF *mf = leerJPG(imagenArchivo);
		mf = escalaGris(mf);
		mf = filtracion(mf,filter);
		mf = binarizacion(mf,umbralBinarizacion[0]);
		char *imagefile = (char *)malloc(1000*sizeof(char));
		strncpy(imagefile, imagenArchivo, strlen(imagenArchivo)-4);
		clasificacion(mf,umbralClasificacion[0],imagefile,aux[0]);
	    image++;
  	}/*Fin while*/
}/*Fin Main*/



/*Función que se encarga de convertir el filtro en una matriz para aplicar la convolución*/
/*Entrada: Valores de convolución y contador.*/
/* Salida: EMatriz de convolución*/

matrixF *convertFilter(char **datefilter, int cont){
	int colfilter = 1;
	for (int x = 0; x < strlen(datefilter[0]); x++){
		if ((datefilter[0][x] == '-') || (datefilter[0][x] == '0') || (datefilter[0][x] == '1') 
			|| (datefilter[0][x] == '2') || (datefilter[0][x] == '3') || (datefilter[0][x] == '4') 
		|| (datefilter[0][x] == '5') || (datefilter[0][x] == '6') || (datefilter[0][x] == '7') 
		|| (datefilter[0][x] == '8') || (datefilter[0][x] == '9')){
			colfilter = colfilter + 0;
		}
		else if (datefilter[0][x] == ' '){
			colfilter = colfilter + 1;
		}
	}
	matrixF *filter = createMF(cont, colfilter);
	int fil = 0, col = 0, pos = 0;
	char *digit = (char *)malloc(10*sizeof(char));
	for (int a = 0; a < cont; a++){
		col = 0;
		for (int b = 0; b < strlen(datefilter[a]); b++){
			if ((datefilter[a][b] == '-') || (datefilter[a][b] == '0') || (datefilter[a][b] == '1') 
			    || (datefilter[a][b] == '2') || (datefilter[a][b] == '3') || (datefilter[a][b] == '4') 
		        || (datefilter[a][b] == '5') || (datefilter[a][b] == '6') || (datefilter[a][b] == '7') 
		        || (datefilter[a][b] == '8') || (datefilter[a][b] == '9')){
			digit[pos] = datefilter[a][b];
			if (b == strlen(datefilter[a]) - 1){
				pos = 0;
				filter = setDateMF(filter, fil, col, (atoi(digit)) * 1.0000);
				col = col + 1;
				digit = (char *)malloc(10*sizeof(char));
			}
			pos = pos + 1;
			}
			else if((datefilter[a][b] == ' ') || (b == strlen(datefilter[a]) - 1)){
				pos = 0;
				filter = setDateMF(filter, fil, col, (atoi(digit)) * 1.0000);
				col = col + 1;
				digit = (char *)malloc(10*sizeof(char));
			}
		}
		fil = fil + 1;
	}
	return filter;
}

METHODDEF(void)
my_error_exit (j_common_ptr cinfo){
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

/*Función que se encarga de leer imágen en formato .jpg*/
/*Entrada: Imágen jpg.*/
/* Salida: Imagen leída, caso contrario retorna cero*/
GLOBAL(matrixF*)
leerJPG(char *nombre){
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
	matrixF *mf = createMF(cinfo.image_height, cinfo.image_width*3);
	for (int i = 0; i < cinfo.image_height; i++){
		for(int j = 0; j < cinfo.image_width; j++)
		{
			mf = setDateMF(mf,i,j*3,(float)filaPixel[(i*cinfo.image_width*3)+(j*3)+0]);
			mf = setDateMF(mf,i,j*3 + 1,(float)filaPixel[(i*cinfo.image_width*3)+(j*3)+1]);
			mf = setDateMF(mf,i,j*3 + 2,(float)filaPixel[(i*cinfo.image_width*3)+(j*3)+2]);
		}
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(imagen);
	return mf;
}

/*Función que se encarga de convertir pixeles en escala de grises*/
/*Entrada: pixeles, alto y largo.*/
/* Salida: Matriz con escala de grises*/
matrixF *escalaGris(matrixF *mf) {
	matrixF *newmf = createMF(countFil(mf), countColumn(mf)/3);
	for(int y = 0; y < countFil(newmf); y++) {
		for(int x = 0; x < countColumn(newmf); x++) {
			float prom = getDateMF(mf,y,x*3)*0.299+getDateMF(mf,y,x*3 + 1)*0.587+getDateMF(mf,y,x*3 + 2)*0.114;
			newmf = setDateMF(newmf, y, x, prom);
		}
	}
	return newmf;
}

/*Función que se encarga de realizar la convolución a la imágen e imprimirla si se introdujo -b*/
/*Entrada: imagen y filtro.*/
/*Salida: Matriz convolucionada*/
matrixF *filtracion(matrixF *mf, matrixF *filter){
	if ((countFil(filter) == countColumn(filter))&&(countFil(filter)%2 == 1)){
		int increase = 0, initial = countFil(filter);
		while (initial != 1){
			initial = initial - 2;
			increase = increase + 1;
		}
		
		matrixF *newmf = createMF(countFil(mf),countColumn(mf));
		for (int cont = 0; cont < increase; cont++){
			mf = amplifyMF(mf);
		}
		for (int fil = increase; fil < countFil(mf) - increase; fil++){
			for (int col = increase; col < countColumn(mf) - increase; col++){
				float sum = 0.0000;
				for (int y = 0; y < countFil(filter); y++){
					for (int x = 0; x < countColumn(filter); x++){
						float result = getDateMF(filter, y, x)*getDateMF(mf, y + fil - increase, x + col - increase);
						sum = sum + result;
					}
				}
				newmf = setDateMF(newmf, fil - increase, col - increase, sum);
				
			}
		}
		for (int cont2 = 0; cont2 < increase; cont2++){
			mf = decreaseMF(mf);
		}
		
		return newmf;
	}
	else{
		return mf;
	}
}

/*Función que se encarga de convertir pixeles en valores binarios (0 o 255)*/
/*Entrada: matriz y umbral.*/
/*Salida: Matriz binaria*/
matrixF *binarizacion(matrixF *mf, int umbral){
  for (int y = 0; y < countFil(mf); y++){
    for (int x = 0; x < countColumn(mf); x++){
      if (getDateMF(mf,y,x) <= umbral){
        mf = setDateMF(mf, y, x, 0.0000);
      }
      else{
        mf = setDateMF(mf, y, x, 255.0000);
      }
    }
  }
  
  return mf;
}

// Funcion: EScribe un archivo en formato png, resultante
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

	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = col; 	/* image width and height, in pixels */
	cinfo.image_height = fil;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 10, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = col * 3;	/* JSAMPLEs per row in image_buffer */
	JSAMPLE *buffer = (JSAMPLE*)malloc(fil*col*3*sizeof(JSAMPLE));
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
// Entrada: Matriz resultante desde etapa de pooling, umbral ingresado por usuario y el nombre d ela imagen.
// Salida: void
void clasificacion(matrixF *mf, int umbral, char *namefile, int aux){
	int maxBlack = 0;
	for (int y = 0; y < countFil(mf); y++){
		for (int x = 0; x < countColumn(mf); x++){
			if (getDateMF(mf, y, x) == 0.0000){
				maxBlack = maxBlack + 1;
			}
		}
	}
	float porcentBlack = (maxBlack * 100.0000)/(countFil(mf) * countColumn(mf));

	if (aux == 1)
	{
		
		if (porcentBlack >= umbral){
			printf("|   %s   |         yes        |\n",namefile);
		}
		if (porcentBlack < umbral){
			printf("|   %s   |         no         |\n",namefile);
		}

	}
	strcat(namefile,"R.jpg");
	escribirJPG(namefile, mf,countFil(mf),countColumn(mf));
}
