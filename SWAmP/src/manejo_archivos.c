#include "manejo_archivos.h"

void* crear_archivo(char* DIR_archivo){

	int file = open(DIR_archivo, O_RDWR | O_CREAT);
	if(file == -1){
		printf("No se pudo generar el archivo\n");
	}
	void* particion = mmap(NULL, particion_size , PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
	truncate(DIR_archivo, particion_size);
	rellenar_archivo(particion);
	return particion;
}
void rellenar_archivo(void* particion){
	int desplazamiento = 0;
	log_info(logger, "Rellenando archivo...");
	for(desplazamiento = 0; desplazamiento < particion_size; desplazamiento++){
		memcpy(particion + desplazamiento, "0", sizeof(char));
	}
	log_info(logger, "Archivo rellenado con exito");
}
void escribir_archivo(void* particion, int largo, char* mensaje){
	log_info(logger, "Escribiendo archivo...");
	int desplazamiento = 0;
	while(largo > 0){
		memcpy(particion + desplazamiento, string_from_format("%c",mensaje[desplazamiento]), sizeof(char));
		desplazamiento++;
		largo--;
	}
	log_info(logger, "Archivo escrito");
}
void leer_archivo(void* particion,int cantidad_caracteres){
	log_info(logger, "Leyendo archivo...");
	char mensaje[cantidad_caracteres+1];
	char* p = mensaje;
	memcpy(p, particion, cantidad_caracteres);
	mensaje[cantidad_caracteres+1] = '\0';
	printf("el archivo contiene: %s \n",mensaje);

}
void vaciar_archivo(void* particion,int cantidad_caracteres){
	log_info(logger, "Vaciando archivo...");
	int desplazamiento = 0;
	while(cantidad_caracteres > 0){
		memcpy(particion + desplazamiento, "0", sizeof(char));
		desplazamiento++;
		cantidad_caracteres--;
	}

}
int obtener_n_particion(void** partiones, void* particion){
	int i;
	for(i = 0; i < cantidad_archivos; i++){
		if(partiones[i] == particion){
			return i;
		}
	}
	return -1;
}
