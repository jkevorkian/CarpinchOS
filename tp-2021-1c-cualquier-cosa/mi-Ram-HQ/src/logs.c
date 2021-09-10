#include "logs.h"

void dump() {
	FILE* archivo_dump;

    char* inicio = "Dump_";
	char* momento = temporal_get_string_time("%d-%m-%y_%H-%M-%S");
	char* extension = ".dmp";
	
	char nombreDeArchivo[strlen(momento) + strlen(extension) + strlen(inicio) + 1];
	sprintf(nombreDeArchivo, "%s%s%s", inicio, momento, extension);

    archivo_dump = fopen(nombreDeArchivo, "w");

    fprintf(archivo_dump, "\n");
    fprintf(archivo_dump, "------------------------------------------------------------------------------------\n");
    fprintf(archivo_dump, "Dump: %s\n", momento);

    log_info(logger, "Creo un archivo con el nombre %s\n", nombreDeArchivo);

    if(memoria_ram.esquema_memoria == SEGMENTACION) {
        dump_segmentacion(archivo_dump);
    }
    if(memoria_ram.esquema_memoria == PAGINACION) {
        dump_paginacion(archivo_dump);
    }

    fprintf(archivo_dump, "------------------------------------------------------------------------------------\n");
	
	fclose(archivo_dump);
	free(momento);
}

void dump_paginacion(FILE* archivo_dump) {
    loggear_marcos_fisicos(archivo_dump);
}

void dump_segmentacion(FILE* archivo_dump){
	loggear_segmentos(archivo_dump);
}

void loggear_patotas() {
    log_info(logger, "Lista de patotas: %d", list_size(lista_patotas));
	
	uint32_t inicio;
	uint32_t pid;
	uint32_t pnt_tareas;
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		for(int i = 0; i < list_size(lista_patotas); i++) {
			patota_data * mi_patota = (patota_data *)list_get(lista_patotas, i);
			// log_info(logger, "Iteracion %d", i);
			log_info(logger, "Patota %d. PID: %d", i + 1, mi_patota->PID);
			log_info(logger, "Puntero a PCB: %d; Puntero a tareas: %d", mi_patota->inicio_elementos[0],	mi_patota->inicio_elementos[1]);

			inicio = ((patota_data *)(uint32_t)list_get(lista_patotas, i))->inicio_elementos[0];
			memcpy(&pid, memoria_ram.inicio + inicio, sizeof(uint32_t));
			memcpy(&pnt_tareas, memoria_ram.inicio + inicio + sizeof(uint32_t), sizeof(uint32_t));
			log_info(logger, "PID: %d; Puntero a tareas: %d", pid, pnt_tareas);
		}
	}

	if(memoria_ram.esquema_memoria == PAGINACION) {
		for(int i = 0; i < list_size(lista_patotas); i++) {
			patota_data * mi_patota = (patota_data *)list_get(lista_patotas, i);
			log_info(logger, "Patota %d. PID: %d; Puntero a PCB: %d; Puntero a tareas: %d", i + 1,
				mi_patota->PID,	mi_patota->inicio_elementos[0],	mi_patota->inicio_elementos[1]);
			
			// log_info(logger, "Marco: %d. Inicio marco %p", mi_patota->frames[0], inicio_marco(mi_patota->frames[0]));
			log_info(logger, "PID: %d; Puntero a tareas: %d", obtener_entero_paginacion(i + 1, 0), obtener_entero_paginacion(i + 1, 4));
		}
	}
}

void loggear_tripulantes() {
    uint32_t inicio;
	uint32_t pid;
	uint32_t tid;
    log_info(logger, "Lista de tripulantes activos: %d", list_size(lista_tripulantes));
	for(int i = 0; i < lista_tripulantes->elements_count; i++) {
		inicio = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->inicio;
		pid = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->PID;
		tid = ((trip_data *)(uint32_t)list_get(lista_tripulantes, i))->TID;
		log_info(logger, "TID: %d; inicio: %d; estado: %c; pos_x: %d; pos_y: %d; IP: %d; Punt PCB: %d",
			obtener_valor_tripulante(pid, tid, TRIP_IP),
			inicio,
			obtener_estado(pid, tid),
			obtener_valor_tripulante(pid, tid, POS_X),
			obtener_valor_tripulante(pid, tid, POS_Y),
			obtener_valor_tripulante(pid, tid, INS_POINTER),
			obtener_valor_tripulante(pid, tid, PCB_POINTER)
			);
	}
}

