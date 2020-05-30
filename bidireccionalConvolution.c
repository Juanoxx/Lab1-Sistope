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


matrixF *bidirectionalConvolution(matrixF *mf, matrixF *filter){
	if ((countFil(filter) == countColumn(filter))&&(countFil(filter)%2 == 1)){
		int increase = 0, initial = countFil(filter);
		while (initial != 1){
			initial = initial - 2;
			increase = increase + 1;
		}
		for (int cont = 0; cont < increase; cont++){
			mf = amplifyMF(mf);
		}
		for (int fil = 0; fil < countFil(filter) - countFil(filter); fil++){
			for (int col = 0; col < countColumn(filter) - countColumn(filter); col++){
				float sum = 0.0000;
				for (int y = 0; y < countFil(filter); y++){
					for (int x = 0; x < countColumn(filter); x++){
						float result = getDateMF(mf, y, x)*getDateMF(filter, y + fil, x + col);
						sum = sum + result;
					}
				}
				mf = setDateMF(mf, fil + (countFil(filter)/2), col + (countColumn(filter)/2), sum);
			}
		}
		for (int cont2 = 0; cont2 < increase; cont2++){
			mf = decreaseMF(mf);
		}
		return mf;
	}
	else{
		return mf;
	}
}



int main(int argc, char *argv[]){
  /*matrixf convolucion;
  aqui iria la matriz para guardar la convolucion*/	
  matrixF *filter;
  matrixF *entrada;
  matrixF *salida;


  int fil, col, fil2, col2;
  float date, date2;
  char imagenArchivo[40]; /*Nombre del archivo imagen_1.png*/
  //char nombreFiltroConvolucion[40]; /*filtro.txt*/
  int umbralClasificacion[1]; /*numero del umbral*/

  pid_t pid;
  int status;

  int pDateMatrix[2];
  int pFilMatrix[2];
  int pColMatrix[2];
  int pUmbral[2]; /*para pasar el umbral para clasificacion*/
  int pNombre[2]; /*Para pasar nombre imagen_1.png*/
  //int pFiltroConvolucion[2]; /*para pasar filtro.txt*/
  int pImagen[2]; /*para pasar la imagen de convolucion*/
  /*Se crean los pipes*/
  //pipe(pFiltroConvolucion); No se tiene que pasar, porque aqui se ocupa
  pipe(pNombre);
  pipe(pUmbral);
  pipe(pImagen);
  pipe(pDateMatrix);
  pipe(pFilMatrix);
  pipe(pColMatrix);
  
  /*Se crea el proceso hijo.*/
  pid = fork();
  
  /*Es el padre*/
  if(pid>0){

    read(3,imagenArchivo,sizeof(imagenArchivo));
   /*read(4, entrada, sizeof(entrada));*/
    /*falta aqui read de la imagen desde lectura desde 4*/
    read(5,umbralClasificacion,sizeof(umbralClasificacion));
    /*read(6, filter,sizeof(filter));*/
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
    salida=bidirectionalConvolution(entrada,filter);
    
    
    /*Para pasar la imagen resultante de convolucion*/

    close(pNombre[0]);
    write(pNombre[1],imagenArchivo,(strlen(imagenArchivo)+1));
    close(pUmbral[0]);
    write(pUmbral[1],umbralClasificacion,sizeof(umbralClasificacion));

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

  }else{ /*Es el hijo*/
    
    close(pNombre[1]);
    dup2(pNombre[0],3);

    close(pUmbral[1]);
    dup2(pUmbral[0],4);

    /*Para que el hijo lea desde 5, la iamgen de convolucion*/
    close(pImagen[1]);
    dup2(pImagen[0],5);
	
	close(pDateMatrix[1]);
	dup2(pDateMatrix[0], 7);
	close(pFilMatrix[1]);
	dup2(pFilMatrix[0], 8);
	close(pColMatrix[1]);
	dup2(pColMatrix[0], 9);



    char *argvHijo[] = {"rectification",NULL};
    execv(argvHijo[0],argvHijo);
  }
    return 0;

}

