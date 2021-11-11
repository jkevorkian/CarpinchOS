#include "memoria.h"

bool iniciar_memoria(t_config* config){
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");

	if(config_memoria.tamanio_memoria % config_memoria.tamanio_pagina > 0) {
        log_error(logger, "Hubo un error al crear la memoria.");
        return false;
    }

    memoria_ram.inicio = (void*) malloc(config_memoria.tamanio_memoria);   
    if (memoria_ram.inicio == NULL) {
        log_error(logger, "Hubo un error al crear la memoria.");
        return false;
    }

	memset(memoria_ram.inicio, 0, config_memoria.tamanio_memoria);

	config_memoria.cant_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	memoria_ram.mapa_fisico = calloc(config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro marcos: %d. Tamaño marco: %d bytes", config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	char * algoritmo_reemplazo_mmu = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
	if(!strcmp(algoritmo_reemplazo_mmu, "LRU"))
		config_memoria.algoritmo_reemplazo = LRU;
	if(!strcmp(algoritmo_reemplazo_mmu, "CLOCK-M")) {
		config_memoria.algoritmo_reemplazo = CLOCK;
		memoria_ram.puntero_clock = 0;
	}

	char * tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	if(!strcmp(tipo_asignacion, "FIJA"))
		config_memoria.tipo_asignacion = FIJA_LOCAL;
	if(!strcmp(tipo_asignacion, "DINAMICA")) {
		config_memoria.tipo_asignacion = DINAMICA_GLOBAL;
	}

	iniciar_marcos(config_memoria.cant_marcos);
	//iniciar_tlb(config);  

	return true;
}

void iniciar_marcos(uint32_t cant_marcos){
	for(int i = 0; i < cant_marcos; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_fisico[i] = marco_auxiliar;

		marco_auxiliar->libre = true;
		marco_auxiliar->nro_real = i;
	}
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco, t_carpincho* carpincho){
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
	list_add(carpincho->tabla_paginas, pagina);

	pagina->nro_marco = nro_marco;
	pagina->presencia = true;
	pagina->modificado = false;
	pagina->uso = true;

	log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
	log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);

	return pagina;
}

uint32_t cant_frames_necesarios(uint32_t tamanio) {
	div_t nro_frames = div(tamanio, config_memoria.tamanio_pagina);
	uint32_t nro_frames_q = nro_frames.quot;
	if(nro_frames.rem > 0) nro_frames_q++;
	return nro_frames_q;
}

uint32_t pagina_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.quot;
}

uint32_t offset_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.rem;
}

t_carpincho *carpincho_de_lista(uint32_t id_carpincho) {
	bool mi_carpincho(void *un_carpincho) {
		if(((t_carpincho *)un_carpincho)->id == id_carpincho)
			return true;
		else
			return false;
	}

	return list_find(lista_carpinchos, mi_carpincho);
}

void *inicio_memoria(uint32_t nro_marco, uint32_t offset) {
	return memoria_ram.inicio + nro_marco * config_memoria.tamanio_pagina + offset;
}

