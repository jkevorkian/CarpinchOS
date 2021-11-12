#include "manejo_archivos.h"

void* crear_archivo(char* DIR_archivo){

	int file = open(DIR_archivo, O_RDWR | O_CREAT);
	if(file == -1){
		printf("No se pudo generar el archivo\n");
	}

	void* particion = mmap(NULL, particion_size , PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
	truncate(DIR_archivo, particion_size);

	rellenar_archivo(particion);
	leer_archivo(particion,10);
	//escribir_archivo(particion,16,"Hola Juan Carlos");
	//leer_archivo(particion,15);
	//vaciar_archivo(particion,7);
	//leer_archivo(particion,20);
	return particion;
}
void rellenar_archivo(void* particion){
	int desplazamiento = 0;
	int caracteres_por_escribir = particion_size;
	log_info(logger, "Rellenando archivo...");
	while(caracteres_por_escribir > 0){
		memcpy(particion + desplazamiento, "\0", sizeof(char));
		desplazamiento++;
		caracteres_por_escribir--;
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
	memcpy(p, particion, cantidad_caracteres+1);
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

