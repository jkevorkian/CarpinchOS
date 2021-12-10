#include "tlb.h"

t_entrada_tlb* es_entrada(uint32_t, uint32_t, uint32_t);
t_entrada_tlb* solicitar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina);

void iniciar_tlb(t_config* config) {
	char * algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	if(!strcmp(algoritmo_reemplazo, "FIFO")) {
		tlb.algoritmo_reemplazo = FIFO;
		pthread_mutex_init(&mutex_fifo_tlb, NULL);
		cola_fifo_tlb = list_create();
	}
	if(!strcmp(algoritmo_reemplazo, "LRU"))
		tlb.algoritmo_reemplazo = LRU;
	
	pthread_mutex_init(&asignacion_entradas_tlb, NULL);

	tlb.cant_entradas = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
	tlb.path_dump = config_get_string_value(config, "PATH_DUMP_TLB");
	tlb.cant_hit = 0;
	tlb.cant_miss = 0;
	tlb.puntero_fifo = 0;
	tlb.mapa = calloc(tlb.cant_entradas, sizeof(t_entrada_tlb *)); // Tiene que ser puntero

	historico_hit_miss = list_create();
	pthread_mutex_init(&mutex_historico_hit_miss, NULL);

	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = malloc(sizeof(t_entrada_tlb));
		entrada->nro_entrada = i;
		entrada->id_car = 0;
		entrada->pagina = -1;
		entrada->marco = -1;
		tlb.mapa[i] = entrada;
		pthread_mutex_init(&entrada->mutex, NULL);

		if(tlb.algoritmo_reemplazo == FIFO) {
			list_add(cola_fifo_tlb, entrada);
		}
	}

	log_info(logger,"TLB inicializada. Nro de entradas: %d", tlb.cant_entradas);
}

t_entrada_tlb *leer_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {	
	t_entrada_tlb* entrada = solicitar_entrada_tlb(id_carpincho, nro_pagina);

	pthread_mutex_lock(&mutex_historico_hit_miss);
	t_hit_miss_tlb *historico_carpincho = list_get(historico_hit_miss, id_carpincho - 1);
	pthread_mutex_unlock(&mutex_historico_hit_miss);

 	if(entrada == NULL){
		usleep(tlb.retardo_fallo * 1000);
		historico_carpincho->cant_miss++;
		tlb.cant_miss++;
		log_info(logger, "TLB Miss - Carpincho #%d, Numero de pagina: %d", id_carpincho, nro_pagina);
		return NULL;
	}
	else {
		usleep(tlb.retardo_acierto * 1000);
		historico_carpincho->cant_hit++;
		tlb.cant_hit++;
		if(tlb.algoritmo_reemplazo == LRU) {
			entrada->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
		}
		log_info(logger, "TLB Hit - Carpincho #%d, Numero de pagina: %d, Numero de marco: %d", id_carpincho, nro_pagina, entrada->marco);
		return entrada;
	}
}

t_entrada_tlb* solicitar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_entrada_tlb* entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		if((entrada = es_entrada(i, id_carpincho, nro_pagina)))
			break;
	}
	return entrada;
}

t_entrada_tlb *obtener_entrada_intercambio_tlb(uint32_t id, uint32_t nro_pagina) {
	bool es_mi_entrada(void *una_entrada) {
		t_entrada_tlb *ent = (t_entrada_tlb *)una_entrada;
		pthread_mutex_lock(&ent->mutex);
		bool es_entrada = ent->id_car == id && ent->pagina == nro_pagina;
		pthread_mutex_unlock(&((t_entrada_tlb *)una_entrada)->mutex);
		return es_entrada;
	}

	t_entrada_tlb* entrada = NULL;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		if(es_mi_entrada(tlb.mapa[i])) {
			entrada = tlb.mapa[i];
			break;
		}
	}

	if(entrada == NULL)
		return NULL;

	if(tlb.algoritmo_reemplazo == FIFO) {
		pthread_mutex_lock(&mutex_fifo_tlb);
		list_remove_by_condition(cola_fifo_tlb, es_mi_entrada);
		list_add(cola_fifo_tlb, entrada);
		pthread_mutex_unlock(&mutex_fifo_tlb);
	}
	else {
		entrada->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
	}
	// pthread_mutex_unlock(&entrada_tlb->mutex);
	return entrada;
}

