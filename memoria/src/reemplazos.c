#include "reemplazos.h"

t_marco *buscar_por_clock(t_marco **lista_paginas, uint32_t nro_paginas) {
	t_marco* marco_referencia;
	t_carpincho *carpincho;
	
	uint32_t puntero_clock_inicial;

	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL)
		puntero_clock_inicial = memoria_ram.puntero_clock;
	else {
		carpincho = carpincho_de_lista(lista_paginas[0]->duenio);
		puntero_clock_inicial = carpincho->puntero_clock;
	}
	
	log_info(logger, "Puntero clock en posicion inicial %d. Nro paginas %d", puntero_clock_inicial, nro_paginas);
	print_marcos_clock();

	bool encontre_marco = false;
	uint8_t ciclo = 0;
	uint32_t puntero_clock = puntero_clock_inicial;

	while(!encontre_marco) {
		marco_referencia = lista_paginas[puntero_clock];

		pthread_mutex_lock(&marco_referencia->mutex_info_algoritmo);
		switch(ciclo) {
		case 0:
			if(!marco_referencia->bit_uso && !marco_referencia->bit_modificado)
				encontre_marco = true;
			break;
		case 1:
			if(!marco_referencia->bit_uso && marco_referencia->bit_modificado)
				encontre_marco = true;
			else
				marco_referencia->bit_uso = 0;
			break;
		}
		pthread_mutex_unlock(&marco_referencia->mutex_info_algoritmo);

		puntero_clock++;

		if(puntero_clock == nro_paginas)
			puntero_clock = 0;

		if(puntero_clock == puntero_clock_inicial)
			ciclo = ciclo ? 0 : 1;

		/*memoria_ram.puntero_clock++;

		if(memoria_ram.puntero_clock == nro_paginas)
			memoria_ram.puntero_clock = 0;

		if(memoria_ram.puntero_clock == puntero_clock_inicial)
			ciclo = ciclo ? 0 : 1;*/
	}
	log_info(logger, "Puntero clock en posicion final %d", memoria_ram.puntero_clock);
	print_marcos_clock();

	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL)
		memoria_ram.puntero_clock = puntero_clock;
	else {
		carpincho->puntero_clock = puntero_clock;
	}
	
	return marco_referencia;
}

t_marco *buscar_por_lru(uint32_t id, uint32_t pagina) {
	bool entrada_a_remover_fija(void *entrada_fake) {
		if(((t_entrada_lru *)entrada_fake)->id == id)
			return true;
		else
			return false;
	}

	pthread_mutex_lock(&mutex_lista_lru);
	t_entrada_lru *entrada_lru;
	if(config_memoria.tipo_asignacion == FIJA_LOCAL)
		entrada_lru = list_remove_by_condition(lista_lru, entrada_a_remover_fija);
	else
		entrada_lru = list_remove(lista_lru, 0);

	log_warning(logger, "Victima id %d pagina %d", entrada_lru->id, entrada_lru->pagina);

	t_marco *marco_referencia;
	for(int i = 0; i < config_memoria.cant_marcos; i++) {
		marco_referencia = memoria_ram.mapa_fisico[i];
		if(marco_referencia->duenio == entrada_lru->id && marco_referencia->pagina_duenio == entrada_lru->pagina)
			break;
	}

	log_warning(logger, "Victima id %d pagina %d", marco_referencia->duenio, marco_referencia->pagina_duenio);
	log_warning(logger, "Victimario id %d pagina %d", id, pagina);

	entrada_lru->id = id;
	entrada_lru->pagina = pagina;
	list_add(lista_lru, entrada_lru);

	for(int i = 0; i < config_memoria.cant_marcos; i++) {
		log_warning(logger, "Entradé %d. Id: %d pagina %d", i, ((t_entrada_lru *)list_get(lista_lru, i))->id, ((t_entrada_lru *)list_get(lista_lru, i))->pagina);
	}

	pthread_mutex_unlock(&mutex_lista_lru);

	return marco_referencia;
}

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado) {
	bool entrada_correcta(void *entrada) {
		if(((t_entrada_lru *)entrada)->id == marco_auxiliar->duenio && ((t_entrada_lru *)entrada)->pagina == marco_auxiliar->pagina_duenio)
			return true;
		else
			return false;
	}

	pthread_mutex_lock(&marco_auxiliar->mutex_info_algoritmo);
	if(modificado)
		marco_auxiliar->bit_modificado = true;

	if(config_memoria.algoritmo_reemplazo == LRU) {
		log_warning(logger, "Actualizo info algoritmo id %d pagina %d", marco_auxiliar->duenio, marco_auxiliar->pagina_duenio);
		pthread_mutex_lock(&mutex_lista_lru);
		t_entrada_lru *entrada_lru = list_remove_by_condition(lista_lru, entrada_correcta);
		list_add(lista_lru, entrada_lru);
		pthread_mutex_unlock(&mutex_lista_lru);
	}
	else
		marco_auxiliar->bit_uso = true;
	pthread_mutex_unlock(&marco_auxiliar->mutex_info_algoritmo);
}

bool primer_tiempo_mas_chico(char *tiempo1, char *tiempo2) {
	// Formato temporal para LRU de marcos: HH:MM:SS:mmm
	if(obtener_tiempo('H', tiempo1) > obtener_tiempo('H', tiempo2))	return 0;
	if(obtener_tiempo('H', tiempo1) < obtener_tiempo('H', tiempo2))	return 1;
	if(obtener_tiempo('M', tiempo1) > obtener_tiempo('M', tiempo2))	return 0;
	if(obtener_tiempo('M', tiempo1) < obtener_tiempo('M', tiempo2))	return 1;
	if(obtener_tiempo('S', tiempo1) > obtener_tiempo('S', tiempo2))	return 0;
	if(obtener_tiempo('S', tiempo1) < obtener_tiempo('S', tiempo2))	return 1;
	if(obtener_tiempo('m', tiempo1) > obtener_tiempo('m', tiempo2))	return 0;
	if(obtener_tiempo('m', tiempo1) < obtener_tiempo('m', tiempo2))	return 1;
	else	return 0;	// Para evitar warnings
}

uint32_t obtener_tiempo(char tipo, char *tiempo) {
	char tiempo_hs[2];
	char tiempo_ms[3];

    uint32_t tiempo_ent;
	switch(tipo) {
	case 'H':
		memcpy(tiempo_hs, tiempo, 2);
        tiempo_ent = atoi(tiempo_hs);
		break;
	case 'M':
		memcpy(tiempo_hs, tiempo + 3, 2);
        tiempo_ent = atoi(tiempo_hs);
		break;
	case 'S':
		memcpy(tiempo_hs, tiempo + 6, 2);
        tiempo_ent = atoi(tiempo_hs);
		break;
	case 'm':
		memcpy(tiempo_ms, tiempo + 9, 3);
		tiempo_ent = atoi(tiempo_ms);
        break;
	}

	return tiempo_ent;
}