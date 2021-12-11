#este make compila pero no ejecuta todos los modulos del tp
#falta implementar la regla de compilacion de los carpinchos (que los linkee con la mateLib) (no se si hace falta igual)
CC=gcc

RUTA_MATELIB=/home/utnso/tp-2021-2c-AprobadOS/mateLib/src
RUTA_UTILS=utils
RUTA_LIBRERIAS_SISTEMA= /usr/lib
RUTA_SWAMP=/home/utnso/tp-2021-2c-AprobadOS/SWAmP
RUTA_MEMORIA=/home/utnso/tp-2021-2c-AprobadOS/memoria
RUTA_KERNEL=/home/utnso/tp-2021-2c-AprobadOS/kernel
RUTA_RAIZ_PROYECTO=/home/utnso/tp-2021-2c-AprobadOS

build-utils:
	cd $(RUTA_UTILS) && \
	$(MAKE) clean && $(MAKE) all

build-lib: build-utils
	cd $(RUTA_MATELIB) && \
	$(CC) -c -Wall  -fpic mateLib.c && \
	$(CC) -c -Wall  -fpic sockets.c && \
	$(CC) -shared mateLib.o sockets.o -o libmatelib.so && \
	rm mateLib.o sockets.o && \
	cd $(RUTA_LIBRERIAS_SISTEMA) && \
	sudo rm -f libmatelib.so && \
	sudo mv $(RUTA_MATELIB)/libmatelib.so  $(RUTA_LIBRERIAS_SISTEMA)

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