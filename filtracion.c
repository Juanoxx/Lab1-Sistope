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


matrixF *filtracion(matrixF *mf, matrixF *filter){
	if ((countFil(filter) == countColumn(filter))&&(countFil(filter)%2 == 1)){
		int increase = 0, initial = countFil(filter);
		while (initial != 1){
			initial = initial - 2;
			increase = increase + 1;
		}
		for (int i = 0; i < countFil(mf); i++){
			for(int j = 0; j < countColumn(mf); j++)
			{
				printf("%f ",getDateMF(mf, i, j));
			}
			printf("\n");
		}
		printf("\n");
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
		for (int i = 0; i < countFil(newmf); i++){
			for(int j = 0; j < countColumn(newmf); j++)
			{
				printf("%f ",getDateMF(newmf, i, j));
			}
			printf("\n");
		}
		printf("\n");
		return newmf;
	}
	else{
		return mf;
	}
}

int main(int argc, char *argv[]){
	/* matrixf rectificacion;
	aqui iria la matriz para guardar la rectificacion*/	
	matrixF *filter;
	matrixF *entrada;
	matrixF *salida;


	int fil, col, fil2, col2;
	float date;
	char imagenArchivo[40]; /*Nombre del archivo imagen_1.png*/
	int umbralBinarizacion[1];
	int umbralClasificacion[1]; /*numero del umbral*/

	pid_t pid;
	int status;

	int pDateMatrix[2];
	int pFilMatrix[2];
	int pColMatrix[2];
	int pUmbral[2]; /*para pasar el umbral para clasificacion*/
	int pUmbralB[2];
	int pNombre[2]; /*Para pasar nombre imagen_1.png*/
	//int pFiltroConvolucion[2]; /*para pasar filtro.txt*/
	int pImagen[2]; /*para pasar la imagen de rectificacion*/
	/*Se crean los pipes*/
	//pipe(pFiltroConvolucion);
	pipe(pNombre);
	pipe(pUmbral);
	pipe(pUmbralB);
	pipe(pImagen);
	pipe(pDateMatrix);
	pipe(pFilMatrix);
	pipe(pColMatrix);
  
	/*Se crea el proceso hijo.*/
	pid = fork();
	/*Es el padre*/
	if(pid>0){
		read(3,imagenArchivo,sizeof(imagenArchivo));
		read(5,umbralClasificacion,sizeof(umbralClasificacion));
		read(15,umbralBinarizacion,sizeof(umbralBinarizacion));
		/*falta aqui read de la imagen desde convolucion*/
		/*read(5, entrada,sizeof(matrixF) );*/
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
		for (int y = 0; y < countFil(entrada); y++){
			for (int x = 0; x < countColumn(entrada); x++){
				read(10, &date, sizeof(date));
				entrada = setDateMF( entrada, y, x, date);
			}
		}
		salida=filtracion(entrada,filter);
    
    
		/*Para pasar la imagen resultante de rectification*/
		/* close(pImagen[0]);
		write(pImagen[1],salida,sizeof(matrixF));*/
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
		close(pUmbralB[0]);
		write(pUmbralB[1],umbralBinarizacion,sizeof(umbralBinarizacion));

		waitpid(pid,&status,0);

	}
	else{ /*Es el hijo*/

    
		close(pNombre[1]);
		dup2(pNombre[0],3);

		close(pUmbral[1]);
		dup2(pUmbral[0],4);
		close(pUmbralB[1]);
		dup2(pUmbralB[0],15);

		close(pDateMatrix[1]);
		dup2(pDateMatrix[0], 7);
		close(pFilMatrix[1]);
		dup2(pFilMatrix[0], 8);
		close(pColMatrix[1]);
		dup2(pColMatrix[0], 9);


		//char *argvHijo[] = {"pooling",NULL};
		char *argvHijo[] = {"binarizacion",NULL};
		execv(argvHijo[0],argvHijo);
	}
    return 0;
}
