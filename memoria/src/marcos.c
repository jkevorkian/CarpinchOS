#include "marcos.h"

t_marco** paginas_reemplazo(uint32_t id_carpincho);
bool tengo_marcos_suficientes(uint32_t);

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_entrada_tp *entrada_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
	if(!entrada_tp) {
		log_warning(logger, "No existe entrada tp");
	}
	
	t_entrada_tlb *entrada_tlb;
	t_marco* marco;
	
	if((entrada_tlb = leer_tlb(entrada_tp))) {
		// TLB hit
		marco = memoria_ram.mapa_fisico[entrada_tlb->marco];
	}
	else {
		// TLB miss
		if(entrada_tp->presencia) {
			// Page hit
			marco = memoria_ram.mapa_fisico[entrada_tp->marco];
		}
		else {
			// Page fault
			marco = incorporar_pagina(entrada_tp);
			entrada_tlb = actualizar_entradas(marco, entrada_tp);
		}

		if(!entrada_tlb) {
			entrada_tlb = asignar_entrada_tlb(entrada_tp);
		}
		
		log_info(logger, "Entrada incorporada nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
			entrada_tlb->nro_entrada, id_carpincho, nro_pagina, marco->nro_real);
	}

	pthread_mutex_lock(&marco->mutex_espera_uso);
	pthread_mutex_unlock(&entrada_tlb->mutex);
	entrada_tp->esta_vacia = false;
	pthread_mutex_unlock(&entrada_tp->mutex);

	log_info(logger, "Obtengo marco %d (car %d, pag %d)", marco->nro_real, id_carpincho, nro_pagina);
	return marco;
}

t_marco *incorporar_pagina(t_entrada_tp *entrada_tp) {
	t_marco** marcos_carpincho = paginas_reemplazo(entrada_tp->id);
	uint32_t nro_paginas = nro_paginas_reemplazo();

	t_marco* marco_a_reemplazar = NULL;
	pthread_mutex_lock(&mutex_asignacion_marcos);

	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL) {
		// Obtengo un marco libre de la memoria
		marco_a_reemplazar = obtener_marco_libre();
		if(marco_a_reemplazar) {
			log_info(logger, "Obtuve marco libre %d", marco_a_reemplazar->nro_real);
			soltar_marco(marco_a_reemplazar);
		}
	}
	else {
		// Si una de las paginas asignadas del carpincho est libre, la uso.
		for(int i = 0; i < config_memoria.cant_marcos_carpincho; i++) {
			if(marcos_carpincho[i]->pagina_duenio == -1) {
				marco_a_reemplazar = marcos_carpincho[i];
				break;
			}
		}
	}
	
	if(!marco_a_reemplazar) {
		log_warning(logger, "No encontre marco libre. Hago reemplazo");
		if(config_memoria.algoritmo_reemplazo == LRU)
			marco_a_reemplazar = buscar_por_lru(marcos_carpincho, nro_paginas);
		if(config_memoria.algoritmo_reemplazo == CLOCK)
			marco_a_reemplazar = buscar_por_clock(marcos_carpincho, nro_paginas);
		
		reasignar_marco(marco_a_reemplazar, entrada_tp);
	}

	if(!entrada_tp->esta_vacia) {			// SWAP IN
		log_info(logger, "Hago swap in de la pagina incorporada");
		log_info(logger, "Marco a actualizar de swap: %d", marco_a_reemplazar->nro_real);
		
		void* buffer = malloc(config_memoria.tamanio_pagina);
		crear_movimiento_swap(GET_PAGE, entrada_tp->id, entrada_tp->pagina, buffer);
		memcpy(inicio_memoria(marco_a_reemplazar->nro_real, 0), buffer, config_memoria.tamanio_pagina);
		// free(buffer);
		
		// >>>>>>>>>>>>>>>>>>> Para testear
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco_a_reemplazar->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		loggear_pagina(logger, buffer);
		free(pagina_generica);
		// <<<<<<<<<<<<<<<<<<<

		free(buffer);
	}
	else {
		memset(inicio_memoria(marco_a_reemplazar->nro_real, 0), 0, config_memoria.tamanio_pagina);
	}
	
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)	free(marcos_carpincho);
	
	return marco_a_reemplazar;
}

void reasignar_marco(t_marco* marco, t_entrada_tp *entrada_tp) {
	uint32_t id_viejo = marco->duenio;
	uint32_t nro_pagina_vieja = marco->pagina_duenio;

	log_info(logger, "Reemplazo de paginas en memoria. Marco %d", marco->nro_real);
	log_info(logger, "Pagina victima. Id: %d, nro_pagina: %d", id_viejo, nro_pagina_vieja);
	log_info(logger, "Pagina incorporada. Id: %d, nro_pagina: %d", entrada_tp->id, entrada_tp->pagina);

	bool hago_swap_out;

	pthread_mutex_lock(&marco->mutex_info_algoritmo);	 // no se si hace falta
	hago_swap_out = marco->bit_modificado;
	pthread_mutex_unlock(&marco->mutex_info_algoritmo);
	
	// Actualizo la tabla de paginas del carpincho
	entrada_tp->marco = marco->nro_real;
	entrada_tp->presencia = true;

	if(hago_swap_out) {			// SWAP OUT
		log_info(logger, "Hago swap out de la pagina victima");
		log_info(logger, "Marco a enviar a swap: %d", marco->nro_real);

		void* buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		crear_movimiento_swap(SET_PAGE, id_viejo, nro_pagina_vieja, buffer);
		free(buffer);

		// >>>>>>>>>>>>>>>>>>>
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);
		// <<<<<<<<<<<<<<<<<<<
	}
}

