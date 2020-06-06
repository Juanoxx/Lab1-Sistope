padre: main.c lectura.c conversion.c filtracion.c binarizacion.c clasificacion.c -ljpeg
	gcc -Wall main.c -o pipeline -I.
	gcc -Wall lectura.c -o lectura -ljpeg -I.
	gcc -Wall conversion.c -o conversion -I.
	gcc -Wall filtracion.c -o filtracion -I.
	gcc -Wall binarizacion.c -o binarizacion -I.
	gcc -Wall clasificacion.c -o clasificacion -ljpeg -I.

