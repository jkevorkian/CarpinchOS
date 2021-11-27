#include "read_write_op.h"

void* obtener_bloque_paginacion(uint32_t id, uint32_t desplazamiento, uint32_t tamanio) {
	div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);

	uint32_t bytes_cargados = 0;
	uint32_t bytes_disponibles = config_memoria.tamanio_pagina - posicion_compuesta.rem;
	uint32_t pagina_actual = posicion_compuesta.quot;
	uint32_t bytes_necesarios = tamanio;

	void* data = malloc(tamanio);
	void* inicio;

	uint32_t inicio_pagina = posicion_compuesta.rem;
	while(bytes_cargados < bytes_necesarios) {
		if(bytes_disponibles > bytes_necesarios - bytes_cargados)
			bytes_disponibles = bytes_necesarios - bytes_cargados;

		t_marco* marco_auxiliar = obtener_marco(id, pagina_actual);
		inicio = inicio_memoria(marco_auxiliar->nro_real, inicio_pagina);

		memcpy(data + bytes_cargados, inicio, bytes_disponibles);

		log_info(logger, "Loggeo marco %d por lectura", marco_auxiliar->nro_real);
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco_auxiliar->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);

		actualizar_info_algoritmo(marco_auxiliar, false);
		soltar_marco(marco_auxiliar);

		bytes_cargados += bytes_disponibles;
		bytes_disponibles = config_memoria.tamanio_pagina;
		inicio_pagina = 0;
		pagina_actual++;
	}
    return data;
}

void actualizar_bloque_paginacion(uint32_t id, uint32_t desplazamiento, void* data, uint32_t tamanio) {
	div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);

	uint32_t bytes_cargados = 0;
	uint32_t bytes_disponibles = config_memoria.tamanio_pagina - posicion_compuesta.rem;
	uint32_t pagina_actual = posicion_compuesta.quot;
	uint32_t bytes_necesarios = tamanio;

	void* inicio;

	uint32_t inicio_pagina = posicion_compuesta.rem;
	while(bytes_cargados < bytes_necesarios) {
		if(bytes_disponibles > bytes_necesarios - bytes_cargados)
			bytes_disponibles = bytes_necesarios - bytes_cargados;

		t_marco* marco_auxiliar = obtener_marco(id, pagina_actual);
		inicio = inicio_memoria(marco_auxiliar->nro_real, inicio_pagina);
		memcpy(inicio, data + bytes_cargados, bytes_disponibles);

		log_info(logger, "Loggeo marco %d por escritura", marco_auxiliar->nro_real);
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco_auxiliar->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);

		actualizar_info_algoritmo(marco_auxiliar, true);
		soltar_marco(marco_auxiliar);

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
	memcpy(&bit_esFree, inicio_memoria(marco->nro_real, posicion_compuesta.rem), 1);
	soltar_marco(marco);

	if(bit_esFree)	return true;
	else			return false;
}

void set_isFree(uint32_t id, uint32_t inicio_heap) {
	uint8_t bit_esFree = 1;
	div_t posicion_compuesta = div(inicio_heap + 8, config_memoria.tamanio_pagina);

	t_marco *marco = obtener_marco(id, posicion_compuesta.quot);
	memcpy(inicio_memoria(marco->nro_real, posicion_compuesta.rem), &bit_esFree, 1);
	soltar_marco(marco);
}

void reset_isFree(uint32_t id, uint32_t inicio_heap) {
	uint8_t bit_esFree = 0;
	div_t posicion_compuesta = div(inicio_heap + 8, config_memoria.tamanio_pagina);

	t_marco *marco = obtener_marco(id, posicion_compuesta.quot);
	memcpy(inicio_memoria(marco->nro_real, posicion_compuesta.rem), &bit_esFree, 1);
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

