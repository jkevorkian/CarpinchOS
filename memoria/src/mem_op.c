#include "mem_op.h"

bool remover_pagina(uint32_t, t_entrada_tp *);
bool dir_logica_es_valida(uint32_t id_carpincho, uint32_t dir_logica);

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(!dir_logica_es_valida(id_carpincho, dir_logica) || get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El puntero es inválido");
		return false;
	}

	// Coloco el bit esFree del heap en 1 (ahora está libre)
	set_isFree(id_carpincho, dir_logica_heap);

	// UNIFICO EL NUEVO FREE CON LOS ADYACENTES
	uint32_t prev_heap;
	uint32_t main_heap = dir_logica - TAMANIO_HEAP;
	uint32_t foot_heap;
	
	uint32_t first_heap, last_heap;

	// BUSCO EL ALLOC ANTERIOR y, si corresponde, los integro
	prev_heap = get_prevAlloc(id_carpincho, main_heap);
	if(HEAP_NULL == prev_heap || !get_isFree(id_carpincho, prev_heap)) {
		first_heap = main_heap;
	}
	else {	// Se elimina el main_heap
		first_heap = prev_heap;
	}

	// BUSCO EL ALLOC SIGUIENTE y, si corresponde, los integro
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
			liberar_paginas_carpincho(id_carpincho, bytes_ocupados);

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

	uint32_t offset_final;
	uint32_t nro_frames_necesarios = 0;
	uint32_t dir_logica = 0;

	t_carpincho* carpincho = carpincho_de_lista(id_carpincho);
	bool crear_footer = true;

	if(config_memoria.tipo_asignacion == FIJA_LOCAL && list_size(carpincho->tabla_paginas) == 0) {
		if(!asignacion_fija(carpincho)) return 0;
	}
	/*
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL) {
		// lo hago solo la primera vez, despues asigno solo cuando me quedo sin paginas
		if(list_size(carpincho->tabla_paginas) == 0) {
			if(crear_movimiento_swap(NEW_PAGE, id_carpincho, nro_frames_necesarios, NULL)){
				for(int i = 0; i < nro_frames_necesarios; i++){
					agregar_pagina(carpincho->id);
				}
			}
			else return 0;
		}
	}*/

	if(carpincho->offset == 0) {
		offset_final = tamanio + 2 * TAMANIO_HEAP;
		nro_frames_necesarios = cant_marcos_faltantes(id_carpincho, offset_final);
		
		/////////////////////////////// Temporal
		t_marco *marco_auxiliar = obtener_marco(id_carpincho, 0);
		memset(inicio_memoria(marco_auxiliar->nro_real, 0), 0, config_memoria.tamanio_pagina);
		soltar_marco(marco_auxiliar);
		set_prevAlloc(id_carpincho, 0, HEAP_NULL);
		///////////////////////////////
		
		dir_logica = heap_header(carpincho, tamanio, 0, &crear_footer);
		heap_footer(carpincho, tamanio, dir_logica + tamanio, HEAP_NULL);
	}
	else {
		uint32_t desplazamiento = 0;
		uint32_t alloc_sig;
		uint8_t is_free;		

		while(true){
			alloc_sig = get_nextAlloc(carpincho->id, desplazamiento);
			is_free = get_isFree(carpincho->id, desplazamiento);

			if(alloc_sig == HEAP_NULL){
				/* uint32_t ult_dir_logica = list_size(carpincho->tabla_paginas) * config_memoria.tamanio_pagina;
				if (desplazamiento + 2*TAMANIO_HEAP + tamanio > ult_dir_logica) {
					if(crear_movimiento_swap(NEW_PAGE, id_carpincho, nro_frames_necesarios, NULL)){
						uint32_t espacio_que_sobra = ult_dir_logica - (desplazamiento + TAMANIO_HEAP);
						nro_frames_necesarios = cant_marcos_necesarios(tamanio + 2*TAMANIO_HEAP - espacio_que_sobra);
						
						for(int i = 0; i < nro_frames_necesarios; i++){
							agregar_pagina(carpincho->id);
						}
					}
					else return 0;
				}*/
				offset_final = desplazamiento + 2*TAMANIO_HEAP + tamanio;
				nro_frames_necesarios = cant_marcos_faltantes(id_carpincho, offset_final);

				if(nro_frames_necesarios) {
					if(crear_movimiento_swap(NEW_PAGE, id_carpincho, nro_frames_necesarios, NULL)) {
						for(int i = 0; i < nro_frames_necesarios; i++) {
							agregar_pagina(carpincho->id);
						}
					}
					else return 0;
				}
				carpincho->offset = offset_final;
				dir_logica = heap_header(carpincho, tamanio, desplazamiento, &crear_footer);
				heap_footer(carpincho, tamanio, dir_logica + tamanio, alloc_sig);
				break;
			}
			else {
				if(is_free) {
					dir_logica = heap_header(carpincho, tamanio, desplazamiento, &crear_footer);

					// no entra en ese espacio alocado, buscar otro
					if(dir_logica == 0) {
						desplazamiento = alloc_sig;
						continue;
					};

					// no necesito el footer porque entra justo
					if(!crear_footer) break;
					heap_footer(carpincho, tamanio, dir_logica + tamanio, alloc_sig);
					break;
				};

			desplazamiento = alloc_sig;

			}
		}
	}	

	log_info(logger, "Direccion logica asignada al carpincho #%d: %d", id_carpincho, dir_logica);

	////////////////////////////////////////////////////////////
	// Corregir, sirve solo para nuevos allocs al final
	// carpincho->offset = dir_logica + tamanio + TAMANIO_HEAP;
	////////////////////////////////////////////////////////////
	return dir_logica; 
}

