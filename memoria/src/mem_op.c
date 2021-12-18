#include "mem_op.h"

bool dir_logica_es_valida(uint32_t id_carpincho, uint32_t dir_logica);

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es valido
	if(!dir_logica_es_valida(id_carpincho, dir_logica) || get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El puntero es invalido");
		return false;
	}

	// Coloco el bit esFree del heap en 1 (ahora esta libre)
	log_info(logger, "Seteo heap como libre");
	set_isFree(id_carpincho, dir_logica_heap);

	// UNIFICO EL NUEVO FREE CON LOS ADYACENTES
	uint32_t prev_heap;
	uint32_t main_heap = dir_logica - TAMANIO_HEAP;
	uint32_t foot_heap;
	
	uint32_t first_heap, last_heap;

	// BUSCO EL ALLOC ANTERIOR y, si corresponde, los integro
	log_info(logger, "Ananlizo alloc anterior");
	prev_heap = get_prevAlloc(id_carpincho, main_heap);
	if(HEAP_NULL == prev_heap || !get_isFree(id_carpincho, prev_heap)) {
		first_heap = main_heap;
	}
	else {	// Se elimina el main_heap
		first_heap = prev_heap;
	}

	// BUSCO EL ALLOC SIGUIENTE y, si corresponde, los integro
	log_info(logger, "Ananlizo alloc siguiente");
	foot_heap = get_nextAlloc(id_carpincho, main_heap);
	if(!get_isFree(id_carpincho, foot_heap)) {
		last_heap = foot_heap;
	}
	else {	// Se elimina el foot_heap
		last_heap = get_nextAlloc(id_carpincho, foot_heap);
	}
	
	if(main_heap != first_heap || foot_heap != last_heap) {
		uint32_t bytes_ocupados = 1;
		if(last_heap == HEAP_NULL) {
			bytes_ocupados = first_heap > 0 ? first_heap + TAMANIO_HEAP : 0;
			log_info(logger, "Libero paginas de carpicho");
			liberar_paginas_carpincho(id_carpincho, bytes_ocupados);
			log_info(logger, "Nuevo offset: %d", bytes_ocupados);
		}
		else {
			set_prevAlloc(id_carpincho, last_heap, first_heap);
		}
		if(bytes_ocupados != 0) {
			set_nextAlloc(id_carpincho, first_heap, last_heap);
		}
	}

	return true;
}

uint32_t mem_alloc(uint32_t id_carpincho, uint32_t tamanio) {
	log_info(logger, "El proceso #%d solicito %d bytes de memoria.", id_carpincho, tamanio);

	bool encontre_alloc = false;
	
	uint32_t main_heap = 0, foot_heap = 0;
	uint32_t next_heap = HEAP_NULL, alloc_sig = HEAP_NULL;

	t_carpincho* carpincho = carpincho_de_lista(id_carpincho);

	if(config_memoria.tipo_asignacion == FIJA_LOCAL && list_size(carpincho->tabla_paginas) == 0) {
		log_info(logger, "Inicio paginas carpincho en asignacion fija");
		if(!asignacion_fija(carpincho)) return 0;
	}

	if(carpincho->offset == 0) {
		foot_heap = main_heap + TAMANIO_HEAP + tamanio;
		encontre_alloc = true;
	}	

	while(!encontre_alloc) {
		alloc_sig = get_nextAlloc(id_carpincho, main_heap);
		log_info(logger, "Obtuve %d alloc_sig", alloc_sig);
		bool is_free = get_isFree(id_carpincho, main_heap);
		log_info(logger, "Obtuve %d is_free", is_free);
		
		if(!is_free) {
			main_heap = alloc_sig;
			continue;
		}

		if(alloc_sig == HEAP_NULL) {
			encontre_alloc = true;
			foot_heap = main_heap + TAMANIO_HEAP + tamanio;
			continue;
		}

		uint32_t bytes_disponibles_alloc = alloc_sig - (main_heap + TAMANIO_HEAP);
		if(bytes_disponibles_alloc == tamanio) {				// Entra sin footer intermedio
			encontre_alloc = true;
			continue;
		}
		if(bytes_disponibles_alloc >= tamanio + TAMANIO_HEAP) {	// Entra con footer intermedio
			encontre_alloc = true;
			foot_heap = main_heap + tamanio + TAMANIO_HEAP;
			next_heap = alloc_sig;
			continue;
		}
		main_heap = alloc_sig;	// No entra
	}

	if(alloc_sig == HEAP_NULL) {
		uint32_t offset_final = main_heap + tamanio + 2*TAMANIO_HEAP;
		uint32_t nro_frames_necesarios = cant_marcos_faltantes(id_carpincho, offset_final);
		if(nro_frames_necesarios) {
			log_info(logger, "Necesito %d pagina/s mas", nro_frames_necesarios);
			if(crear_movimiento_swap(NEW_PAGE, id_carpincho, nro_frames_necesarios, NULL)) {
				for(int i = 0; i < nro_frames_necesarios; i++) {
					agregar_pagina(id_carpincho);
				}
			}
			else return 0;
		}
		carpincho->offset = offset_final;	// La variable solo la usa el carpincho, no hace falta mutex
	}
	
	log_info(logger, "Seteo el bit de libre de main_heap en 0");
	reset_isFree(id_carpincho, main_heap);
	if(!main_heap)	set_prevAlloc(id_carpincho, main_heap, HEAP_NULL);	

	if(foot_heap) {
		log_info(logger, "Seteo foot heap en posicion %d", foot_heap);
		set_nextAlloc(id_carpincho, main_heap, foot_heap);
		set_prevAlloc(id_carpincho, foot_heap, main_heap);
		set_nextAlloc(id_carpincho, foot_heap, next_heap);
		set_isFree(id_carpincho, foot_heap);

		if(next_heap != HEAP_NULL)	set_prevAlloc(id_carpincho, next_heap, foot_heap);
	}
	
	log_info(logger, "Direccion logica asignada al carpincho #%d: %d", id_carpincho, main_heap + TAMANIO_HEAP);
	return main_heap + TAMANIO_HEAP; 
}

void *mem_read(uint32_t id_carpincho, uint32_t dir_logica, uint32_t tamanio) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es valido
	if(!dir_logica_es_valida(id_carpincho, dir_logica) || get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El puntero es invalido");
		return false;
	}
	log_info(logger, "Obtengo next alloc");
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;

	if(tamanio_alocado < tamanio) {
		log_warning(logger, "El tamanio pedido es mayor que el tamanio asignado. %d >> %d", tamanio, tamanio_alocado);
		return false;
	}
	log_info(logger, "Obtengo bloque paginacion");
	return obtener_bloque_paginacion(id_carpincho, dir_logica, tamanio);
}

bool mem_write(uint32_t id_carpincho, uint32_t dir_logica, void* contenido, uint32_t tamanio) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es valido
	if(!dir_logica_es_valida(id_carpincho, dir_logica) || get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El puntero es invalido");
		return false;
	}
	
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;

	if(tamanio_alocado < tamanio) {
		log_warning(logger, "El tamanio pedido es mayor que el tamanio asignado. %d >> %d", tamanio, tamanio_alocado);
		return false;
	}

	actualizar_bloque_paginacion(id_carpincho, dir_logica, contenido, tamanio);
	free(contenido);
	return true;
}

bool dir_logica_es_valida(uint32_t id_carpincho, uint32_t dir_logica) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	log_info(logger, "Offset: %d", carpincho->offset);
	return carpincho->offset > dir_logica;
}
