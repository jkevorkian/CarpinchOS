#ifndef SWAMP_H_
#define SWAMP_H_


#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

#include "global.h"
#include "utilidades.h"
#include "manejo_archivos.h"
#include "manejo_paginas.h"
#include "manejo_carpinchos.h"

char** file_locations;
int swamp_size;
int particion_size;
int pagina_size;
int cantidad_archivos;
int marcos_maximos;
char* ip_swamp;
int* espacio_disponible;
int tipo_asignacion;


t_log* logger;
t_config* config;

char itoc(int numero);
int ctoi(char caracter);
void* algoritmo_de_particiones(void** particiones, int* espacio_disponible);

#define ERROR_CONEXION -1


#endif /* SWAMP_H_ */
