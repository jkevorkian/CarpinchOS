#include "patota.h"

uint32_t creo_segmento_pcb(uint32_t, uint32_t);
uint32_t creo_segmento_tareas(uint32_t, uint32_t, char**, uint32_t*);
void segmentar_pcb_p(uint32_t, uint32_t, char**);

bool iniciar_patota(uint32_t id_patota, t_list* parametros) {
	uint32_t tamanio_pcb = TAMANIO_PATOTA;	// TO CLEAN
	uint32_t tamanio_bloque_tareas = 0;
	uint32_t tamanio_tripulantes = (uint32_t)list_get(parametros, 1) * TAMANIO_TRIPULANTE;

	uint32_t tamanio_tarea = 0;
	uint32_t cantidad_tareas = (uint32_t)list_get(parametros, 2);

	for(int i = 0; i < cantidad_tareas; i++) {
		tamanio_bloque_tareas += strlen((char *)list_get(parametros, 3 + i));
	}

	// Valido que haya memoria desponible en la memoria ram
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		if(TAMANIO_PATOTA + tamanio_bloque_tareas + tamanio_tripulantes > memoria_libre_segmentacion())
			return false;
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		if(frames_necesarios(0, TAMANIO_PATOTA + tamanio_bloque_tareas + tamanio_tripulantes) > marcos_logicos_disponibles())
			return false;
	}
	// log_info(logger, "Hay memoria disponible");

	tareas_data* nuevo_bloque_tareas = malloc(sizeof(tareas_data));
	nuevo_bloque_tareas->cant_tareas = cantidad_tareas;
	nuevo_bloque_tareas->inicio_tareas = malloc(sizeof(uint32_t) * cantidad_tareas);
	nuevo_bloque_tareas->tamanio_tareas = malloc(sizeof(uint32_t) * cantidad_tareas);
	list_add(lista_tareas, nuevo_bloque_tareas);

	char** vtareas = malloc(sizeof(char *) * cantidad_tareas);

	tamanio_bloque_tareas = 0;
	for(int i = 0; i < cantidad_tareas; i++) {
		char* tarea_i = (char *)list_get(parametros, 3 + i);
		tamanio_tarea = strlen(tarea_i);
		nuevo_bloque_tareas->inicio_tareas[i] = tamanio_bloque_tareas;
		tamanio_bloque_tareas += tamanio_tarea;
		vtareas[i] = tarea_i;
		nuevo_bloque_tareas->tamanio_tareas[i] = tamanio_tarea;
		// log_info(logger, "Recibo tarea: %s. Inicio = %d/%d. Tamanio = %d/%d", tarea_i, tamanio_bloque_tareas, nuevo_bloque_tareas->inicio_tareas[i], tamanio_tarea, nuevo_bloque_tareas->tamanio_tareas[i]);
	}

	// CREO ESTRUCTURA PATOTA
	patota_data* nueva_patota = malloc(sizeof(patota_data));
	nueva_patota->PID = id_patota;
	nueva_patota->inicio_elementos = calloc((2 + (uint32_t)list_get(parametros, 1)), sizeof(uint32_t));
	nueva_patota->cantidad_elementos = 2 + (uint32_t)list_get(parametros, 1);
	nueva_patota->cant_frames = 0;
	nueva_patota->memoria_ocupada = 0;
	list_add(lista_patotas, nueva_patota);

	log_info(logger, "Cree estructura patota");
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		nueva_patota->inicio_elementos[0] = creo_segmento_pcb(tamanio_pcb, id_patota);
		nueva_patota->inicio_elementos[1] = creo_segmento_tareas(tamanio_bloque_tareas, id_patota, vtareas, &nueva_patota->inicio_elementos[0]);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		asignar_frames(id_patota, frames_necesarios(0, TAMANIO_PATOTA + tamanio_bloque_tareas));
		nueva_patota->inicio_elementos[0] = 0;
		nueva_patota->inicio_elementos[1] = TAMANIO_PATOTA;
		segmentar_pcb_p(id_patota, cantidad_tareas, vtareas);
	}
	return true;
}

uint32_t creo_segmento_pcb(uint32_t tamanio_pcb, uint32_t id_patota) {
	t_segmento* segmento_pcb = crear_segmento(tamanio_pcb);
	segmento_pcb->duenio = id_patota;
	segmento_pcb->indice = 0;
	actualizar_entero_segmentacion(segmento_pcb, 0, id_patota);
	return segmento_pcb->inicio;
}

uint32_t creo_segmento_tareas(uint32_t tamanio_bloque_tareas, uint32_t id_patota, char** vector_tareas, uint32_t* inicio_pcb) {
	tareas_data* mis_tareas = (tareas_data *)list_get(lista_tareas, id_patota - 1);
	t_segmento* segmento_pcb = segmento_desde_inicio(*inicio_pcb);
	t_segmento* segmento_tareas = crear_segmento(tamanio_bloque_tareas);
	segmento_tareas->duenio = id_patota;
	segmento_tareas->indice = 1;

	for(int i = 0; i < mis_tareas->cant_tareas; i++) {
		memcpy(memoria_ram.inicio + segmento_tareas->inicio + mis_tareas->inicio_tareas[i], vector_tareas[i], mis_tareas->tamanio_tareas[i]);
		free(vector_tareas[i]);
	}
	free(vector_tareas);
	
	*inicio_pcb = segmento_pcb->inicio;
	actualizar_entero_segmentacion(segmento_pcb, sizeof(uint32_t), segmento_tareas->inicio);
	return segmento_tareas->inicio;
}

