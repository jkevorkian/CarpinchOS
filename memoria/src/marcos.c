#include "marcos.h"
#include "tlb.h"

t_marco** paginas_reemplazo(uint32_t id_carpincho);

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_marco* marco;
	t_entrada_tlb *entrada_tlb;
	
	if((entrada_tlb = leer_tlb(id_carpincho, nro_pagina))) {
		// TLB hit
		marco = memoria_ram.mapa_fisico[entrada_tlb->marco];
		// pthread_mutex_lock(&marco->mutex);
		// pthread_mutex_unlock(&entrada_tlb->mutex);
	}
	else {
		// TLB miss
		t_entrada_tp *entrada_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
		if(entrada_tp->presencia) {
			marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
			entrada_tlb = asignar_entrada_tlb(id_carpincho, nro_pagina);
			if(entrada_tlb->id_car) {
				log_info(logger, "Realizo reemplazo de entrada de tlb");
				log_info(logger, "Entrada victima nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
					entrada_tlb->nro_entrada, entrada_tlb->id_car, entrada_tlb->pagina, entrada_tlb->marco);
			}
			else
				log_info(logger, "Asigno entrada tlb a entrada nro %d, que estaba libre", entrada_tlb->nro_entrada);
			entrada_nueva(id_carpincho, nro_pagina, entrada_tlb);
		}
		else {	// Page fault
			marco = incorporar_pagina(id_carpincho, nro_pagina);
			// pthread_mutex_lock(&marco->mutex);
			// pthread_mutex_unlock(&entrada_tlb->mutex);
		}
	}
	reservar_marco(marco);

	log_info(logger, "Obtengo marco %d (car %d, pag %d)", marco->nro_real, id_carpincho, nro_pagina);
	return marco;
}

t_marco *incorporar_pagina(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_marco** marcos_carpincho = paginas_reemplazo(id_carpincho);
	uint32_t nro_paginas = nro_paginas_reemplazo();

	t_marco* marco_a_reemplazar = NULL;
	pthread_mutex_lock(&mutex_asignacion_marcos);
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL) {
		// Obtengo un marco libre de la memoria
		marco_a_reemplazar = obtener_marco_libre();
		asignar_marco_libre(marco_a_reemplazar, id_carpincho, nro_pagina);
	}

	if(config_memoria.tipo_asignacion == FIJA_LOCAL) {
		// Si una de las paginas asignadas del carpincho est libre, la uso.
		for(int i = 0; i < config_memoria.cant_marcos_carpincho; i++) {
			if(marcos_carpincho[i]->pagina_duenio == -1) {
				marco_a_reemplazar = marcos_carpincho[i];
				marco_a_reemplazar->pagina_duenio = nro_pagina;
				asignar_marco_libre(marco_a_reemplazar, id_carpincho, nro_pagina);
				break;
			}
		}
	}
		
	if(!marco_a_reemplazar) {
		if(config_memoria.algoritmo_reemplazo == LRU)
			marco_a_reemplazar = buscar_por_lru(marcos_carpincho, nro_paginas);
		if(config_memoria.algoritmo_reemplazo == CLOCK)
			marco_a_reemplazar = buscar_por_clock(marcos_carpincho, nro_paginas);
		
		reasignar_marco(marco_a_reemplazar, id_carpincho, nro_pagina);
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
	
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)	free(marcos_carpincho);
	
	return marco_a_reemplazar;
}

void asignar_marco_libre(t_marco *marco_nuevo, uint32_t id, uint32_t nro_pagina) {
	pthread_mutex_lock(&marco_nuevo->mutex_espera_uso);
	marco_nuevo->duenio = id;
	marco_nuevo->pagina_duenio = nro_pagina;
    marco_nuevo->libre = false;
	marco_nuevo->bit_modificado = false;
	marco_nuevo->bit_uso = false;
	pthread_mutex_unlock(&marco_nuevo->mutex_espera_uso);

	asignar_entrada_tlb(id, nro_pagina);
}