/*
void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado) {
	if(modificado)
		marco_auxiliar->bit_modificado = true;

	if(config_memoria.algoritmo_reemplazo == LRU)
		marco_auxiliar->temporal = temporal_get_string_time("%H:%M:%S");
	else
		marco_auxiliar->bit_uso = true;
}

t_marco* obtener_marco_libre() {
    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			marco->libre = false; //mutex
			return marco; 
		}
    }


    // t_marco* marco = pedir_frame_swap();


	return NULL;
}

uint32_t cant_marcos_necesarios(uint32_t tamanio) {
	div_t nro_marcos = div(tamanio, config_memoria.tamanio_pagina); 
	uint32_t nro_marcos_q = nro_marcos.quot;

	if(nro_marcos.rem > 0) nro_marcos_q++;
	return nro_marcos_q;
}

bool tengo_marcos_suficientes(uint32_t necesarios){
	// no entiendo si yo aca devuelvo que tengo marcos necesarios pero despues mientras le asigno me quedo sin marcos porque los uso otro proceso que pasa?
    uint32_t contador_necesarios = necesarios;

    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			contador_necesarios--;
		}
        if(contador_necesarios == 0) return true;
    }

    //seguir buscando en swap

    return false;
}


bool get_isFree(uint32_t id, uint32_t inicio_heap) {
	uint8_t bit_esFree;
	div_t posicion_compuesta = div(inicio_heap + 8, config_memoria.tamanio_pagina);

	t_marco *marco = obtener_marco(id, posicion_compuesta.quot);
	memcpy(&bit_esFree, inicio_memoria(marco->nro_real, posicion_compuesta.quot), 1);
	soltar_marco(marco);

	if(bit_esFree)	return true;
	else			return false;
}

void set_isFree(uint32_t id, uint32_t inicio_heap) {
	uint8_t bit_esFree = 1;
	div_t posicion_compuesta = div(inicio_heap + 8, config_memoria.tamanio_pagina);

	t_marco *marco = obtener_marco(id, posicion_compuesta.quot);
	memcpy(inicio_memoria(marco->nro_real, posicion_compuesta.quot), &bit_esFree, 1);
	soltar_marco(marco);
}

void reset_isFree(uint32_t id, uint32_t inicio_heap) {
	uint8_t bit_esFree = 0;
	div_t posicion_compuesta = div(inicio_heap + 8, config_memoria.tamanio_pagina);

	t_marco *marco = obtener_marco(id, posicion_compuesta.quot);
	memcpy(inicio_memoria(marco->nro_real, posicion_compuesta.quot), &bit_esFree, 1);
	soltar_marco(marco);
}

uint32_t get_prevAlloc(uint32_t id, uint32_t inicio_heap) {
	uint32_t pos_alloc;
	void * bloque = obtener_bloque_paginacion(id, inicio_heap, 4);
	memcpy(&pos_alloc, bloque, 4);
	return pos_alloc;
}

uint32_t get_nextAlloc(uint32_t id, uint32_t inicio_heap) {
	uint32_t pos_alloc;
	void * bloque = obtener_bloque_paginacion(id, inicio_heap + 4, 4);
	memcpy(&pos_alloc, bloque, 4);
	return pos_alloc;
}

void set_prevAlloc(uint32_t id, uint32_t inicio_heap, uint32_t nuevo_valor) {
	uint32_t data = nuevo_valor;
	actualizar_bloque_paginacion(id, inicio_heap, &data, 4);
}

void set_nextAlloc(uint32_t id, uint32_t inicio_heap, uint32_t nuevo_valor) {
	uint32_t data = nuevo_valor;
	actualizar_bloque_paginacion(id, inicio_heap + 4, &data, 4);
}

void soltar_marco(t_marco *marco_auxiliar) {
	pthread_mutex_unlock(&marco_auxiliar->mutex);
}

void reservar_marco(t_marco *marco_auxiliar) {
	pthread_mutex_lock(&marco_auxiliar->mutex);
}

void* obtener_bloque_paginacion(uint32_t id, uint32_t desplazamiento, uint32_t tamanio) {
	// t_carpincho* mi_carpincho = (t_carpincho *)list_get(lista_carpinchos, id - 1);

	div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);

	uint32_t bytes_cargados = 0;
	uint32_t bytes_disponibles = config_memoria.tamanio_pagina - posicion_compuesta.rem;
	uint32_t pagina_actual = posicion_compuesta.quot;
	uint32_t bytes_necesarios = tamanio;

	void* data = malloc(tamanio);

	uint32_t inicio_pagina = posicion_compuesta.rem;
	while(bytes_cargados < bytes_necesarios) {
		if(bytes_disponibles > bytes_necesarios - bytes_cargados)
			bytes_disponibles = bytes_necesarios - bytes_cargados;

		t_marco* marco_auxiliar = obtener_marco(id, pagina_actual);
		memcpy(data, inicio_memoria(marco_auxiliar->nro_real, inicio_pagina), bytes_disponibles);
		actualizar_info_algoritmo(marco_auxiliar, true);
		soltar_marco(marco_auxiliar);

		// log_warning(logger, "Incorpore marco. Obtengo dato paginacion");
		// log_info(logger, "Data: %d", obtener_entero_paginacion(id_patota, desplazamiento));

		bytes_cargados += bytes_disponibles;
		bytes_disponibles = config_memoria.tamanio_pagina;
		inicio_pagina = 0;
		pagina_actual++;
	}
    return data;
}

void actualizar_bloque_paginacion(uint32_t id, uint32_t desplazamiento, void* data, uint32_t tamanio) {
	// t_carpincho* mi_carpincho = (t_carpincho *)list_get(lista_carpinchos, id - 1);

    div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);

	uint32_t bytes_cargados = 0;
	uint32_t bytes_disponibles = config_memoria.tamanio_pagina - posicion_compuesta.rem;
	uint32_t pagina_actual = posicion_compuesta.quot;
	uint32_t bytes_necesarios = tamanio;

	uint32_t inicio_pagina = posicion_compuesta.rem;
	while(bytes_cargados < bytes_necesarios) {
		if(bytes_disponibles > bytes_necesarios - bytes_cargados)
			bytes_disponibles = bytes_necesarios - bytes_cargados;

		t_marco* marco_auxiliar = obtener_marco(id, pagina_actual);
		memcpy(inicio_memoria(marco_auxiliar->nro_real, inicio_pagina), &data, bytes_disponibles);
		actualizar_info_algoritmo(marco_auxiliar);
		soltar_marco(marco_auxiliar);

		// log_warning(logger, "Incorpore marco. Obtengo dato paginacion");
		// log_info(logger, "Data: %d", obtener_entero_paginacion(id_patota, desplazamiento));

		bytes_cargados += bytes_disponibles;
		bytes_disponibles = config_memoria.tamanio_pagina;
		inicio_pagina = 0;
		pagina_actual++;
	}
}

void reasignar_marco(uint32_t id_carpincho, uint32_t nro_pagina, t_marco* marco) {
	// actualizar_tp();
	// t_carpincho * nuevo_carpincho = list_get(lista_carpinchos, id_carpincho - 1);
	// t_carpincho * viejo_carpincho = list_get(lista_carpinchos, marco->duenio - 1);

	pthread_mutex_lock(&marco->mutex);
	if(id_carpincho) {
		// enviar pagina a swamp
		// swap_out(viejo_carpincho->id, nro_pagina);
		// viejo_carpincho->tabla_paginas[marco->pagina_duenio] = NULL; // ??
	}

	// nuevo_carpincho->tabla_paginas[nro_pagina] debería ser NULL al no estar asignado
	// nuevo_carpincho->tabla_paginas[nro_pagina] = marco;
	marco->duenio = id_carpincho;
	marco->pagina_duenio = nro_pagina;
	marco->bit_modificado = false;
	marco->bit_uso = true;

	// pedir pagina a swap
	// void * buffer = swap_in(id_carpincho, nro_pagina);
	//memcpy(inicio_memoria(marco->nro_real, 0), buffer, config_memoria.tamanio_pagina);
	pthread_mutex_lock(&marco->mutex);

	//actualizar_tlb();
}


t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	// el marco hay que reservarlo para que no lo saquen a memoria secundaria
	// durante el proceso
	// lo mismo con los marcos que se pidan para inificar el heapMetaData

	// uint32_t nro_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	t_marco* marco;
	t_carpincho* mi_carpincho = carpincho_de_lista(id_carpincho);
	// t_entrada_tp *entrada_tp = mi_carpincho->tabla_paginas[nro_pagina];
	//if(entrada_tp->id_carpincho == id_carpincho && entrada_tp->nro_pagina == nro_pagina) {
	//	marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
	//	reservar_marco(marco);
	//}

	//else {
		// Page fault
		marco = realizar_algoritmo_reemplazo(id_carpincho);
		reasignar_marco(id_carpincho, nro_pagina, marco);
	// }
	return marco;
}

uint32_t mem_alloc(t_carpincho* carpincho, uint32_t tamanio) {
	return 0;
}

t_heap_metadata* buscar_alloc_libre(uint32_t carpincho_id){
	if(tam < heap_actual->alloc_sig - sizeof(t_heap_metadata)) {

		t_heap_metadata* nuevo_heap = malloc(sizeof(t_heap_metadata));
		heap_actual->alloc_sig = sizeof(t_heap_metadata) + tam;
		nuevo_heap->alloc_prev = heap_actual->alloc_prev;
		nuevo_heap->alloc_sig = 0;
		nuevo_heap->libre = true;

		log_info(logger, "Heap metadata: |%d|%d|%d|%d|", heap_actual->alloc_prev, heap_actual->alloc_sig, nuevo_heap->alloc_prev, nuevo_heap->alloc_sig);
	}
	return heap_actual;
	return NULL;
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
*/

