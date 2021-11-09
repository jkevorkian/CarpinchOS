#ifndef SWAMP_H_
#define SWAMP_H_


#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "global.h"
#include "utilidades.h"

char** file_locations;
int swamp_size;
int particion_size;
int pagina_size;
int cantidad_archivos;
int cantidad_total_paginas;
int paginas_por_particion;
char* ip_swamp;


t_log* logger;
t_config* config;
typedef struct {
	int id;
	void* particion;
	char* tabla_paginas;
	//carpinchos?
}t_particion;

t_particion crear_particion(char* DIR_archivo, int numero_particion);
void crear_tabla_marcos(char* tabla_global);
void crear_tabla_particion(char* tabla_particion);
void* crear_archivo(char*);
void rellenar_archivo(void* particion);
void escribir_archivo(void* particion, int largo, char* mensaje);
void leer_archivo(void* particion,int cantidad_caracteres);
void vaciar_archivo(void* particion,int cantidad_caracteres);





#define ERROR_CONEXION -1


#endif /* SWAMP_H_ */
