#include "mem_op.h"

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
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

		if(get_isFree(id_carpincho, pos_alloc_siguiente)) {
			pos_final_free = get_nextAlloc(id_carpincho, pos_alloc_siguiente);
			set_nextAlloc(id_carpincho, dir_logica_heap, pos_final_free);
		}
		else {
			// Hacer nada, creeo
		}
	}

	// BUSCO EL ALLOC ANTERIOR y, si corresponde, le actualizo el nextAlloc
	uint32_t pos_alloc_anterior = get_prevAlloc(id_carpincho, dir_logica_heap);
	if(HEAP_NULL != pos_alloc_anterior) {

		if(get_isFree(id_carpincho, pos_alloc_anterior)) {
			set_nextAlloc(id_carpincho, pos_alloc_anterior, pos_final_free);
		}
		else {
			// Hacer nada, creeo
		}
	}
	return true;
}

uint32_t mem_alloc(uint32_t carpincho, uint32_t tamanio) {
	return 0;
}

/*
uint32_t mem_alloc(t_carpincho* carpincho, uint32_t tamanio) {
	log_info(logger, "El proceso #%d solicito %d bytes de memoria.", carpincho->id, tamanio);
	uint32_t nro_frames_asignados = 0;
	uint32_t nro_frames_necesarios = cant_frames_necesarios(tamanio);
	uint32_t dir_logica = 0;

    while(nro_frames_necesarios > nro_frames_asignados) {
		t_marco* marco = obtener_marco_libre();

		if(marco == NULL) { 
			nuevo_marco = pedir_frame_swap(); 
			if(nuevo_marco == NULL) { 
				log_info(logger, "No se pudo asignar memoria");
				return NULL;
			}
		};

		t_entrada_tp* nueva_pagina = crear_nueva_pagina(marco->nro_real);
		// list_add(carpincho->tabla_paginas, nueva_pagina);

		// log_info(logger, "Asigno frame. Cant frames del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
		log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", nueva_pagina->nro_marco,nueva_pagina->presencia,nueva_pagina->modificado,nueva_pagina->uso);
		
		nro_frames_asignados++;
	}

    t_heap_metadata* metadata = buscar_alloc_libre(carpincho->id);

    dir_logica = metadata->alloc_prev + sizeof(t_heap_metadata); 
	log_info(logger, "Direccion logica asignada: %d", dir_logica);
	return dir_logica;
}
*/

void *mem_read(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap))
		return false;
	
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;
	
	return obtener_bloque_paginacion(id_carpincho, dir_logica, tamanio_alocado);
}

bool mem_write(uint32_t id_carpincho, uint32_t dir_logica, void* contenido) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap))
		return false;
	
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;
	uint32_t tamanio_data = strlen(contenido);
	int32_t diferencia_tamanios = tamanio_alocado - strlen(contenido);

	if(diferencia_tamanios < 0)
		return false;

	void *data = malloc(tamanio_alocado);
	memcpy(data, contenido, tamanio_data);
	char relleno = '\0';
	
	if(diferencia_tamanios > 0) {
		while(diferencia_tamanios > 0) {
			memcpy(data + tamanio_alocado - diferencia_tamanios, &relleno, 1);
			diferencia_tamanios--;
		}
	}
	actualizar_bloque_paginacion(id_carpincho, dir_logica, data, tamanio_alocado);
	return true;
}