t_entrada_tlb *quitar_entrada_tlb_fifo(uint32_t id, uint32_t nro_pagina) {
	bool es_mi_entrada(void *una_entrada) {
		bool salida;
		pthread_mutex_lock(&((t_entrada_tlb *)una_entrada)->mutex);
		salida = ((t_entrada_tlb *)una_entrada)->id_car == id && ((t_entrada_tlb *)una_entrada)->pagina == nro_pagina;
		pthread_mutex_unlock(&((t_entrada_tlb *)una_entrada)->mutex);
		return salida;
	}

	pthread_mutex_lock(&mutex_fifo_tlb);
	t_entrada_tlb *entrada = list_remove_by_condition(cola_fifo_tlb, (es_mi_entrada));
	list_add(cola_fifo_tlb, entrada);
	pthread_mutex_unlock(&mutex_fifo_tlb);
	return entrada;
}

t_entrada_tlb* asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	bool asigne_entrada = false;
	pthread_mutex_lock(&asignacion_entradas_tlb);
	t_entrada_tlb* entrada;

	for(int i = 0; i < tlb.cant_entradas && !asigne_entrada; i++) {
		entrada = tlb.mapa[i];
		pthread_mutex_lock(&entrada->mutex);
		if(entrada->id_car == 0) {
			asigne_entrada = true;
		}
		pthread_mutex_unlock(&entrada->mutex);
	}

	if(asigne_entrada) {
		pthread_mutex_unlock(&asignacion_entradas_tlb);
		return entrada;
	}

	if(tlb.algoritmo_reemplazo == FIFO) {
		pthread_mutex_lock(&mutex_fifo_tlb);
		entrada = list_remove(cola_fifo_tlb, 0);
		list_add(cola_fifo_tlb, entrada);
		pthread_mutex_unlock(&mutex_fifo_tlb);
	}
	else {
		bool primero_mas_viejo;
		t_entrada_tlb* entrada_siguiente;
		entrada = tlb.mapa[0];

		for(int i = 1; i < tlb.cant_entradas; i++) {
			entrada_siguiente = tlb.mapa[i];
			pthread_mutex_lock(&entrada->mutex);
			pthread_mutex_lock(&entrada_siguiente->mutex);
			primero_mas_viejo = primer_tiempo_mas_chico(entrada->tiempo_lru, entrada_siguiente->tiempo_lru);
			pthread_mutex_unlock(&entrada->mutex);
			pthread_mutex_unlock(&entrada_siguiente->mutex);
			
			if(!primero_mas_viejo) entrada = entrada_siguiente;
		}
	}
	pthread_mutex_unlock(&asignacion_entradas_tlb);
	
	return entrada;
}

void entrada_nueva(uint32_t id_carpincho, uint32_t nro_pagina, t_entrada_tlb* entrada_a_asignar){
	t_entrada_tp* pagina = pagina_de_carpincho(id_carpincho, nro_pagina);

	entrada_a_asignar->id_car = id_carpincho;
	entrada_a_asignar->pagina = nro_pagina;
	entrada_a_asignar->marco = pagina->nro_marco;
	
	bool es_mi_entrada(void *una_entrada) {
		return una_entrada == entrada_a_asignar;
	}

	if(tlb.algoritmo_reemplazo == FIFO) {
		pthread_mutex_lock(&mutex_fifo_tlb);
		list_remove_by_condition(cola_fifo_tlb, es_mi_entrada);
		list_add(cola_fifo_tlb, entrada_a_asignar);
		pthread_mutex_unlock(&mutex_fifo_tlb);
	}
	else {
		entrada_a_asignar->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
	}
}