void segmentar_pcb_p(uint32_t id_patota, uint32_t cant_tareas, char** tareas) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	tareas_data* mis_tareas = (tareas_data *)list_get(lista_tareas, id_patota - 1);
	uint32_t pagina_actual = 0;
	
	actualizar_entero_paginacion(id_patota, 0, id_patota);
	actualizar_entero_paginacion(id_patota, sizeof(uint32_t), mi_patota->frames[0] * TAMANIO_PAGINA + TAMANIO_PATOTA);

	log_info(logger, "Patota %d. Valores: %d-%d", id_patota, obtener_entero_paginacion(id_patota, 0), obtener_entero_paginacion(id_patota, 4));
	// log_info(logger, "Sigo");
	mi_patota->memoria_ocupada += TAMANIO_PATOTA;
	uint32_t bytes_disponibles = TAMANIO_PAGINA - TAMANIO_PATOTA;
	div_t posicion_compuesta;
	// log_info(logger, "Memoria ocupada: %d", mi_patota->memoria_ocupada);
	for(int i = 0; i < cant_tareas; i++) {
		posicion_compuesta = div(mi_patota->memoria_ocupada, TAMANIO_PAGINA);
		pagina_actual = posicion_compuesta.quot;
		bytes_disponibles = TAMANIO_PAGINA - posicion_compuesta.rem;
		uint32_t bytes_cargados = 0;

		while(bytes_cargados < mis_tareas->tamanio_tareas[i]) {
			if(bytes_disponibles > mis_tareas->tamanio_tareas[i] - bytes_cargados)
				bytes_disponibles = mis_tareas->tamanio_tareas[i] - bytes_cargados;
			
			t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];	
			sem_wait(&marco_auxiliar->semaforo_mutex);
			incorporar_marco(mi_patota->frames[pagina_actual]);
			memcpy(inicio_marco(mi_patota->frames[pagina_actual]) + (mi_patota->memoria_ocupada % TAMANIO_PAGINA),
				tareas[i] + bytes_cargados, bytes_disponibles);
			marco_auxiliar->modificado = true;
			marco_auxiliar->bit_uso = true;
			sem_post(&marco_auxiliar->semaforo_mutex);

			bytes_cargados += bytes_disponibles;
			mi_patota->memoria_ocupada += bytes_disponibles;
			bytes_disponibles = TAMANIO_PAGINA;			
			pagina_actual++;
		}

		// log_info(logger, "Memoria ocupada total: %d, pagina: %d", mi_patota->memoria_ocupada, posicion_compuesta.rem);
		free(tareas[i]);
	}
	log_info(logger, "Segmentar pcb. Patota %d. Valores: %d-%d", id_patota, obtener_entero_paginacion(id_patota, 0), obtener_entero_paginacion(id_patota, 4));
	// log_info(logger, "Dejo de iterar");
	free(tareas);
}

char* obtener_tarea(uint32_t id_patota, uint32_t nro_tarea) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	tareas_data* mis_tareas = (tareas_data *)list_get(lista_tareas, id_patota - 1);
	
	#define bytes_necesarios (mis_tareas->tamanio_tareas[nro_tarea])
	char * tarea = malloc(bytes_necesarios + 1);
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		// TO DO SEMAFOROS
		memcpy(tarea, memoria_ram.inicio + mi_patota->inicio_elementos[1] + mis_tareas->inicio_tareas[nro_tarea], bytes_necesarios);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		uint32_t inicio_tarea = mi_patota->inicio_elementos[1] + mis_tareas->inicio_tareas[nro_tarea];
		div_t posicion_compuesta = div(inicio_tarea, TAMANIO_PAGINA);
		uint32_t bytes_cargados = 0;
		uint32_t bytes_disponibles = TAMANIO_PAGINA - posicion_compuesta.rem;
		uint32_t pagina_actual = posicion_compuesta.quot;
		uint32_t inicio_pagina = posicion_compuesta.rem;

		// uint32_t bytes_necesarios = mis_tareas->tamanio_tareas[nro_tarea];
		while(bytes_cargados < bytes_necesarios) {
			if(bytes_disponibles > bytes_necesarios - bytes_cargados)
				bytes_disponibles = bytes_necesarios - bytes_cargados;
			t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
			sem_wait(&marco_auxiliar->semaforo_mutex);
			memcpy(tarea + bytes_cargados, inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, bytes_disponibles);
			marco_auxiliar->bit_uso = true;
			sem_post(&marco_auxiliar->semaforo_mutex);
			bytes_cargados += bytes_disponibles;
			pagina_actual++;
			bytes_disponibles = TAMANIO_PAGINA;
			inicio_pagina = 0;
			// log_info(logger, "Obtuve fragmento %s", tarea);
		}
		// log_info(logger, "Obtuve_tarea. Inicio: pagina %d, byte n.o %d. Final: pagina %d", posicion_compuesta.quot, posicion_compuesta.rem, pagina_actual - 1);
	}
	
	char final = '\0';
	memcpy(tarea + mis_tareas->tamanio_tareas[nro_tarea], &final, 1);
	return tarea;
}

bool patota_sin_tripulantes(uint32_t id_patota) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	bool sin_tripulantes = true;
	for(int i = 2; i < mi_patota->cantidad_elementos; i++) {
		if(posicion_trip(id_patota, i - 1) != -1) {
			sin_tripulantes = false;
			break;
		}
	}
	return sin_tripulantes;
}

void eliminar_tareas(uint32_t id_patota) {
	patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
	
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		sem_wait(&mutex_compactacion);
		eliminar_segmento(nro_segmento_desde_inicio(mi_patota->inicio_elementos[1]));
		sem_post(&mutex_compactacion);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		mi_patota->memoria_ocupada = TAMANIO_PATOTA;
		reasignar_frames(id_patota);
	}
}