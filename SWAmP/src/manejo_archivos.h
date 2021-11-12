#ifndef SRC_MANEJO_ARCHIVOS_H_
#define SRC_MANEJO_ARCHIVOS_H_

#include "swamp.h"

void* crear_archivo(char*);
void rellenar_archivo(void* particion);
void escribir_archivo(void* particion, int largo, char* mensaje);
void leer_archivo(void* particion,int cantidad_caracteres);
void vaciar_archivo(void* particion,int cantidad_caracteres);

#endif /* SRC_MANEJO_ARCHIVOS_H_ */
