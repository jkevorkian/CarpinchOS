#ifndef SWAMP_H_
#define SWAMP_H_


#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "global.h"
#include "utilidades.h"
#include "manejo_archivos.h"
#include "manejo_paginas.h"

char** file_locations;
int swamp_size;
int particion_size;
int pagina_size;
int cantidad_archivos;


char* ip_swamp;


t_log* logger;
t_config* config;
typedef struct{
	int id;
	void* particion;
	char* tabla_paginas;
	//carpinchos?
}t_particion;

t_particion crear_particion(char* DIR_archivo, int numero_particion);



char itoc(int numero);


#define ERROR_CONEXION -1


#endif /* SWAMP_H_ */
