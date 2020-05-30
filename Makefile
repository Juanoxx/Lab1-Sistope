padre: main.c lectura.c bidireccionalConvolution.c rectification.c pooling.c classification.c -ljpeg
	gcc -ggdb main.c -o pipeline -I.
	gcc -ggdb lectura.c -o lectura -ljpeg -I.
	gcc -ggdb bidireccionalConvolution.c -o bidireccionalConvolution -I.
	gcc -ggdb rectification.c -o rectification -I.
	gcc -ggdb pooling.c -o pooling -I.
	gcc -ggdb classification.c -o classification -ljpeg -I.