t_entrada_tlb *actualizar_entradas(t_marco *marco, t_entrada_tp *entrada_tp) {
	t_entrada_tlb *entrada_tlb = NULL;
	// Actualizo tabla de paginas del carpincho que perdio el marco
	t_entrada_tp *entrada_vieja_tp = pagina_de_carpincho(marco->duenio, marco->pagina_duenio);
	if(entrada_vieja_tp) {
		entrada_vieja_tp->presencia = false;
		// Si la pagina saliente tenia entrada tlb, utilizo esa para la nueva pagina
		entrada_tlb = reemplazar_entrada_tlb(entrada_vieja_tp, entrada_tp);
		pthread_mutex_unlock(&entrada_vieja_tp->mutex);
		log_info(logger, "Actualizo pagina anterior");
	}

	// Actualizo valor del marco
	pthread_mutex_lock(&marco->mutex_espera_uso);
	marco->duenio = entrada_tp->id;
	marco->pagina_duenio = entrada_tp->pagina;
	marco->bit_modificado = false;
	pthread_mutex_unlock(&marco->mutex_espera_uso);
	
	entrada_tp->marco = marco->nro_real;
	entrada_tp->presencia = true;

	pthread_mutex_unlock(&mutex_asignacion_marcos);

	return entrada_tlb;
}

void reservar_marco(t_marco *marco) {
	pthread_mutex_lock(&marco->mutex_espera_uso);
}

void soltar_marco(t_marco *marco) {
	pthread_mutex_unlock(&marco->mutex_espera_uso);
}

t_marco* obtener_marco_libre() {
    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		reservar_marco(marco);
		if (marco->duenio == 0) {
			marco->duenio = -1;
			return marco;
		}
		soltar_marco(marco);
    }

	return NULL;
}

uint32_t cant_marcos_faltantes(uint32_t id, uint32_t offset) {
	t_carpincho *carpincho = carpincho_de_lista(id);
	pthread_mutex_lock(&carpincho->mutex_tabla);
	uint32_t paginas_asignadas = list_size(carpincho->tabla_paginas);
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	div_t nro_marcos = div(offset, config_memoria.tamanio_pagina);
	uint32_t nro_marcos_necesarios = nro_marcos.quot;

	if(nro_marcos.rem > 0) nro_marcos_necesarios++;
	uint32_t nro_marcos_faltantes = nro_marcos_necesarios > paginas_asignadas ? nro_marcos_necesarios - paginas_asignadas : 0;
	
	return nro_marcos_faltantes;
}

bool tengo_marcos_suficientes(uint32_t necesarios){
	uint32_t contador_necesarios = necesarios;

    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->duenio == 0) {
			contador_necesarios--;
		}
        if(contador_necesarios == 0) return true;
    }
	
	return false;
}

bool asignacion_fija(t_carpincho* carpincho) {
	uint32_t cant_marcos = config_memoria.cant_marcos_carpincho;
	bool resultado = false;
	pthread_mutex_lock(&mutex_asignacion_marcos);
	if(tengo_marcos_suficientes(cant_marcos) && crear_movimiento_swap(NEW_PAGE, carpincho->id, cant_marcos, NULL)) {
		for(int i = 0; i < cant_marcos; i++){
			t_marco* marco = obtener_marco_libre();
			marco->duenio = carpincho->id;
			marco->pagina_duenio = -1;
			soltar_marco(marco);
			
			t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));			
			pthread_mutex_init(&pagina->mutex, NULL);
			pagina->presencia = false;
			pagina->id = carpincho->id;
			pagina->pagina = i;
			pagina->esta_vacia = true;

			pthread_mutex_lock(&carpincho->mutex_tabla);
			list_add(carpincho->tabla_paginas, pagina);
			pthread_mutex_unlock(&carpincho->mutex_tabla);

			log_info(logger, "Creo pagina %d para el carpincho %d", i, carpincho->id);
		}
		resultado = true;
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);

	return resultado;
}

void suspend(uint32_t id) {
	// Limpio entradas de la tlb
	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = tlb.mapa[i];
		if(entrada->id == id){
			pthread_mutex_lock(&mutex_asignacion_tlb);
			pthread_mutex_lock(&entrada->mutex);
			entrada->id = 0;
			pthread_mutex_unlock(&entrada->mutex);
			pthread_mutex_unlock(&mutex_asignacion_tlb);
		}
	}

	// Libero marcos de memoria
	uint32_t cant_marcos;
	t_marco **lista_marcos = obtener_marcos_proceso(id, &cant_marcos);

	pthread_mutex_lock(&mutex_asignacion_marcos);
	for(int i = 0; i < cant_marcos; i++) {
		t_entrada_tp *pagina = pagina_de_carpincho(id, lista_marcos[i]->pagina_duenio);
		if(pagina) {
			void *buffer = malloc(config_memoria.tamanio_pagina);
			memcpy(buffer, inicio_memoria(lista_marcos[i]->nro_real, 0), config_memoria.tamanio_pagina);
			crear_movimiento_swap(SET_PAGE, id, pagina->pagina, buffer);

			pagina->presencia = false;
			pthread_mutex_unlock(&pagina->mutex);
		}
		lista_marcos[i]->duenio = 0;
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
}

