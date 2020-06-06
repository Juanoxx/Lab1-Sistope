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

matrixF *binarizacion(matrixF *mf, int umbral);


int main(int argc, char *argv[])
{

  /*Se definen las matrices*/ 
  matrixF *entrada;
  matrixF *salida;


  int fil, col;
  float date;
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
  int pImagen[2]; /*para pasar la imagen de rectificacion*/
  int resultPantalla[2];

  /*Se crean los pipes*/
  pipe(pNombre);
  pipe(pUmbral);
  pipe(pUmbralB);
  pipe(pImagen);
  pipe(pDateMatrix);
  pipe(pFilMatrix);
  pipe(pColMatrix);
  pipe(resultPantalla);
  
  /*Se crea el proceso hijo.*/
  pid = fork();
  /*Es el padre*/
  if(pid>0){
    read(3,imagenArchivo,sizeof(imagenArchivo));
    read(4,umbralClasificacion,sizeof(umbralClasificacion));
	  read(13,umbralBinarizacion,sizeof(umbralBinarizacion));
    read(6,aux,sizeof(aux));
	  read(8, &fil, sizeof(fil));
	  read(9, &col, sizeof(col));
	  entrada = createMF(fil, col);
	for (int y = 0; y < countFil(entrada); y++){
		for (int x = 0; x < countColumn(entrada); x++){
			read(7, &date, sizeof(date));
			entrada = setDateMF( entrada, y, x, date);
		}
	}
    salida=binarizacion(entrada,umbralBinarizacion[0]);
    
    
	close(pDateMatrix[0]);
	close(pFilMatrix[0]);
	close(pColMatrix[0]);
	int filmatrix = countFil(salida);
	int colmatrix = countColumn(salida);
	write(pFilMatrix[1], &filmatrix, sizeof(filmatrix));
	write(pColMatrix[1], &colmatrix, sizeof(colmatrix));
	for (int y2 = 0; y2 < countFil(salida); y2++){
		for (int x2 = 0; x2 < countColumn(salida); x2++){
			float datematrix = getDateMF(salida, y2, x2);
			write(pDateMatrix[1], &datematrix, sizeof(datematrix));
		}
	}

    close(pNombre[0]);
    write(pNombre[1],imagenArchivo,(strlen(imagenArchivo)+1));

    close(pUmbral[0]);
    write(pUmbral[1],umbralClasificacion,sizeof(umbralClasificacion));
    close(resultPantalla[0]);
    write(resultPantalla[1],aux,sizeof(aux));

    waitpid(pid,&status,0);

  }else{ /*Es el hijo*/

    
    close(pNombre[1]);
    dup2(pNombre[0],3);

    close(pUmbral[1]);
    dup2(pUmbral[0],4);

    close(resultPantalla[1]);
    dup2(resultPantalla[0],6);

	  close(pDateMatrix[1]);
	  dup2(pDateMatrix[0], 7);

	  close(pFilMatrix[1]);
	  dup2(pFilMatrix[0], 8);

	  close(pColMatrix[1]);
	  dup2(pColMatrix[0], 9);


    char *argvHijo[] = {"clasificacion",NULL};

    execv(argvHijo[0],argvHijo);
  }
    return 0;
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
