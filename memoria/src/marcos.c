#include "marcos.h"
#include "tlb.h"

t_marco** paginas_reemplazo(uint32_t id_carpincho);

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_marco* marco;
	t_entrada_tp *entrada_tp;

	// uint32_t nro_marco_tlb = leer_tlb(id_carpincho, nro_pagina);
	// if((nro_marco_tlb != -1){
	t_entrada_tlb *entrada_tlb;
	if((entrada_tlb = leer_tlb(id_carpincho, nro_pagina))) {
		// TLB hit
		marco = memoria_ram.mapa_fisico[entrada_tlb->marco];
		// pthread_mutex_lock(&marco->mutex);
		// pthread_mutex_unlock(&entrada_tlb->mutex);
	}
	else {
		// TLB miss
		entrada_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
		if(entrada_tp->presencia) {
			marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
			asignar_entrada_tlb(id_carpincho, nro_pagina);
			// pthread_mutex_lock(&marco->mutex);
			// pthread_mutex_unlock(&entrada_tlb->mutex);
		}
		else {	// Page fault
			marco = incorporar_pagina(id_carpincho, nro_pagina);
			// pthread_mutex_lock(&marco->mutex);
			// pthread_mutex_unlock(&entrada_tlb->mutex);

			// Estaba
			// asignar_entrada_tlb(id_carpincho, nro_pagina);
		}
	}
	reservar_marco(marco);

	log_info(logger, "Obtengo marco %d (pag %d, car %d)", marco->nro_real, nro_pagina, id_carpincho);
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
		// Si una de las paginas asignadas del carpincho está libre, la uso.
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
		// actualizar_entrada_tlb(marco->nro_real, id_carpincho, nro_pagina);
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

	// ESTO ES NUEVO MIO
	asignar_entrada_tlb(id, nro_pagina);
}

