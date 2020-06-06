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
  	
  	while(image <= cantidadImagenes){ /*Se ejecuta while miestras sea cantidadImagenes>0*/
		matrixF *filter = convertFilter(datefilter, cont); 
		int status;
		int pDateMatrix[2];
		int pFilMatrix[2];
		int pColMatrix[2];
		int pUmbral[2];
		int pUmbralB[2];
		int pNombre[2];
		int resultPantalla[2];
	    char cantidadImg[10];
	    sprintf(cantidadImg,"%d",image); 
	    char imagenArchivo[] = "imagen_"; /*Archivo de entrada imagenes*/
	    char extension[] = ".jpg"; /*Extension de imagen*/
	    strcat(imagenArchivo,cantidadImg); /*imagen_1*/
	    strcat(imagenArchivo,extension); /*imagen_1.jpg*/
	    
	    /*Se crean los pipes*/
	    pipe(pNombre); /*Para pasar el nombreImagen.*/
	    pipe(pUmbral); /*Para pasar el umbral para clasificacion.*/
		pipe(pUmbralB);
	    pipe(pDateMatrix);
		pipe(pFilMatrix);
		pipe(pColMatrix);
		pipe(resultPantalla);
	    /*Se crea el proceso hijo*/

	    pid_t pid = fork();
	    /*Mayor que 0 es el PADRE*/
	    if(pid>0){

	    	close(pNombre[0]); /*Se cierra la lectura*/
	      	write(pNombre[1],imagenArchivo,(strlen(imagenArchivo)+1));
	      	close(pUmbral[0]); /*Se cierra la lectura*/
	      	write(pUmbral[1],umbralClasificacion,sizeof(umbralClasificacion));
			close(pUmbralB[0]); /*Se cierra la lectura*/
	      	write(pUmbralB[1],umbralBinarizacion,sizeof(umbralBinarizacion));
	      	close(resultPantalla[0]);
	      	write(resultPantalla[1],aux,sizeof(aux));
			close(pDateMatrix[0]);
			close(pFilMatrix[0]);
			close(pColMatrix[0]);
			int filmatrix = countFil(filter);
			int colmatrix = countColumn(filter);
			write(pFilMatrix[1], &filmatrix, sizeof(filmatrix));
			write(pColMatrix[1], &colmatrix, sizeof(colmatrix));
			for (int y = 0; y < countFil(filter); y++){
				for (int x = 0; x < countColumn(filter); x++){
					float datematrix = getDateMF(filter, y, x);
					write(pDateMatrix[1], &datematrix, sizeof(datematrix));
				}
			}
	      	waitpid(pid,&status,0);

	    }else{/*Es hijo*/

	      	close(pNombre[1]);
	      	dup2(pNombre[0],3);

	      	close(pUmbral[1]);
	      	dup2(pUmbral[0],4);

			close(pUmbralB[1]);
	      	dup2(pUmbralB[0],5);

			close(resultPantalla[1]);
			dup2(resultPantalla[0],6);

			close(pDateMatrix[1]);
	      	dup2(pDateMatrix[0], 7);

			close(pFilMatrix[1]);
	      	dup2(pFilMatrix[0], 8);

			close(pColMatrix[1]);
	      	dup2(pColMatrix[0], 9);

	      	char *argvHijo[] = {"lectura",NULL}; /*Nombre del archivo al cual pasara el hijo*/
	      	execv(argvHijo[0],argvHijo); /*Reemplaza el codigo del proceso, por el cual apunta argvHijo*/
	    }
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

