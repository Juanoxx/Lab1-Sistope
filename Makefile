padre: main.c lectura.c conversion.c filtracion.c binarizacion.c clasificacion.c -ljpeg
	gcc -ggdb -Wall main.c -o pipeline -I.
	gcc -ggdb -Wall lectura.c -o lectura -ljpeg -I.
	gcc -ggdb -Wall conversion.c -o conversion -I.
	gcc -ggdb -Wall filtracion.c -o filtracion -I.
	gcc -ggdb -Wall binarizacion.c -o binarizacion -I.
	gcc -ggdb -Wall clasificacion.c -o clasificacion -ljpeg -I.

