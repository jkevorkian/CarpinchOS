#ifndef SRC_MANEJO_CARPINCHOS_H_
#define SRC_MANEJO_CARPINCHOS_H_

#include "swamp.h"
int cantidad_total_paginas;

int carpincho_existente(char tabla_paginas[][cantidad_total_paginas], int id_carpincho);
int carpincho_tiene_marcos_disponibles(char tabla_paginas[][cantidad_total_paginas], int id_carpincho);
void recibir_carpincho(int id_carpincho, int n_pagina, char* mensaje_pagina, char tabla_paginas[][cantidad_total_paginas], void** particiones);
void eliminar_carpincho(int id_carpincho, char tabla_paginas[][cantidad_total_paginas], void** particiones);

#endif /* SRC_MANEJO_CARPINCHOS_H_ */
