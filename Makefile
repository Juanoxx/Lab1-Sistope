padre: main.c lectura.c bidireccionalConvolution.c rectification.c pooling.c classification.c -ljpeg
	gcc main.c -o pipeline -I.
	gcc lectura.c -o lectura -ljpeg -I.
	gcc bidireccionalConvolution.c -o bidireccionalConvolution -I.
	gcc rectification.c -o rectification -I.
	gcc pooling.c -o pooling -I.
	gcc classification.c -o classification -ljpeg -I.

