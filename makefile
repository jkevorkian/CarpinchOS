#este makefile no anda y esta incompleto, no logro que compile matelib como .so pq no puedo linkearle las utils.
#tambien falta implementar la regla de compilacion de los carpinchos (que los linkee con la mateLib) 
CC=gcc
RUTA_MATELIB="mateLib/src"
RUTA_UTILS=utils


build-utils:
	cd $(RUTA_UTILS) && \
	$(MAKE) clean && $(MAKE) all

build-lib: build-utils
	cd $(RUTA_MATELIB) && \
	$(CC) -c -Wall  -fpic mateLib.c && \
	$(CC) -c -Wall  -fpic sockets.c && \
	$(CC) -shared mateLib.o sockets.o -o libmatelib.so && \
	rm mateLib.o

clean:
	rm ../mateLib/src/libmate.so