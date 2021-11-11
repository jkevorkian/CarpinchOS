#include "mem_op.h"

uint32_t mem_alloc(uint32_t id_carpincho, uint32_t tamanio) {
	return 0;
}

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t nro_pagina = pagina_segun_posicion(dir_logica - TAMANIO_HEAP);
	uint32_t offset_main = offset_segun_posicion(dir_logica - TAMANIO_HEAP);

	// t_marco* marco = obtener_marco(id_carpincho, nro_pagina);
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap))
		return false;

	// Coloco el bit esFree del heap en 1 (ahora está libre)
	set_isFree(id_carpincho, dir_logica_heap);

	// UNIFICO EL NUEVO FREE CON LOS ADYACENTES
	uint32_t pos_final_free = dir_logica;	// posicion final del nextAlloc

	// BUSCO EL ALLOC SIGUIENTE y, si corresponde, actualizo el nextAlloc del primero
	uint32_t pos_alloc_siguiente = get_nextAlloc(id_carpincho, dir_logica_heap);
	if(HEAP_NULL != pos_alloc_siguiente) {
		// t_marco* marco_siguiente = obtener_marco(id_carpincho, pagina_segun_posicion(pos_alloc_siguiente));
		// uint32_t offset_next = offset_segun_posicion(pos_alloc_siguiente);

		if(get_isFree(id_carpincho, pos_alloc_siguiente)) {
			pos_final_free = get_nextAlloc(id_carpincho, pos_alloc_siguiente);
			// debería obtener el marco inicial de vuelta porque la incorporación del segundo
			// pudo deberse a la salida del primero
			// marco = obtener_marco(id_carpincho, nro_pagina);
			set_nextAlloc(id_carpincho, dir_logica_heap, pos_final_free);
		}
		else {
			// Hacer nada, creeo
		}
	}


	// BUSCO EL ALLOC ANTERIOR y, si corresponde, le actualizo el nextAlloc
	uint32_t pos_alloc_anterior = get_prevAlloc(id_carpincho, dir_logica_heap);
	if(HEAP_NULL != pos_alloc_anterior) {
		// t_marco* marco_anterior = obtener_marco(id_carpincho, pagina_segun_posicion(pos_alloc_anterior));
		// uint32_t offset_prev = offset_segun_posicion(pos_alloc_siguiente);

		if(get_isFree(id_carpincho, pos_alloc_anterior)) {
			// debería obtener el marco inicial de vuelta porque la incorporación del segundo
			// pudo deberse a la salida del primero
			// marco = obtener_marco(id_carpincho, pagina_segun_posicion(pos_alloc_anterior));
			set_nextAlloc(id_carpincho, pos_alloc_anterior, pos_final_free);
			// soltar_marco()
		}
		else {
			// Hacer nada, creeo
		}
	}
	return true;
}