void reasignar_marco(t_marco* marco, uint32_t id_carpincho, uint32_t nro_pagina) {
	uint32_t id_viejo = marco->duenio;
	uint32_t nro_pagina_vieja = marco->pagina_duenio;

	bool hago_swap_in;
	bool hago_swap_out;

	pthread_mutex_lock(&marco->mutex_espera_uso);

	pthread_mutex_lock(&marco->mutex_info_algoritmo);	 // no se si hace falta
	hago_swap_out = marco->bit_modificado /* && !marco->libre */; // Para integración con asignar pagina
	pthread_mutex_unlock(&marco->mutex_info_algoritmo);

	if(hago_swap_out) {			// SWAP OUT
		log_info(logger, "Hago swap out");

		void* buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);

		// Para testear
		// >>>>>>>>>>>>>>>>>>>
		log_info(logger, "Reasigno marco %d. Información a enviar:", marco->nro_real);
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);
		// <<<<<<<<<<<<<<<<<<<
		
		crear_movimiento_swap(SET_PAGE, id_viejo, nro_pagina_vieja, buffer);
		log_info(logger, "Libero buffer marcos.c : 83");
		free(buffer);
	}

	// Actualizo la tabla de paginas del carpincho
	t_entrada_tp *entrada_nueva_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
	log_info(logger, "Hago mutex entrada_nueva_tp en reasignación");

	if(entrada_nueva_tp) {
		log_info(logger, "Pagina %d, duenio %d, marco %d", nro_pagina, id_carpincho, entrada_nueva_tp->nro_marco);
		pthread_mutex_lock(&entrada_nueva_tp->mutex);	//	-> Se bloquea, no entiendo por qué
		hago_swap_in = !entrada_nueva_tp->esta_vacia;
		entrada_nueva_tp->nro_marco = marco->nro_real;
		entrada_nueva_tp->presencia = true;
		pthread_mutex_unlock(&entrada_nueva_tp->mutex);
	}

	if(hago_swap_in) {			// SWAP IN
		log_info(logger, "Hago swap in");

		void* buffer = malloc(config_memoria.tamanio_pagina);
		crear_movimiento_swap(GET_PAGE, id_carpincho, nro_pagina, buffer);

		log_info(logger, "Memcpy marcos.c. Nro. marco %d. Buffer = %p. Memoria %p", marco->nro_real, buffer, memoria_ram.inicio);
		
		void *inicio = inicio_memoria(marco->nro_real, 0);
		log_info(logger, "Desplazamiento inicio marco: %d", inicio - memoria_ram.inicio);
		memcpy(inicio, buffer, config_memoria.tamanio_pagina);
		
		log_info(logger, "Libero buffer marcos.c : 126");
		free(buffer);
		
		// Para testear
		// >>>>>>>>>>>>>>>>>>>
		log_info(logger, "Reasigno marco %d. Información recuperada:", marco->nro_real);
		void *pagina_generica = malloc(config_memoria.tamanio_pagina);
		memcpy(pagina_generica, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		loggear_pagina(logger, pagina_generica);
		free(pagina_generica);
		// <<<<<<<<<<<<<<<<<<<
	}

	// Actualizo tabla de paginas del carpincho que perdio el marco
	if(carpincho_de_lista(id_viejo)) {
		t_entrada_tp *entrada_vieja_tp = pagina_de_carpincho(id_viejo, nro_pagina_vieja);
		
		log_info(logger, "Hago mutex entrada_vieja_tp en reasignación");
		
		if(entrada_vieja_tp) {
			log_info(logger, "Pagina %d, duenio %d, marco %d", nro_pagina_vieja, id_viejo, entrada_vieja_tp->nro_marco);
			pthread_mutex_lock(&entrada_vieja_tp->mutex);	// -> Se bloquea, no entiendo por que
			entrada_vieja_tp->presencia = false;
			pthread_mutex_unlock(&entrada_vieja_tp->mutex);
		}
		
	}

	// Actualizo la entrada de la tlb
	t_entrada_tlb* entrada_tlb = obtener_entrada_tlb(id_viejo, nro_pagina_vieja);
	if(entrada_tlb) {
		entrada_tlb->id_car = id_carpincho;
		entrada_tlb->pagina = nro_pagina;
		entrada_tlb->marco = marco->nro_real;		// Debería ser igual
	}
	else {
		entrada_tlb = asignar_entrada_tlb(id_viejo, nro_pagina_vieja);
		entrada_tlb->id_car = id_carpincho;
		entrada_tlb->pagina = nro_pagina;
		entrada_tlb->marco = marco->nro_real;		// No debería ser igual
	}
	pthread_mutex_unlock(&entrada_tlb->mutex);
	

	// Actualizo valor del marco
	marco->duenio = id_carpincho;		// Útil (Necesario?) para identificar cambios de tabla de paginas
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

void suspend(uint32_t id) {
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

		// actualizar tlb
		// borrar_entrada_tlb(id, lista_marcos[i]->pagina_duenio);

		lista_marcos[i]->libre = true;
	}

	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = tlb.mapa[i];
		if(entrada->id_car == id){
			borrar_entrada_tlb(i);
		}
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
		marco_nuevo->pagina_duenio = -1;	// Está asignada al proceso pero no es ninguna página especifica
		marco_nuevo->bit_modificado = false;
		marco_nuevo->bit_uso = false;
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
}

t_entrada_tp *pagina_de_carpincho(uint32_t id, uint32_t nro_pagina) {
	t_carpincho *carpincho = carpincho_de_lista(id);
	t_entrada_tp *entrada;
	pthread_mutex_lock(&carpincho->mutex_tabla);
	entrada = (t_entrada_tp *)list_get(carpincho->tabla_paginas, nro_pagina);
	pthread_mutex_unlock(&carpincho->mutex_tabla);
	return entrada;
}

void liberar_marco(t_marco *marco) {
	pthread_mutex_lock(&marco->mutex_espera_uso);
	marco->duenio = 0;
	marco->libre = true;
	pthread_mutex_unlock(&marco->mutex_espera_uso);
}

t_marco** paginas_reemplazo(uint32_t id_carpincho) {
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		return obtener_marcos_proceso(id_carpincho, NULL);
	else
		return memoria_ram.mapa_fisico;
}