void reasignar_marco(t_marco* marco, uint32_t id_carpincho, uint32_t nro_pagina) {
	uint32_t id_viejo = marco->duenio;
	uint32_t nro_pagina_vieja = marco->pagina_duenio;

	log_info(logger, "Reemplazo de paginas en en marco %d", marco->nro_real);
	log_info(logger, "Pagina victima. Id: %d, nro_pagina: %d", id_viejo, nro_pagina_vieja);
	log_info(logger, "Pagina incorporada. Id: %d, nro_pagina: %d", id_carpincho, nro_pagina);

	bool hago_swap_in = false;
	bool hago_swap_out;

	pthread_mutex_lock(&marco->mutex_espera_uso);

	pthread_mutex_lock(&marco->mutex_info_algoritmo);	 // no se si hace falta
	hago_swap_out = marco->bit_modificado /* && !marco->libre */; // Para integracion con asignar pagina
	pthread_mutex_unlock(&marco->mutex_info_algoritmo);

	if(hago_swap_out) {			// SWAP OUT
		log_info(logger, "Hago swap out de la pagina victima");
		log_info(logger, "Marco a enviar a swap: %d", marco->nro_real);

		void* buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);

		// Para testear
		// >>>>>>>>>>>>>>>>>>>
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);
		// <<<<<<<<<<<<<<<<<<<
		
		crear_movimiento_swap(SET_PAGE, id_viejo, nro_pagina_vieja, buffer);
		free(buffer);
	}

	// Actualizo la tabla de paginas del carpincho
	t_entrada_tp *entrada_nueva_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
	if(entrada_nueva_tp) {
		pthread_mutex_lock(&entrada_nueva_tp->mutex);
		hago_swap_in = !entrada_nueva_tp->esta_vacia;
		entrada_nueva_tp->nro_marco = marco->nro_real;
		entrada_nueva_tp->presencia = true;
		pthread_mutex_unlock(&entrada_nueva_tp->mutex);
	}

	if(hago_swap_in) {			// SWAP IN
		log_info(logger, "Hago swap in de la pagina incorporada");
		log_info(logger, "Marco a actualizar de swap: %d", marco->nro_real);
		
		void* buffer = malloc(config_memoria.tamanio_pagina);
		crear_movimiento_swap(GET_PAGE, id_carpincho, nro_pagina, buffer);

		void *inicio = inicio_memoria(marco->nro_real, 0);
		memcpy(inicio, buffer, config_memoria.tamanio_pagina);
		
		free(buffer);
		
		// Para testear
		// >>>>>>>>>>>>>>>>>>>
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);
		// <<<<<<<<<<<<<<<<<<<
	}
	else {
		memset(inicio_memoria(marco->nro_real, 0), 0, config_memoria.tamanio_pagina);
	}

	// Actualizo tabla de paginas del carpincho que perdio el marco
	if(carpincho_de_lista(id_viejo)) {
		t_entrada_tp *entrada_vieja_tp = pagina_de_carpincho(id_viejo, nro_pagina_vieja);
		if(entrada_vieja_tp) {
			pthread_mutex_lock(&entrada_vieja_tp->mutex);
			entrada_vieja_tp->presencia = false;
			pthread_mutex_unlock(&entrada_vieja_tp->mutex);
		}
		
	}

	// Actualizo la entrada de la tlb
	t_entrada_tlb* entrada_tlb = obtener_entrada_intercambio_tlb(id_viejo, nro_pagina_vieja);
	if(entrada_tlb) {
		log_info(logger, "Realizo reemplazo de entrada de tlb con entrada de pagina saliente");
		log_info(logger, "Entrada victima nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
				entrada_tlb->nro_entrada, entrada_tlb->id_car, entrada_tlb->pagina, entrada_tlb->marco);
		
		pthread_mutex_lock(&entrada_tlb->mutex);
		entrada_tlb->id_car = id_carpincho;
		entrada_tlb->pagina = nro_pagina;
		entrada_tlb->marco = marco->nro_real;
		pthread_mutex_unlock(&entrada_tlb->mutex);
	}
	else {
		entrada_tlb = asignar_entrada_tlb(id_viejo, nro_pagina_vieja);
		if(entrada_tlb->id_car) {
			log_info(logger, "Realizo reemplazo de entrada de tlb");
			log_info(logger, "Entrada victima nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
				entrada_tlb->nro_entrada, entrada_tlb->id_car, entrada_tlb->pagina, entrada_tlb->marco);
		}
		else
			log_info(logger, "Asigno entrada tlb a entrada nro %d, que estaba libre", entrada_tlb->nro_entrada);
		
		entrada_nueva(id_carpincho, nro_pagina, entrada_tlb);
	}
	
	log_info(logger, "Entrada incorporada nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
		entrada_tlb->nro_entrada, id_carpincho, nro_pagina, marco->nro_real);
	
	// Actualizo valor del marco
	marco->duenio = id_carpincho;		// Util (Necesario?) para identificar cambios de tabla de paginas
	marco->pagina_duenio = nro_pagina;	// Util para facilitar futuros reemplazos
	marco->bit_uso = false;
	marco->bit_modificado = false;
	// marco_nuevo->libre = false;			// Necesario si quiero integrar con funcion de asignar_marco_libre

	pthread_mutex_unlock(&marco->mutex_espera_uso);
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
		if (marco->libre) {
			marco->libre = false;
			return marco;
		}
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
		if (marco->libre) {
			contador_necesarios--;
		}
        if(contador_necesarios == 0) return true;
    }
	
	return false;
}