void loggear_prueba_segmentos(uint32_t nro_segmento) {
	t_segmento* segmento = (t_segmento *)list_get(memoria_ram.mapa_segmentos, nro_segmento);
	log_info(logger, "SEGMENTO %d/Duenio: %d/Indice: %d/Inicio: %d/Tamanio: %d",
		segmento->n_segmento + 1, segmento->duenio,	segmento->indice, segmento->inicio,	segmento->tamanio);
}

void loggear_segmento(FILE* archivo, uint32_t nro_segmento, uint32_t nro_segmento_relativo) {
	t_segmento* segmento = (t_segmento *)list_get(memoria_ram.mapa_segmentos, nro_segmento);
    fprintf(archivo, "Proceso: %02d    Segmento: %02d    Inicio: 0x%04X    Tam: %db\n", segmento->duenio, nro_segmento_relativo, segmento->inicio, segmento->tamanio);
	fprintf(archivo,"\n");
}

void loggear_segmentos(FILE* archivo) {
	log_info(logger, "Cantidad de segmentos: %d. Memoria libre: %d", list_size(memoria_ram.mapa_segmentos), memoria_libre_segmentacion());
	if(archivo != NULL) {
		for (int id_patota = 0; id_patota < list_size(lista_patotas); id_patota++) {
			t_list* lista_segmentos_patota = seg_ordenados_de_patota(id_patota + 1);
			for(int trip = 0; trip < list_size(lista_segmentos_patota); trip++) {
				t_segmento* seg_trip = list_get(lista_segmentos_patota, trip);
				loggear_segmento(archivo, seg_trip->n_segmento, trip);
			}
		}
	}
	else {
		for (int id_seg = 0; id_seg < list_size(memoria_ram.mapa_segmentos); id_seg++) {
			loggear_prueba_segmentos(id_seg);
		}
	}
}

void loggear_marco(FILE* archivo, uint32_t nro_marco) {
	t_marco* un_marco = memoria_ram.mapa_fisico[nro_marco];
	if(un_marco->duenio) {//El frame esta ocupado
		fprintf(archivo, "Marco: %d    Estado: Ocupado    Proceso: %d    Pagina: %d \n",
			nro_marco, un_marco->duenio, nro_pagina_de_patota(un_marco->duenio, nro_marco));
    }
    else {
        fprintf(archivo, "Marco: %d    Estado: Libre      Proceso:  -    Pagina:  - \n", nro_marco);
    }
	fprintf(archivo,"\n");
}

void loggear_marcos_logicos(FILE* archivo) {
	if(archivo == NULL) {
		log_info(logger, "Marcos libres fisicos: %d. Marcos libres logicos: %d", marcos_reales_disponibles(), marcos_logicos_disponibles());
		for (int i = 0; i < (uint32_t)(memoria_ram.tamanio_swap / TAMANIO_PAGINA); i++) {
			log_info(logger, "ML %d/ MF: %d/ Duenio: %d/ Presencia: %d/ Modificado: %d/ Uso: %d",
				memoria_ram.mapa_logico[i]->nro_virtual,
				memoria_ram.mapa_logico[i]->nro_real,
				memoria_ram.mapa_logico[i]->duenio,
				memoria_ram.mapa_logico[i]->presencia,
				memoria_ram.mapa_logico[i]->modificado,
				memoria_ram.mapa_logico[i]->bit_uso
				);
		}
	}
}

void loggear_marcos_fisicos(FILE* archivo) {
	log_info(logger, "Marcos libres fisicos: %d. Marcos libres logicos: %d", marcos_reales_disponibles(), marcos_logicos_disponibles());
	for (int i = 0; i < (uint32_t)(memoria_ram.tamanio_memoria / TAMANIO_PAGINA); i++) {
		if(archivo == NULL) {
			log_info(logger, "ML %2d/ MF: %2d/ Duenio: %2d/ Presencia: %d/ Modificado: %d/ Uso: %d",
				memoria_ram.mapa_fisico[i]->nro_virtual,
				memoria_ram.mapa_fisico[i]->nro_real,
				memoria_ram.mapa_fisico[i]->duenio,
				memoria_ram.mapa_fisico[i]->presencia,
				memoria_ram.mapa_fisico[i]->modificado,
				memoria_ram.mapa_fisico[i]->bit_uso
				);
		}
		else {
			loggear_marco(archivo, i);
		}
	}
}

void loggear_data() {
	log_info(logger, "NUEVOS RESULTADOS");
	if(memoria_ram.esquema_memoria == SEGMENTACION) {
		loggear_segmentos(NULL);
	}
	if(memoria_ram.esquema_memoria == PAGINACION) {
		loggear_marcos_logicos(NULL);
	}
    loggear_patotas();
    loggear_tripulantes();
	// dump();
}
