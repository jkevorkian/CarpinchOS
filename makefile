#este make compila pero no ejecuta todos los modulos del tp
#falta implementar la regla de compilacion de los carpinchos (que los linkee con la mateLib) (no se si hace falta igual)
CC=gcc
RUTA_MATELIB="mateLib/src"
RUTA_UTILS=utils
RUTA_LIBRERIAS_SISTEMA= /usr/lib
RUTA_MATELIB=/home/utnso/workspace/tp-2021-2c-AprobadOS/mateLib/src
RUTA_SWAMP=/home/utnso/workspace/tp-2021-2c-AprobadOS/SWAmP
RUTA_MEMORIA=/home/utnso/workspace/tp-2021-2c-AprobadOS/memoria
RUTA_KERNEL=/home/utnso/workspace/tp-2021-2c-AprobadOS/kernel

build-utils:
	cd $(RUTA_UTILS) && \
	$(MAKE) clean && $(MAKE) all

build-lib: build-utils
	cd $(RUTA_MATELIB) && \
	$(CC) -c -Wall  -fpic mateLib.c && \
	$(CC) -c -Wall  -fpic sockets.c && \
	$(CC) -shared mateLib.o sockets.o -o libmatelib.so && \
	rm mateLib.o && \
	cd $(RUTA_LIBRERIAS_SISTEMA) && \
	sudo rm libmatelib.so && \
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

all: build-kernel

clean:
	rm ../mateLib/src/libmate.so