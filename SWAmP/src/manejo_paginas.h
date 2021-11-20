#ifndef SRC_MANEJO_PAGINAS_H_
#define SRC_MANEJO_PAGINAS_H_

#include "swamp.h"

int cantidad_total_paginas;
int paginas_por_particion;

#define FIJA  0
#define GLOBAL  1

void crear_tabla(char tabla_paginas[][cantidad_total_paginas]);
void imprimir_tabla(char tabla_paginas[][cantidad_total_paginas]);
void cargar_particion_en_tabla(char tabla_paginas[][paginas_por_particion], int n_particion);
void cargar_pagina_en_tabla(char tabla_paginas[][cantidad_total_paginas],int n_particion, int id_carpincho, int n_pagina);
void guardar_pagina_en_memoria(char* mensaje, int desplazamiento, void* particion);
void leer_pagina(void* particion,int desplazamiento);
void reservar_marcos(int id_carpincho, int cantidad_marcos, char tabla_paginas[][cantidad_total_paginas], void** particiones);
char* obtener_pagina(int id_carpincho, int n_pagina,char tabla_paginas[][cantidad_total_paginas], void** particiones);

int existe_pagina(int id_carpincho, int n_pagina, int n_particion, char tabla_paginas[][cantidad_total_paginas]);
int proximo_marco_libre(char tabla_paginas[][cantidad_total_paginas],int n_particion);
int prox_marco_carpincho(char tabla_paginas[][cantidad_total_paginas],int id_carpincho);
//void reservar_marcos(int id_carpincho, int cantidad_marcos, char tabla_paginas[][cantidad_total_paginas]);
void reservar_marcos_fijo(int id_carpincho, int n_particion, char tabla_paginas[][cantidad_total_paginas]);
void reservar_marcos_global(int id_carpincho, int n_particion, int cantidad_marcos, char tabla_paginas[][cantidad_total_paginas]);
void eliminar_pagina(char tabla_paginas[][cantidad_total_paginas], int id_carpincho, int n_pagina, void** particiones);
void borrar_contenido_pagina(void* particion, int desplazamiento);

#endif /* SRC_MANEJO_PAGINAS_H_ */
