#include "mem_op.h"

bool remover_pagina(uint32_t, t_entrada_tp *);

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap)) {
		log_warning(logger, "El alloc estaba libre");
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

	uint32_t nro_frames_necesarios = cant_marcos_necesarios(tamanio + 2*TAMANIO_HEAP);
	uint32_t dir_logica = 0;
	t_carpincho* carpincho = carpincho_de_lista(id_carpincho);
	bool crear_footer = true;

	if(config_memoria.tipo_asignacion == FIJA_LOCAL) {
		if(list_size(carpincho->tabla_paginas) == 0) {
			if(!asignacion_fija(carpincho)) return 0;
		}
		// ya se que tiene todos los marcos asignados que va a usar, por lo que solo debo fijarme que nro_frames_necesarios no exceda la cantidad maxima de marcos en swap?
		// deberia fijarme que mi cantidad de marcos libres sea mayor a la cant de marcos necesarios?
	}
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL) {
		// lo hago solo la primera vez, despues asigno solo cuando me quedo sin paginas
		if(list_size(carpincho->tabla_paginas) == 0) {
			if(crear_movimiento_swap(NEW_PAGE, id_carpincho, nro_frames_necesarios, NULL)){
				for(int i = 0; i < nro_frames_necesarios; i++){
					t_marco* marco_a_reemplazar = realizar_algoritmo_reemplazo(id_carpincho, i);
					crear_nueva_pagina(marco_a_reemplazar->nro_real, carpincho);
				}
			}
			else return 0;
		}
	}

	if(carpincho->heap_metadata == NULL) {
		carpincho->heap_metadata = dir_fisica_proceso(carpincho->tabla_paginas);
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
				if(is_free) {
					uint32_t ult_dir_logica = list_size(carpincho->tabla_paginas) * config_memoria.tamanio_pagina;
					if (desplazamiento + 2*TAMANIO_HEAP + tamanio > ult_dir_logica) {
						if(crear_movimiento_swap(NEW_PAGE, id_carpincho, nro_frames_necesarios, NULL)){
							// en algun caso puede sobrar espacio negativo? no creo
							uint32_t espacio_que_sobra = ult_dir_logica - (desplazamiento + TAMANIO_HEAP);
							nro_frames_necesarios = cant_marcos_necesarios(tamanio + 2*TAMANIO_HEAP - espacio_que_sobra);
							
							for(int i = 0; i < nro_frames_necesarios; i++){
								t_marco* marco_a_reemplazar = realizar_algoritmo_reemplazo(id_carpincho, i);
								crear_nueva_pagina(marco_a_reemplazar->nro_real, carpincho);
							}
						}
						else return 0;
					}
					dir_logica = heap_header(carpincho, tamanio, desplazamiento, &crear_footer);
					heap_footer(carpincho, tamanio, dir_logica + tamanio, alloc_sig);
					break;
				};
				// siempre voy a tener un metadata free al final de todo o no?
			}
			else {
				if(is_free) {
					dir_logica = heap_header(carpincho, tamanio, desplazamiento, &crear_footer);

					// no entra en ese espacio alocado, buscar otro
					// o tambien: no necesito el footer porque entra justo
					if(dir_logica == 0) {
						desplazamiento = alloc_sig;
						continue;
					};

					if(!crear_footer) break;
					heap_footer(carpincho, tamanio, dir_logica + tamanio, alloc_sig);
					break;
				};

			desplazamiento = alloc_sig;

			}
		}
	}	

	log_info(logger, "Direccion logica asignada al carpincho #%d: %d", id_carpincho, dir_logica);
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

void liberar_paginas_carpincho(uint32_t id_carpincho, uint32_t desplazamiento) {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return;
	
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	

	t_list *tabla_de_paginas = ((t_carpincho *)carpincho_de_lista(id_carpincho))->tabla_paginas;
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
			pthread_mutex_lock(&mutex_asignacion_marcos);
			marco = memoria_ram.mapa_fisico[entrada->nro_marco];
			marco->libre = true;
			marco->duenio = 0;
			pthread_mutex_unlock(&mutex_asignacion_marcos);
		}
		free(entrada);
		crear_movimiento_swap(RM_PAGE, id_carpincho, 0, NULL);

		pthread_mutex_lock(&carpincho->mutex_tabla);
	}
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	// Esto se hace para evitar swap out innecesarios
	pthread_mutex_lock(&carpincho->mutex_tabla);
	uint32_t nro_paginas_vacias = list_size(tabla_de_paginas) - paginas_minimas;
	for(int i = paginas_minimas; i < nro_paginas_vacias; i++) {
		entrada = pagina_de_carpincho(id_carpincho, i);
		pthread_mutex_lock(&entrada->mutex);
		entrada->esta_vacia = true;
	}
}