uint32_t heap_header(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento, bool* crear_footer){
	bool tiene_footer = get_nextAlloc(carpincho->id, desplazamiento) != 0 || get_nextAlloc(carpincho->id, desplazamiento) != HEAP_NULL;
	uint32_t bytes_allocados = get_nextAlloc(carpincho->id, desplazamiento) - desplazamiento - TAMANIO_HEAP;

	// tiene que entrar justo en el tamaño de aloc libre o tiene que sobrar espacio para un nuevo heap footer
	if(tiene_footer){
		if(bytes_allocados == tam){
			//printf("Entra entero pero sin footer intermedio");
			*crear_footer = false;
			return desplazamiento + TAMANIO_HEAP;
		}
		else if(bytes_allocados >= tam + TAMANIO_HEAP){
			//printf("Entra con footer intermedio");
		}
		else {
			//printf("No entra, buscar otro libre.");
			return 0;
		}
	}

	reset_isFree(carpincho->id, desplazamiento);
	set_nextAlloc(carpincho->id, desplazamiento, TAMANIO_HEAP + tam + desplazamiento);

	log_info(logger, "Heap header creado en direccion logica: %d.", desplazamiento);
	log_info(logger, "%d %d %d", get_prevAlloc(carpincho->id, desplazamiento), get_nextAlloc(carpincho->id, desplazamiento), get_isFree(carpincho->id, desplazamiento));

	return desplazamiento + TAMANIO_HEAP;
}

uint32_t heap_footer(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento, uint32_t alloc_sig){
	set_isFree(carpincho->id, desplazamiento);
	set_nextAlloc(carpincho->id, desplazamiento, alloc_sig);
	set_prevAlloc(carpincho->id, desplazamiento, desplazamiento - tam - TAMANIO_HEAP);

	log_info(logger, "Heap footer creado en direccion logica: %d.", desplazamiento);
	log_info(logger, "%d %d %d", get_prevAlloc(carpincho->id, desplazamiento), get_nextAlloc(carpincho->id, desplazamiento), get_isFree(carpincho->id, desplazamiento));

	return desplazamiento + TAMANIO_HEAP;
}

void *mem_read(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(!dir_logica_es_valida(id_carpincho, dir_logica) || get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El puntero es inválido");
		return false;
	}
	
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;
	
	return obtener_bloque_paginacion(id_carpincho, dir_logica, tamanio_alocado);
}

bool mem_write(uint32_t id_carpincho, uint32_t dir_logica, void* contenido) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(!dir_logica_es_valida(id_carpincho, dir_logica) || get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El puntero es inválido");
		return false;
	}
	
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

void liberar_paginas_carpincho(uint32_t id_carpincho, uint32_t desplazamiento) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	carpincho->offset = desplazamiento;

	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return;	

	t_list *tabla_de_paginas = carpincho->tabla_paginas;
	t_entrada_tp *entrada;
	t_marco *marco;

	div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);
	
	uint32_t paginas_minimas = posicion_compuesta.quot;
	if(posicion_compuesta.rem)
		paginas_minimas++;
	
	pthread_mutex_lock(&carpincho->mutex_tabla);
	while(config_memoria.tipo_asignacion == FIJA_LOCAL && paginas_minimas < list_size(tabla_de_paginas)) {
		entrada = list_remove(tabla_de_paginas, list_size(tabla_de_paginas) - 1);
		pthread_mutex_unlock(&carpincho->mutex_tabla);
		// liberar marco, avisar a swap y quitar ultimo elemento de la lista de paginas del carpincho
		if(entrada->presencia) {
			marco = memoria_ram.mapa_fisico[entrada->nro_marco];
			pthread_mutex_lock(&mutex_asignacion_marcos);
			marco->libre = true;
			marco->duenio = 0;
			memset(inicio_memoria(marco->nro_real, 0), 0, config_memoria.tamanio_pagina);
			pthread_mutex_unlock(&mutex_asignacion_marcos);
		}
		free(entrada);
		crear_movimiento_swap(RM_PAGE, id_carpincho, 0, NULL);

		pthread_mutex_lock(&carpincho->mutex_tabla);
	}

	uint32_t nro_paginas_vacias = list_size(tabla_de_paginas) - paginas_minimas;
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	// Esto se hace para evitar swap out innecesarios	
	for(int i = paginas_minimas; i < nro_paginas_vacias; i++) {
		entrada = pagina_de_carpincho(id_carpincho, i);
		pthread_mutex_lock(&entrada->mutex);
		entrada->esta_vacia = true;
		pthread_mutex_unlock(&entrada->mutex);
	}
}

bool dir_logica_es_valida(uint32_t id_carpincho, uint32_t dir_logica) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	return carpincho->offset > dir_logica;
}