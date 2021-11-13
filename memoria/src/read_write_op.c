#include "read_write_op.h"

void* obtener_bloque_paginacion(uint32_t id, uint32_t desplazamiento, uint32_t tamanio) {
	// t_carpincho* mi_carpincho = (t_carpincho *)list_get(lista_carpinchos, id - 1);

	div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);

	uint32_t bytes_cargados = 0;
	uint32_t bytes_disponibles = config_memoria.tamanio_pagina - posicion_compuesta.rem;
	uint32_t pagina_actual = posicion_compuesta.quot;
	uint32_t bytes_necesarios = tamanio;

	void* data = malloc(tamanio);

	/////////////////////// Para testear
	void* inicio;
	///////////////////////

	uint32_t inicio_pagina = posicion_compuesta.rem;
	while(bytes_cargados < bytes_necesarios) {
		if(bytes_disponibles > bytes_necesarios - bytes_cargados)
			bytes_disponibles = bytes_necesarios - bytes_cargados;

		t_marco* marco_auxiliar = obtener_marco(id, pagina_actual);
		inicio = inicio_memoria(marco_auxiliar->nro_real, inicio_pagina);

		memcpy(data + bytes_cargados, inicio, bytes_disponibles);

		actualizar_info_algoritmo(marco_auxiliar, false);
		soltar_marco(marco_auxiliar);

		// log_warning(logger, "Incorpore marco. Obtengo dato paginacion");
		// log_info(logger, "Data: %d", obtener_entero_paginacion(id_patota, desplazamiento));

		bytes_cargados += bytes_disponibles;
		bytes_disponibles = config_memoria.tamanio_pagina;
		inicio_pagina = 0;
		pagina_actual++;
	}
	log_info(logger, "Inscribo %s", data);
    return data;
}

void actualizar_bloque_paginacion(uint32_t id, uint32_t desplazamiento, void* data, uint32_t tamanio) {
	// t_carpincho* mi_carpincho = (t_carpincho *)list_get(lista_carpinchos, id - 1);

    div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);

	uint32_t bytes_cargados = 0;
	uint32_t bytes_disponibles = config_memoria.tamanio_pagina - posicion_compuesta.rem;
	uint32_t pagina_actual = posicion_compuesta.quot;
	uint32_t bytes_necesarios = tamanio;

	/////////////////////// Para testear
	void* inicio;
	///////////////////////

	uint32_t inicio_pagina = posicion_compuesta.rem;
	while(bytes_cargados < bytes_necesarios) {
		if(bytes_disponibles > bytes_necesarios - bytes_cargados)
			bytes_disponibles = bytes_necesarios - bytes_cargados;

		t_marco* marco_auxiliar = obtener_marco(id, pagina_actual);
		inicio = inicio_memoria(marco_auxiliar->nro_real, inicio_pagina);
		memcpy(inicio, data + bytes_cargados, bytes_disponibles);

		actualizar_info_algoritmo(marco_auxiliar, true);
		soltar_marco(marco_auxiliar);

		// log_warning(logger, "Incorpore marco. Obtengo dato paginacion");
		// log_info(logger, "Data: %d", obtener_entero_paginacion(id_patota, desplazamiento));

		bytes_cargados += bytes_disponibles;
		bytes_disponibles = config_memoria.tamanio_pagina;
		inicio_pagina = 0;
		pagina_actual++;
	}
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

