padre: main.c lectura.c bidireccionalConvolution.c rectification.c pooling.c classification.c -ljpeg
	gcc -ggdb -Wall main.c -o pipeline -I.
	gcc -ggdb -Wall lectura.c -o lectura -ljpeg -I.
	gcc -ggdb -Wall bidireccionalConvolution.c -o bidireccionalConvolution -I.
	gcc -ggdb -Wall rectification.c -o rectification -I.
	gcc -ggdb -Wall pooling.c -o pooling -I.
	gcc -ggdb -Wall classification.c -o classification -ljpeg -I.

