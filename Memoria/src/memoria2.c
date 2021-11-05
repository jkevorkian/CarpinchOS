#include "memoria2.h"

bool get_isFree(uint32_t nro_marco, uint32_t desplazamiento);
void set_isFree(uint32_t nro_marco, uint32_t desplazamiento);
void reset_isFree(uint32_t nro_marco, uint32_t desplazamiento);

uint32_t get_prevAlloc(uint32_t nro_marco, uint32_t desplazamiento);
uint32_t get_nextAlloc(uint32_t nro_marco, uint32_t desplazamiento);

void set_prevAlloc(uint32_t nro_marco, uint32_t desplazamiento, uint32_t nuevo_valor);
void set_nextAlloc(uint32_t nro_marco, uint32_t desplazamiento, uint32_t nuevo_valor);

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

uint32_t pagina_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.quot;
}

uint32_t offset_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.rem;
}

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	// el marco hay que reservarlo para que no lo saquen a memoria secundaria
	// durante el proceso
	// lo mismo con los marcos que se pidan para inificar el heapMetaData

	// uint32_t nro_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	t_marco* marco;
	t_carpincho* mi_carpincho = carpincho_de_lista(id_carpincho);
	t_entrada_tp *entrada_tp = mi_carpincho->tabla_paginas[nro_pagina];
	if(entrada_tp->id_carpincho == id_carpincho && entrada_tp->nro_pagina == nro_pagina) {
		marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
		reservar_marco(marco);
	}

	else {
		// Page fault
		marco = realizar_algoritmo_reemplazo(id_carpincho);
		reasignar_marco(id_carpincho, nro_pagina, marco);
	}
	return marco;
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
	memcpy(inicio_memoria(marco->nro_real, &bit_esFree, posicion_compuesta.quot), 1);
	soltar_marco(marco);
}

void reset_isFree(uint32_t id, uint32_t inicio_heap) {
	uint8_t bit_esFree = 0;
	div_t posicion_compuesta = div(inicio_heap + 8, config_memoria.tamanio_pagina);

	t_marco *marco = obtener_marco(id, posicion_compuesta.quot);
	memcpy(inicio_memoria(marco->nro_real, &bit_esFree, posicion_compuesta.quot), 1);
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
	t_carpincho* mi_carpincho = (t_carpincho *)list_get(lista_carpinchos, id - 1);

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
	t_carpincho* mi_carpincho = (t_carpincho *)list_get(lista_carpinchos, id - 1);

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
	t_carpincho * nuevo_carpincho = list_get(lista_carpinchos, id_carpincho - 1);
	t_carpincho * viejo_carpincho = list_get(lista_carpinchos, marco->duenio - 1);

	pthread_mutex_lock(&marco->mutex);
	if(id_carpincho) {
		// enviar pagina a swamp
		swap_out(viejo_carpincho->id, nro_pagina);
		viejo_carpincho->tabla_paginas[marco->pagina_duenio] = NULL; // ??
	}

	// nuevo_carpincho->tabla_paginas[nro_pagina] debería ser NULL al no estar asignado
	nuevo_carpincho->tabla_paginas[nro_pagina] = marco;
	marco->duenio = id_carpincho;
	marco->pagina_duenio = nro_pagina;
	marco->bit_modificado = false;
	marco->bit_uso = true;

	// pedir pagina a swap
	void * buffer = swap_in(id_carpincho, nro_pagina);
	memcpy(inicio_memoria(marco->nro_real, 0), buffer, config_memoria.tamanio_pagina);
	pthread_mutex_lock(&marco->mutex);

	//actualizar_tlb();
}

void *inicio_memoria(uint32_t nro_marco, uint32_t offset) {
	return memoria_ram.inicio + nro_marco * config_memoria.tamanio_pagina + offset;
}

void swap_out(uint32_t id_carpincho, uint32_t nro_pagina) {

}