void unsuspend(uint32_t id) {
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL)
		return;
	
	pthread_mutex_lock(&mutex_asignacion_marcos);
	for(int i = 0; i < config_memoria.cant_marcos_carpincho; i++) {
		t_marco *marco_nuevo = obtener_marco_libre();
		marco_nuevo->duenio = id;
		marco_nuevo->pagina_duenio = -1;	// Esta asignada al proceso pero no es ninguna pagina especifica
		marco_nuevo->bit_modificado = false;
		marco_nuevo->bit_uso = false;
		soltar_marco(marco_nuevo);
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
}

t_marco** paginas_reemplazo(uint32_t id_carpincho) {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return obtener_marcos_proceso(id_carpincho, NULL);
	else
		return memoria_ram.mapa_fisico;
}

t_entrada_tp* agregar_pagina(uint32_t id_carpincho) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));

	pthread_mutex_lock(&carpincho->mutex_tabla);
	pagina->pagina = list_size(carpincho->tabla_paginas);
	list_add(carpincho->tabla_paginas, pagina);
	pthread_mutex_unlock(&carpincho->mutex_tabla);
	
	pagina->id = id_carpincho;
	pagina->presencia = false;
	pagina->esta_vacia = true;
	pthread_mutex_init(&pagina->mutex, NULL);

	log_info(logger, "Creo pagina %d para el carpincho %d", pagina->pagina, carpincho->id);

	return pagina;
}

void eliminar_pagina(uint32_t id, uint32_t nro_pagina) {
	log_info(logger, "Remuevo pagina %d del carpincho %d", nro_pagina, id);
	t_carpincho *carpincho = carpincho_de_lista(id);
	
	t_entrada_tp *entrada_tp;
	t_marco *marco;

	if(config_memoria.tipo_asignacion == FIJA_LOCAL) {
		// Elimino entrada tlb
		borrar_pagina_carpincho_tlb(id, nro_pagina);

		// Elimino entrada de tabla de paginas
		// En fija_local no se remueven las paginas que sobran porque la swap las conserva igualmente
		entrada_tp = pagina_de_carpincho(id, nro_pagina);
		entrada_tp->esta_vacia = true;
		pthread_mutex_unlock(&entrada_tp->mutex);

		// Libero marco si corresponde
		if(entrada_tp->presencia) {
			marco = memoria_ram.mapa_fisico[entrada_tp->marco];
			pthread_mutex_lock(&mutex_asignacion_marcos);
			marco->pagina_duenio = -1;
			marco->bit_modificado = false;
			marco->bit_uso = false;
			memset(inicio_memoria(marco->nro_real, 0), 0, config_memoria.tamanio_pagina);
			pthread_mutex_unlock(&mutex_asignacion_marcos);
		}
	}
	else {
		// Elimino entrada_tps tlb
		borrar_pagina_carpincho_tlb(id, nro_pagina);

		// Elimino tablas de paginas
		pthread_mutex_lock(&carpincho->mutex_tabla);
		entrada_tp = list_remove(carpincho->tabla_paginas, list_size(carpincho->tabla_paginas) - 1);
		pthread_mutex_unlock(&carpincho->mutex_tabla);
		
		// Libero marcos si corresponde
		if(entrada_tp->presencia) {
			marco = memoria_ram.mapa_fisico[entrada_tp->marco];
			pthread_mutex_lock(&mutex_asignacion_marcos);
			marco->duenio = 0;
			marco->bit_modificado = false;
			marco->bit_uso = false;
			memset(inicio_memoria(marco->nro_real, 0), 0, config_memoria.tamanio_pagina);
			pthread_mutex_unlock(&mutex_asignacion_marcos);
		}
		free(entrada_tp);

		// Aviso a swap que elimino pagina
		crear_movimiento_swap(RM_PAGE, id, 0, NULL);
	}
}

void liberar_paginas_carpincho(uint32_t id_carpincho, uint32_t desplazamiento) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	carpincho->offset = desplazamiento;
	
	div_t posicion_compuesta = div(desplazamiento, config_memoria.tamanio_pagina);
	
	uint32_t paginas_minimas = posicion_compuesta.quot;
	if(posicion_compuesta.rem)
		paginas_minimas++;
	
	pthread_mutex_lock(&carpincho->mutex_tabla);
	uint32_t cant_paginas_iniciales = list_size(carpincho->tabla_paginas);
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	for(int i = cant_paginas_iniciales; i > paginas_minimas; i--) {
		eliminar_pagina(id_carpincho, i - 1);
	}
}