t_entrada_tp* agregar_pagina(uint32_t id_carpincho) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));

	pthread_mutex_lock(&carpincho->mutex_tabla);
	list_add(carpincho->tabla_paginas, pagina);
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	pagina->presencia = false;
	pagina->esta_vacia = true;
	pthread_mutex_init(&pagina->mutex, NULL);
	return pagina;
}

bool asignacion_fija(t_carpincho* carpincho) {
	uint32_t cant_marcos = config_memoria.cant_marcos_carpincho;
	bool resultado = false;
	pthread_mutex_lock(&mutex_asignacion_marcos);
	if(tengo_marcos_suficientes(cant_marcos) && crear_movimiento_swap(NEW_PAGE, carpincho->id, cant_marcos, NULL)) {
		for(int i = 0; i < cant_marcos; i++){
			t_marco* marco = obtener_marco_libre();
			marco->duenio = carpincho->id;
			marco->pagina_duenio = i;
			
			t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));			
			pthread_mutex_init(&pagina->mutex, NULL);
			pagina->nro_marco = marco->nro_real;
			pagina->presencia = true;

			pthread_mutex_lock(&carpincho->mutex_tabla);
			list_add(carpincho->tabla_paginas, pagina);
			pthread_mutex_unlock(&carpincho->mutex_tabla);
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
		if(entrada->id_car == id){
			pthread_mutex_lock(&asignacion_entradas_tlb);
			pthread_mutex_lock(&entrada->mutex);
			entrada->id_car = 0;
			pthread_mutex_unlock(&entrada->mutex);
			pthread_mutex_unlock(&asignacion_entradas_tlb);
		}
	}

	// Libero marcos de memoria
	uint32_t cant_marcos;
	t_marco **lista_marcos = obtener_marcos_proceso(id, &cant_marcos);

	pthread_mutex_lock(&mutex_asignacion_marcos);
	for(int i = 0; i < cant_marcos; i++) {
		void *buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(lista_marcos[i]->nro_real, 0), config_memoria.tamanio_pagina);
		crear_movimiento_swap(SET_PAGE, id, lista_marcos[i]->pagina_duenio, buffer);

		t_entrada_tp *pagina = pagina_de_carpincho(id, lista_marcos[i]->pagina_duenio);
		pthread_mutex_lock(&pagina->mutex);
		pagina->presencia = false;
		pthread_mutex_unlock(&pagina->mutex);
		lista_marcos[i]->libre = true;
	}
	
	pthread_mutex_unlock(&mutex_asignacion_marcos);
}

void unsuspend(uint32_t id) {
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL)
		return;
	
	pthread_mutex_lock(&mutex_asignacion_marcos);
	for(int i = 0; i < config_memoria.cant_marcos_carpincho; i++) {
		t_marco *marco_nuevo = obtener_marco_libre();
		marco_nuevo->libre = false;
		marco_nuevo->duenio = id;
		marco_nuevo->pagina_duenio = -1;	// Esta asignada al proceso pero no es ninguna pagina especifica
		marco_nuevo->bit_modificado = false;
		marco_nuevo->bit_uso = false;
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
}

t_marco** paginas_reemplazo(uint32_t id_carpincho) {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return obtener_marcos_proceso(id_carpincho, NULL);
	else
		return memoria_ram.mapa_fisico;
}

void eliminar_pagina(uint32_t id, uint32_t nro_pagina) {
	t_carpincho *carpincho = carpincho_de_lista(id);
	
	t_entrada_tp *entrada_tp;
	t_marco *marco;

	if(config_memoria.tipo_asignacion == FIJA_LOCAL) {
		// Elimino entrada tlb
		borrar_pagina_carpincho_tlb(id, nro_pagina);

		// Elimino entrada de tabla de paginas
		// En fija_local no se remueven las paginas que sobran porque la swap las conserva igualmente
		entrada_tp = pagina_de_carpincho(id, nro_pagina);
		pthread_mutex_lock(&entrada_tp->mutex);
		entrada_tp->esta_vacia = true;
		pthread_mutex_unlock(&entrada_tp->mutex);

		// Libero marco si corresponde
		if(entrada_tp->presencia) {
			marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
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
			marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
			pthread_mutex_lock(&mutex_asignacion_marcos);
			marco->libre = true;
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
		eliminar_pagina(id_carpincho, i);
	}
}