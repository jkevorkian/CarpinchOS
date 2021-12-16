CC=gcc

RUTA_MATELIB=mateLib/src
RUTA_UTILS=utils
RUTA_LIBRERIAS_SISTEMA=/usr/lib
RUTA_SWAMP=SWAmP
RUTA_MEMORIA=memoria
RUTA_KERNEL=kernel
RUTA_RAIZ_PROYECTO=$(shell pwd)
PROYECTO=$(shell basename $(RUTA_RAIZ_PROYECTO))
LIBRERIA=matelib

build-utils:
	cd $(RUTA_UTILS) && \
	$(MAKE) clean && $(MAKE) all

build-lib: build-utils
	cd $(RUTA_MATELIB) && \
	$(CC) -c -Wall  -fpic mateLib.c && \
	$(CC) -c -Wall  -fpic sockets.c && \
	$(CC) -shared mateLib.o sockets.o -o $(RUTA_LIBRERIAS_SISTEMA)/lib$(LIBRERIA).so && \
	rm mateLib.o sockets.o

build-swamp: build-lib
	cd $(RUTA_SWAMP) && \
	$(MAKE) clean && $(MAKE) all

build-memoria: build-swamp
	cd $(RUTA_MEMORIA) && \
	$(MAKE) clean && $(MAKE) all

build-kernel: build-memoria
	cd $(RUTA_KERNEL) && \
	$(MAKE) clean && $(MAKE) all

compile: build-lib

#las siguientes reglas corren los respectivos modulos una vez que ya fueron compilados. SOLO ANDA LA DEL SWAMP E IGUALMENTE CREA UN LOG INNECESARIO EN LA CARPETA RAIZ

swamp: 
	cd $(RUTA_RAIZ_PROYECTO) && \
	cd SWAmP && \
	./bin/SWAmP

memoria: 
	cd $(RUTA_RAIZ_PROYECTO) && \
	cd memoria && \
	./bin/memoria

kernel: 
	cd $(RUTA_RAIZ_PROYECTO) && \
	cd kernel && \
	./bin/kernel


clean:
	rm ../mateLib/src/libmate.so
