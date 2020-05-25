#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//Programa principal
int main(int argc, char** argv)
{

	//Definimos las variables que serán usadas al momento de compilar
	int cantidadImagenes;
	int umbralBinarizacion;
	int umbralClasificacion;
	char * nombreArchivo;
	int opt; //Variable utilizada para trabajar el getopt

	//c: Cantidad de imágenes
	//u: Umbral de binarización de la imágen
	//n: Umbral para clasificación
	//m: NOMBRE del archivo que contiene la máscara a utilizar
	while ((opt = getopt(argc, argv, "c:u:n:m:b")) != -1)
    {
        switch (opt)
        {
            case 'c':
                cantidadImagenes = atoi(optarg);

                break;
            case 'u':
                umbralBinarizacion = atoi(optarg);

                break;
            case 'n':
                umbralClasificacion = atoi(optarg);

                break;
	    case 'm':
                nombreArchivo = malloc(sizeof(char)*20); //Asignamos memoria al archivo de entrada
                strcpy(nombreArchivo, optarg);
                break;
            case 'b':
               exit(1);
	
                break;
        }
    }

}
