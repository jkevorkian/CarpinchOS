#ifndef SRC_MANEJO_PAGINAS_H_
#define SRC_MANEJO_PAGINAS_H_

#include "swamp.h"

int cantidad_total_paginas;
int paginas_por_particion;

void crear_tabla(char tabla_paginas[][cantidad_total_paginas]);
void imprimir_tabla(char tabla_paginas[][cantidad_total_paginas]);
void cargar_particion_en_tabla(char tabla_paginas[][paginas_por_particion], int n_particion);
void cargar_pagina_en_tabla(char tabla_paginas[][cantidad_total_paginas],int n_particion, int id_carpincho, int n_pagina);

void guardar_pagina_en_memoria(char* mensaje, int desplazamiento, void* particion);
void leer_pagina(void* particion,int desplazamiento);

#endif /* SRC_MANEJO_PAGINAS_H_ */
