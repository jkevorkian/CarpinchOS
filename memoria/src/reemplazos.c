#include "reemplazos.h"

t_marco *buscar_por_clock(t_marco **lista_paginas, uint32_t nro_paginas) {
	bool encontre_marco = false;
	t_marco* marco_referencia;
	uint32_t puntero_clock = 0;
	uint8_t ciclo = 0;
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
		if(puntero_clock == nro_paginas) {
			puntero_clock = 0;
			ciclo = ciclo ? 0 : 1;
		}
	}
	
	pthread_mutex_lock(&marco_referencia->mutex_info_algoritmo);
	marco_referencia->bit_uso = false;
	pthread_mutex_unlock(&marco_referencia->mutex_info_algoritmo);

	return marco_referencia;
}

t_marco *buscar_por_lru(t_marco **lista_paginas, uint32_t nro_paginas) {
	t_marco *marco_referencia = lista_paginas[0];
	t_marco *marco_siguiente;
	bool primero_mas_viejo;

	for(int i = 1; i < nro_paginas; i++) {
		marco_siguiente = lista_paginas[i];
		pthread_mutex_lock(&marco_referencia->mutex_info_algoritmo);	// TODO eliminar mutex
		pthread_mutex_lock(&marco_siguiente->mutex_info_algoritmo);
		primero_mas_viejo = primer_tiempo_mas_chico(marco_referencia->temporal, marco_siguiente->temporal);
		pthread_mutex_unlock(&marco_referencia->mutex_info_algoritmo);
		pthread_mutex_unlock(&marco_siguiente->mutex_info_algoritmo);

		if(!primero_mas_viejo)	marco_referencia = marco_siguiente;
	}

	pthread_mutex_lock(&marco_referencia->mutex_info_algoritmo);
	marco_referencia->bit_uso = false;
	pthread_mutex_unlock(&marco_referencia->mutex_info_algoritmo);

	return marco_referencia;
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