void borrar_pagina_carpincho_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_entrada_tlb* entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		if((entrada = es_entrada(i, id_carpincho, nro_pagina))) {
			pthread_mutex_lock(&asignacion_entradas_tlb);
			entrada->id_car = 0;
			pthread_mutex_unlock(&asignacion_entradas_tlb);
			break;
		}
	}
}

t_entrada_tlb* es_entrada(uint32_t nro_entrada, uint32_t id_car, uint32_t nro_pagina) {
	t_entrada_tlb* entrada = tlb.mapa[nro_entrada];
	return entrada->id_car == id_car && entrada->pagina == nro_pagina ? entrada : NULL;
}

void print_tlb() {
	// char path_base[] = "/home/utnso/dumps";
	log_info(logger, "Imprimo valores de tlb");
	char* timestamp = temporal_get_string_time("%d/%m/%y %H:%M:%S");
    char* filename = string_from_format("%s/Dump_<%s>.dmp", tlb.path_dump, temporal_get_string_time("%d_%m_%y-%H_%M_%S"));
    FILE* dump_file = fopen(filename, "w");
	if(!dump_file) {
		log_warning(logger, "El archivo no pudo ser creado");
		return;
	}

	fprintf(dump_file, "-------------------------------------------------------------------------- \n");
	fprintf(dump_file, "Dump: %s \n", timestamp);
	
	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = tlb.mapa[i];
		if(entrada->id_car == 0){
			fprintf(dump_file, "Entrada:%d\tEstado:Libre\tCarpincho:%c\tPagina:%c\tMarco:%c\n", i, '-', '-', '-');
		}
		else {
			fprintf(dump_file, "Entrada:%d\tEstado:Ocupado\tCarpincho:%d\tPagina:%d\tMarco:%d\n", i, entrada->id_car, entrada->pagina, entrada->marco);
		}
	}

	fprintf(dump_file, "-------------------------------------------------------------------------- \n");

	fclose(dump_file);
	free(timestamp);
	free(filename);
}

void resetear_tlb() {
	tlb.cant_hit = 0;
	tlb.cant_miss = 0;
	list_clean(tlb.hit_miss_proceso);
	for(int i = 0; i < tlb.cant_entradas; i++) {
		// borrar_entrada_tlb(i);
	}
	log_info(logger, "TLB reseteada.");
}

void print_hit_miss(){
	void mostrar_hit(void *historico_carpincho) {
		log_info(logger, "Cantidad de TLB Hit carpincho %d: %d",
			((t_hit_miss_tlb *)historico_carpincho)->id_carpincho, ((t_hit_miss_tlb *)historico_carpincho)->cant_hit);
	}
	
	void mostrar_miss(void *historico_carpincho) {
		log_info(logger, "Cantidad de TLB Hit carpincho %d: %d",
			((t_hit_miss_tlb *)historico_carpincho)->id_carpincho, ((t_hit_miss_tlb *)historico_carpincho)->cant_miss);
	}

	log_info(logger, "Cantidad de TLB Hit totales: %d", tlb.cant_hit);
	
	pthread_mutex_lock(&mutex_historico_hit_miss);
	
	list_iterate(historico_hit_miss, mostrar_hit);
	
	log_info(logger, "Cantidad de TLB Miss totales: %d", tlb.cant_miss);
	
	list_iterate(historico_hit_miss, mostrar_miss);
	
	pthread_mutex_unlock(&mutex_historico_hit_miss);
}

void obtener_control_tlb() {
	// t_carpincho *aux;
	// for(int i = 0; i < list_size(lista_carpinchos); i++) {
	// 	aux = list_get(lista_carpinchos, i);
	// 	sem_wait(aux->sem_tlb);
	// }
}

void liberar_control_tlb() {
	// t_carpincho *aux;
	// for(int i = 0; i < list_size(lista_carpinchos); i++) {
	// 	aux = list_get(lista_carpinchos, i);
	// 	sem_post(aux->sem_tlb);
	// }
}