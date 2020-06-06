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

matrixF *escalaGris(matrixF *mf);


int main(int argc, char *argv[]){

  /*Se definen las matrices*/	
  matrixF *filter;
  matrixF *entrada;
  matrixF *salida;


  int fil, col, fil2, col2;
  float date, date2;
  char imagenArchivo[40]; /*Nombre del archivo imagen_1.png*/
  int umbralBinarizacion[1]; /*Número del umbral de binarización*/
  int umbralClasificacion[1]; /*Número del umbral de clasificación*/
  int aux[1]; /*Bandera -b*/

  pid_t pid;
  int status;

  int pDateMatrix[2];
  int pFilMatrix[2];
  int pColMatrix[2];
  int pUmbral[2]; /*para pasar el umbral para clasificacion*/
  int pUmbralB[2];
  int pNombre[2]; /*Para pasar nombre imagen_1.png*/
  int resultPantalla[2]; /*Para pasar la escritura por pantalla*/
  int pImagen[2]; /*para pasar la imagen de convolucion*/
  int pDateFilter[2];
  int pFilFilter[2];
  int pColFilter[2];

  /*Se crean los pipes*/
  pipe(pNombre);
  pipe(pUmbral);
  pipe(pUmbralB);
  pipe(pImagen);
  pipe(pDateFilter);
  pipe(pFilFilter);
  pipe(pColFilter);
  pipe(pDateMatrix);
  pipe(pFilMatrix);
  pipe(pColMatrix);
  pipe(resultPantalla);

  /*Se crea el proceso hijo.*/
  pid = fork();
  /*Es el padre*/
  if(pid>0){

    read(3,imagenArchivo,sizeof(imagenArchivo));
    read(5,umbralClasificacion,sizeof(umbralClasificacion));
	read(13,umbralBinarizacion,sizeof(umbralBinarizacion));
	read(6,aux,sizeof(aux));
	read(8, &fil, sizeof(fil));
	read(9, &col, sizeof(col));
	filter = createMF(fil, col);
	for (int y = 0; y < countFil(filter); y++){
		for (int x = 0; x < countColumn(filter); x++){
			read(7, &date, sizeof(date));
			filter = setDateMF( filter, y, x, date);
		}
	}
	read(11, &fil2, sizeof(fil2));
	read(12, &col2, sizeof(col2));
	entrada = createMF(fil2, col2);
	for (int y2 = 0; y2 < countFil(entrada); y2++){
		for (int x2 = 0; x2 < countColumn(entrada); x2++){
			read(10, &date2, sizeof(date2));
			entrada = setDateMF( entrada, y2, x2, date2);
		}
	}

	salida=escalaGris(entrada);
    /*Para pasar la imagen resultante de convolucion*/

    close(pNombre[0]);
    write(pNombre[1],imagenArchivo,(strlen(imagenArchivo)+1));
    close(pUmbral[0]);
    write(pUmbral[1],umbralClasificacion,sizeof(umbralClasificacion));
	close(pUmbralB[0]);
    write(pUmbralB[1],umbralBinarizacion,sizeof(umbralBinarizacion));
    close(resultPantalla[0]);
	write(resultPantalla[1],aux,sizeof(aux));
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

  }else{ /*Es el hijo*/
    
    close(pNombre[1]);
    dup2(pNombre[0],3);

    close(pImagen[1]);
		dup2(pImagen[0],4);

		close(pUmbral[1]);
		dup2(pUmbral[0],5);
		
		close(pUmbralB[1]);
		dup2(pUmbralB[0],13);

		close(resultPantalla[1]);
		dup2(resultPantalla[0],6);

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

		char *argvHijo[] = {"filtracion",NULL};
    	execv(argvHijo[0],argvHijo);
  	}

  return 0;
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