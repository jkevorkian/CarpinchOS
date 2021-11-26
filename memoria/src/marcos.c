#include "marcos.h"
#include "tlb.h"

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_marco* marco;

	uint32_t nro_marco_tlb = leer_tlb(id_carpincho, nro_pagina);
	if(nro_marco_tlb != -1) {	// TLB hit
		marco = memoria_ram.mapa_fisico[nro_marco_tlb];
		reservar_marco(marco);
		return marco;
	}

	// TLB miss
	t_entrada_tp *entrada_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
	
	if(entrada_tp->presencia) {
		marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
		reservar_marco(marco);
		asignar_entrada_tlb(id_carpincho, nro_pagina);
	}
	else {	// Page fault
		marco = realizar_algoritmo_reemplazo(id_carpincho, nro_pagina);
		asignar_entrada_tlb(id_carpincho, nro_pagina);
	}
	log_info(logger, "Obtengo marco %d (pag %d, car %d)", marco->nro_real, nro_pagina, id_carpincho);
	return marco;
}

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado) {
	pthread_mutex_lock(&marco_auxiliar->mutex_info_algoritmo);
	if(modificado)
		marco_auxiliar->bit_modificado = true;

	if(config_memoria.algoritmo_reemplazo == LRU) {
		if(marco_auxiliar->temporal)	free(marco_auxiliar->temporal);
		marco_auxiliar->temporal = temporal_get_string_time("%H:%M:%S:%MS");
	}
	else
		marco_auxiliar->bit_uso = true;
	pthread_mutex_unlock(&marco_auxiliar->mutex_info_algoritmo);
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
	// t_carpincho *nuevo_carpincho = carpincho_de_lista(id_carpincho);
	// t_carpincho *viejo_carpincho = carpincho_de_lista(marco->duenio);

	uint32_t nro_pagina_vieja = marco->pagina_duenio;
	uint32_t id_viejo = marco->duenio;

	void* buffer = NULL;
	pthread_mutex_lock(&marco->mutex_espera_uso);
	if(marco->bit_modificado) {			// SWAP OUT
		// No necesita mutex porque el proceso no lo puede tomar y tampoco lo pueden hacer los demas
		// porque esta bloqueada la asignacion
		marco->bit_modificado = false;
		buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(marco->nro_real, 0), config_memoria.tamanio_pagina);
		crear_movimiento_swap(SET_PAGE, marco->duenio, marco->pagina_duenio, buffer);
	}

	// Actualizo tlb y tabla de paginas del que perdio el marco
	// obtener_control_tlb();
	t_entrada_tlb* entrada_vieja_tlb = solicitar_entrada_tlb(id_viejo, nro_pagina_vieja);
	// entrada_vieja->tiempo_lru = ...;
	entrada_vieja_tlb->id_car = id_carpincho;
	entrada_vieja_tlb->pagina = nro_pagina;
	// entrada_vieja->marco = ;		// Debería ser igual
	// liberar_control_tlb();

	//if(viejo_carpincho) {
	if(carpincho_de_lista(marco->duenio)) {
		t_entrada_tp *entrada_vieja = pagina_de_carpincho(id_viejo, nro_pagina_vieja);
		pthread_mutex_lock(&entrada_vieja->mutex);
		entrada_vieja->presencia = false;
		// borrar_pagina_carpincho_tlb(id_viejo, nro_pagina_vieja);	// Para mi no va
		pthread_mutex_unlock(&entrada_vieja->mutex);
	}
	
	// Corrijo valores del marco actual
	t_entrada_tp *entrada_nueva_tp = pagina_de_carpincho(id_carpincho, nro_pagina);
	// pthread_mutex_lock(&entrada_nueva_tp->mutex); -> Se bloquea, no entiendo por que?
	bool hago_swap_in = !entrada_nueva_tp->esta_vacia;
	entrada_nueva_tp->nro_marco = marco->nro_real;
	entrada_nueva_tp->presencia = true;
	// pthread_mutex_unlock(&entrada_nueva_tp->mutex);
	
	if(hago_swap_in) {	// SWAP IN
		buffer = malloc(config_memoria.tamanio_pagina);
		crear_movimiento_swap(GET_PAGE, marco->duenio, marco->pagina_duenio, buffer);
		memcpy(inicio_memoria(marco->nro_real, 0), buffer, config_memoria.tamanio_pagina);
		// free(buffer);
	}

	marco->duenio = id_carpincho;		// Útil (Necesario?) para identificar cambios de tabla de paginas
	marco->pagina_duenio = nro_pagina;	// Util para facilitar futuros reemplazos
	marco->bit_uso = false;
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

uint32_t cant_marcos_necesarios(uint32_t tamanio) {
	div_t nro_marcos = div(tamanio, config_memoria.tamanio_pagina);
	uint32_t nro_marcos_q = nro_marcos.quot;

	if(nro_marcos.rem > 0) nro_marcos_q++;
	return nro_marcos_q;
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

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco, t_carpincho* carpincho){
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
	pagina->nro_marco = nro_marco;
	pagina->presencia = true;
	pagina->esta_vacia = true;

	pthread_mutex_lock(&carpincho->mutex_tabla);
	list_add(carpincho->tabla_paginas, pagina);
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
	// log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);
	asignar_entrada_tlb(carpincho->id, list_size(carpincho->tabla_paginas) - 1);
	return pagina;
}

// TODO: funcion fantasma
bool agregar_pagina(uint32_t id_carpincho) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	if(crear_movimiento_swap(NEW_PAGE, id_carpincho, 1, NULL)) {
		t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));

		pthread_mutex_lock(&carpincho->mutex_tabla);
		list_add(carpincho->tabla_paginas, pagina);
		pthread_mutex_unlock(&carpincho->mutex_tabla);
		
		pagina->esta_vacia = true;
		pagina->presencia = false;
		return true;
	}
	else
		return false;
}

void suspend(uint32_t id) {
	uint32_t cant_marcos;
	t_marco **lista_marcos = obtener_marcos_proceso(id, &cant_marcos);

	for(int i = 0; i < cant_marcos; i++) {
		void *buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(lista_marcos[i]->nro_real, 0), config_memoria.tamanio_pagina);
		crear_movimiento_swap(SET_PAGE, id, lista_marcos[i]->pagina_duenio, buffer);

		lista_marcos[i]->libre = true;
		// actualizar tlb y tabla de paginas
	}
	flush_proceso_tlb(id);
}

void unsuspend(uint32_t id) {
	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL)
		return;
	// t_marco **lista_marcos = obtener_marcos_proceso(id);
	// uint32_t cant_marcos = sizeof(lista_marcos) / sizeof(t_marco *);

	// continuar
	/*for(int i = 0; i < cant_marcos; i++) {
		void *buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(lista_marcos[i]->nro_real, 0), config_memoria.tamanio_pagina);
		crear_movimiento_swap(GET_PAGE, id, lista_marcos[i]->pagina_duenio, buffer);
	}*/

	// TODO: agregar a tlb
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