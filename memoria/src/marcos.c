#include "marcos.h"

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	// uint32_t nro_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	log_info(logger, "Obtengo pagina %d del carpincho %d", nro_pagina, id_carpincho);
	t_marco* marco;
	t_carpincho* mi_carpincho = carpincho_de_lista(id_carpincho);
	if(!mi_carpincho) {
		log_warning(logger, "El carpincho no existe");
		return NULL;
	}
	marco = obtener_marco_mp(mi_carpincho, nro_pagina);
	if(!marco) {	// Page fault
		marco = realizar_algoritmo_reemplazo(id_carpincho, nro_pagina);
	}

	return marco;
}

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado) {
	if(modificado)
		marco_auxiliar->bit_modificado = true;

	if(config_memoria.algoritmo_reemplazo == LRU)
		marco_auxiliar->temporal = temporal_get_string_time("%H:%M:%S");
	else
		marco_auxiliar->bit_uso = true;
}

void reasignar_marco(t_marco* marco, uint32_t id_carpincho, uint32_t nro_pagina) {
	// actualizar_tp();
	t_carpincho *nuevo_carpincho = list_get(lista_carpinchos, id_carpincho - 1);
	t_carpincho *viejo_carpincho = list_get(lista_carpinchos, marco->duenio - 1);

	uint32_t nro_pagina_vieja = marco->pagina_duenio;
	uint32_t id_viejo = marco->duenio;

	void* buffer;
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
	t_entrada_tp *entrada_vieja = (t_entrada_tp *)list_get(viejo_carpincho->tabla_paginas, nro_pagina_vieja);
	
	pthread_mutex_lock(&entrada_vieja->mutex);
	entrada_vieja->presencia = false;
	pthread_mutex_unlock(&entrada_vieja->mutex);
	
	// Corrijo valores del marco actual
	t_entrada_tp *entrada_nueva = (t_entrada_tp *)list_get(nuevo_carpincho->tabla_paginas, nro_pagina);
	
	if(!entrada_nueva->esta_vacia) {	// SWAP IN
		if(!buffer)	buffer = malloc(config_memoria.tamanio_pagina);
		crear_movimiento_swap(SET_PAGE, marco->duenio, marco->pagina_duenio, buffer);
		memcpy(inicio_memoria(marco->nro_real, 0), buffer, config_memoria.tamanio_pagina);
	}
	if(buffer)	free(buffer);

	marco->duenio = id_carpincho;		// Útil (Necesario?) para identificar cambios de tabla de paginas
	marco->pagina_duenio = nro_pagina;	// Util para facilitar futuros reemplazos
	pthread_mutex_unlock(&marco->mutex_espera_uso);
}

t_marco *obtener_marco_mp(t_carpincho *mi_carpincho, uint32_t nro_pagina) {
	t_entrada_tp *entrada_tp = (t_entrada_tp *)list_get(mi_carpincho->tabla_paginas, nro_pagina);
	pthread_mutex_lock(&entrada_tp->mutex);
	bool presencia = entrada_tp->presencia;
	pthread_mutex_unlock(&entrada_tp->mutex);

	t_marco *marco;
	if(entrada_tp->presencia) {
		marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
		pthread_mutex_lock(&marco->mutex_espera_uso);
		if(marco->duenio == mi_carpincho->id)
			return marco;
		else {
			pthread_mutex_unlock(&marco->mutex_espera_uso);

			pthread_mutex_lock(&entrada_tp->mutex);
			entrada_tp->presencia = false;
			pthread_mutex_unlock(&entrada_tp->mutex);
			return NULL;
		}
	}
	return NULL;
}

/*bool reservar_marco(t_marco *marco_auxiliar, uint32_t id) {
	bool respuesta = true;
	

	return respuesta;
}*/

void soltar_marco(t_marco *marco_auxiliar) {
	pthread_mutex_unlock(&marco_auxiliar->mutex_espera_uso);
}

t_marco* obtener_marco_libre() {
    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			marco->libre = false; //mutex
			return marco;
		}
    }

    /* Esto no va
    t_marco* marco = pedir_frame_swap();
    */

	return NULL;
}

t_marco* obtener_marco_libre_mp() {
    t_marco* marco;
	for(int i = 0; i < config_memoria.cant_marcos; i++) {
        marco = memoria_ram.mapa_fisico[i];
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
	// no entiendo si yo aca devuelvo que tengo marcos necesarios pero despues mientras le asigno me quedo sin marcos porque los uso otro proceso que pasa?
    uint32_t contador_necesarios = necesarios;

    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			contador_necesarios--;
		}
        if(contador_necesarios == 0) return true;
    }

	if(crear_movimiento_swap(NEW_PAGE, /*id_carpincho*/1, contador_necesarios, NULL))
		return true;
	else
		return false;
}

t_marco* asignar_marco_libre(uint32_t nro_marco, uint32_t id) {
	pthread_mutex_lock(&memoria_ram.mutex_mapa);	// no se si hace falta
	t_marco* marco_nuevo = memoria_ram.mapa_fisico[nro_marco];
	pthread_mutex_unlock(&memoria_ram.mutex_mapa);
	
	// mutex ?
	marco_nuevo->duenio = id;
	marco_nuevo->pagina_duenio = nro_marco;
    marco_nuevo->libre = false;				// to remove o cambiar lo de duenio

	return marco_nuevo;
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco, t_carpincho* carpincho){
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
	list_add(carpincho->tabla_paginas, pagina);
	pagina->nro_marco = nro_marco;
	pagina->presencia = true;

	// -> Esto agregaría yo, si el marco está en memoria
	// t_marco* marco_nuevo = asignar_marco_libre(nro_marco, carpincho->id);
	// marco_nuevo->pagina_duenio = list_size(carpincho->tabla_paginas) - 1;
	// <- */

	log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
	// log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);

	return pagina;
}

t_entrada_tp* crear_nueva_pagina2(t_carpincho* carpincho){
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
	list_add(carpincho->tabla_paginas, pagina);
	// pagina->nro_marco = nro_marco;
	pagina->presencia = false;

	// -> Esto agregaría yo, si el marco está en memoria
	// t_marco* marco_nuevo = asignar_marco_libre(nro_marco, carpincho->id);
	// marco_nuevo->pagina_duenio = list_size(carpincho->tabla_paginas) - 1;
	// <- */

	log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
	// log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);

	return pagina;
}

bool agregar_pagina(uint32_t id_carpincho) {
	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);
	if(crear_movimiento_swap(NEW_PAGE, id_carpincho, 1, NULL)) {
		t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
		list_add(carpincho->tabla_paginas, pagina);
		pagina->presencia = false;
		return true;
	}
	else
		return false;
}

void suspend(uint32_t id) {
	t_marco **lista_marcos = obtener_marcos_proceso(id);
	uint32_t cant_marcos = sizeof(lista_marcos) / sizeof(t_marco *);

	for(int i = 0; i < cant_marcos; i++) {
		void *buffer = malloc(config_memoria.tamanio_pagina);
		memcpy(buffer, inicio_memoria(lista_marcos[i]->nro_real, 0), config_memoria.tamanio_pagina);
		crear_movimiento_swap(SET_PAGE, id, lista_marcos[i]->pagina_duenio, buffer);

		lista_marcos[i]->libre = true;
		// actualizar tlb y tabla de paginas
	}
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
}