#include "tlb.h"

t_entrada_tlb* es_entrada(uint32_t, uint32_t, uint32_t);
t_entrada_tlb* solicitar_entrada_tlb(t_entrada_tp *entrada_tp);

void iniciar_tlb(t_config* config) {
	char * algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	if(!strcmp(algoritmo_reemplazo, "FIFO")) {
		log_info(logger, "Algoritmo de asignacion de entradas TLB: FIFO");
		tlb.algoritmo_reemplazo = FIFO;
		pthread_mutex_init(&mutex_fifo_tlb, NULL);
		cola_fifo_tlb = list_create();
	}
	else {
		log_info(logger, "Algoritmo de asignacion de entradas TLB: LRU");
		tlb.algoritmo_reemplazo = LRU;
	}
	
	pthread_mutex_init(&mutex_asignacion_tlb, NULL);

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
		entrada->tiempo_lru = NULL;
		entrada->id = 0;
		entrada->pagina = -1;
		entrada->marco = -1;
		tlb.mapa[i] = entrada;
		pthread_mutex_init(&entrada->mutex, NULL);

		if(tlb.algoritmo_reemplazo == FIFO) {
			list_add(cola_fifo_tlb, entrada);
		}
	}

    mkdir(tlb.path_dump, 0755);

	log_info(logger,"TLB inicializada. Nro de entradas: %d", tlb.cant_entradas);
}

t_entrada_tlb *leer_tlb(t_entrada_tp *entrada_tp) {	
	t_entrada_tlb* entrada_tlb = solicitar_entrada_tlb(entrada_tp);

	pthread_mutex_lock(&mutex_historico_hit_miss);
	t_hit_miss_tlb *historico_carpincho = list_get(historico_hit_miss, entrada_tp->id - 1);
	pthread_mutex_unlock(&mutex_historico_hit_miss);

 	if(entrada_tlb == NULL) {
		usleep(tlb.retardo_fallo * 1000);
		historico_carpincho->cant_miss++;
		tlb.cant_miss++;
		log_info(logger, "TLB Miss - Carpincho #%d, Numero de pagina: %d", entrada_tp->id, entrada_tp->pagina);
		return NULL;
	}
	else {
		usleep(tlb.retardo_acierto * 1000);
		historico_carpincho->cant_hit++;
		tlb.cant_hit++;
		if(tlb.algoritmo_reemplazo == LRU) {

			if(entrada_tlb->tiempo_lru)
				free(entrada_tlb->tiempo_lru);

			entrada_tlb->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
		}
		log_info(logger, "TLB Hit - Carpincho #%d, Numero de pagina: %d, Numero de marco: %d", entrada_tp->id, entrada_tp->pagina, entrada_tp->marco);
		return entrada_tlb;
	}
}

t_entrada_tlb* solicitar_entrada_tlb(t_entrada_tp *entrada_tp) {
	t_entrada_tlb* entrada_tlb;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		entrada_tlb = tlb.mapa[i];
		pthread_mutex_lock(&entrada_tlb->mutex);
		if(entrada_tlb->id == entrada_tp->id && entrada_tlb->pagina == entrada_tp->pagina)
			return entrada_tlb;
		pthread_mutex_unlock(&entrada_tlb->mutex);
	}
	return NULL;
}

t_entrada_tlb *reemplazar_entrada_tlb(t_entrada_tp *entrada_vieja_tp, t_entrada_tp *entrada_nueva_tp) {	
	bool es_mi_entrada(void *una_entrada) {
		t_entrada_tlb *ent = (t_entrada_tlb *)una_entrada;
		pthread_mutex_lock(&ent->mutex);
		bool es_entrada = ent->id == entrada_vieja_tp->id && ent->pagina == entrada_vieja_tp->pagina;
		if(!es_entrada)
			pthread_mutex_unlock(&((t_entrada_tlb *)una_entrada)->mutex);
		return es_entrada;
	}

	t_entrada_tlb* entrada_tlb = NULL;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		if(es_mi_entrada(tlb.mapa[i])) {
			entrada_tlb = tlb.mapa[i];
			break;
		}
	}

	if(entrada_tlb == NULL)
		return NULL;

	bool es_mi_entrada_sin_bloqueo(void *una_entrada) {
		return una_entrada == entrada_tlb;
	}

	if(tlb.algoritmo_reemplazo == FIFO) {
		pthread_mutex_lock(&mutex_fifo_tlb);
		list_remove_by_condition(cola_fifo_tlb, es_mi_entrada_sin_bloqueo);
		list_add(cola_fifo_tlb, entrada_tlb);
		pthread_mutex_unlock(&mutex_fifo_tlb);
	}
	else {
		if(entrada_tlb->tiempo_lru)
			free(entrada_tlb->tiempo_lru);

		entrada_tlb->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
	}
	// pthread_mutex_unlock(&entrada_tlb->mutex);
	log_info(logger, "Realizo reemplazo de entrada de tlb con entrada de pagina saliente");
	log_info(logger, "Entrada victima nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
		entrada_tlb->nro_entrada, entrada_tlb->id, entrada_tlb->pagina, entrada_tlb->marco);
	
	entrada_tlb->id = entrada_nueva_tp->id;
	entrada_tlb->pagina = entrada_nueva_tp->pagina;
	entrada_tlb->marco = entrada_nueva_tp->marco;

	// loggear_tlb();
	
	return entrada_tlb;
}

t_entrada_tlb *quitar_entrada_tlb_fifo(uint32_t id, uint32_t nro_pagina) {
	bool es_mi_entrada(void *una_entrada) {
		bool salida;
		pthread_mutex_lock(&((t_entrada_tlb *)una_entrada)->mutex);
		salida = ((t_entrada_tlb *)una_entrada)->id == id && ((t_entrada_tlb *)una_entrada)->pagina == nro_pagina;
		pthread_mutex_unlock(&((t_entrada_tlb *)una_entrada)->mutex);
		return salida;
	}

	pthread_mutex_lock(&mutex_fifo_tlb);
	t_entrada_tlb *entrada = list_remove_by_condition(cola_fifo_tlb, (es_mi_entrada));
	list_add(cola_fifo_tlb, entrada);
	pthread_mutex_unlock(&mutex_fifo_tlb);
	return entrada;
}

t_entrada_tlb* asignar_entrada_tlb(t_entrada_tp *entrada_tp) {
	bool asigne_entrada = false;
	pthread_mutex_lock(&mutex_asignacion_tlb);
	t_entrada_tlb* entrada_tlb;

	for(int i = 0; i < tlb.cant_entradas && !asigne_entrada; i++) {
		entrada_tlb = tlb.mapa[i];
		pthread_mutex_lock(&entrada_tlb->mutex);
		if(entrada_tlb->id == 0) {
			asigne_entrada = true;
		}
		pthread_mutex_unlock(&entrada_tlb->mutex);
	}

	if(asigne_entrada) {
		entrada_nueva(entrada_tlb, entrada_tp);
		pthread_mutex_unlock(&mutex_asignacion_tlb);
		
		log_info(logger, "Asigno entrada tlb nro %d, que estaba libre", entrada_tlb->nro_entrada);
		return entrada_tlb;
	}

	if(tlb.algoritmo_reemplazo == FIFO) {
		pthread_mutex_lock(&mutex_fifo_tlb);
		entrada_tlb = list_remove(cola_fifo_tlb, 0);
		list_add(cola_fifo_tlb, entrada_tlb);
		pthread_mutex_unlock(&mutex_fifo_tlb);
		pthread_mutex_lock(&entrada_tlb->mutex);
	}
	else {
		bool primero_mas_viejo;
		t_entrada_tlb* entrada_siguiente;
		entrada_tlb = tlb.mapa[0];

		obtener_control_tlb();
		for(int i = 1; i < tlb.cant_entradas; i++) {
			entrada_siguiente = tlb.mapa[i];
			primero_mas_viejo = primer_tiempo_mas_chico(entrada_tlb->tiempo_lru, entrada_siguiente->tiempo_lru);
			
			if(!primero_mas_viejo) entrada_tlb = entrada_siguiente;
		}
		liberar_control_tlb();
		pthread_mutex_lock(&entrada_tlb->mutex);
	}
	pthread_mutex_unlock(&entrada_tp->mutex);
	pthread_mutex_unlock(&mutex_asignacion_tlb);

	log_info(logger, "Realizo reemplazo de entrada de tlb");
	log_info(logger, "Entrada victima nro %d. Id: %d. Nro_pagina: %d. Nro_marco: %d",
		entrada_tlb->nro_entrada, entrada_tlb->id, entrada_tlb->pagina, entrada_tlb->marco);
	
	entrada_tlb->id = entrada_tp->id;
	entrada_tlb->pagina = entrada_tp->pagina;
	entrada_tlb->marco = entrada_tp->marco;

	// loggear_tlb();
	
	return entrada_tlb;
}

void entrada_nueva(t_entrada_tlb* entrada_tlb, t_entrada_tp *entrada_tp){
	entrada_tlb->id = entrada_tp->id;
	entrada_tlb->pagina = entrada_tp->pagina;
	entrada_tlb->marco = entrada_tp->marco;
	
	bool es_mi_entrada(void *una_entrada) {
		return una_entrada == entrada_tlb;
	}

	if(tlb.algoritmo_reemplazo == FIFO) {
		pthread_mutex_lock(&mutex_fifo_tlb);
		list_remove_by_condition(cola_fifo_tlb, es_mi_entrada);
		list_add(cola_fifo_tlb, entrada_tlb);
		pthread_mutex_unlock(&mutex_fifo_tlb);
	}
	else {
		if(entrada_tlb->tiempo_lru)
			free(entrada_tlb->tiempo_lru);

		entrada_tlb->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
	}
}

void borrar_pagina_carpincho_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_entrada_tlb* entrada;
	bool no_encontre = true;

	pthread_mutex_lock(&mutex_asignacion_tlb);
	for(int i = 0; i < tlb.cant_entradas && no_encontre; i++) {
		entrada = tlb.mapa[i];
		pthread_mutex_lock(&entrada->mutex);
		if(entrada->id == id_carpincho && entrada->pagina == nro_pagina) {
			entrada->id = 0;
			no_encontre = false;
		}
		pthread_mutex_unlock(&entrada->mutex);
	}
	pthread_mutex_unlock(&mutex_asignacion_tlb);
}

t_entrada_tlb* es_entrada(uint32_t nro_entrada, uint32_t id_car, uint32_t nro_pagina) {
	t_entrada_tlb* entrada = tlb.mapa[nro_entrada];
	return entrada->id == id_car && entrada->pagina == nro_pagina ? entrada : NULL;
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
		if(entrada->id == 0){
			fprintf(dump_file, "Entrada:%d\tEstado:Libre\tCarpincho:%c\tPagina:%c\tMarco:%c\n", i, '-', '-', '-');
		}
		else {
			fprintf(dump_file, "Entrada:%d\tEstado:Ocupado\tCarpincho:%d\tPagina:%d\tMarco:%d\n", i, entrada->id, entrada->pagina, entrada->marco);
		}
	}

	fprintf(dump_file, "-------------------------------------------------------------------------- \n");

	fclose(dump_file);
	free(timestamp);
	free(filename);
}

void resetear_tlb() {
	pthread_mutex_lock(&mutex_asignacion_tlb);
	obtener_control_tlb();
	for(int i = 0; i < tlb.cant_entradas; i++) {
		tlb.mapa[i]->id = 0;
	}
	liberar_control_tlb();
	pthread_mutex_unlock(&mutex_asignacion_tlb);
	log_info(logger, "TLB reseteada.");
}

void print_hit_miss(){
	void mostrar_hit(void *historico_carpincho) {
		log_info(logger, "Cantidad de TLB Hit carpincho %d: %d",
			((t_hit_miss_tlb *)historico_carpincho)->id_carpincho, ((t_hit_miss_tlb *)historico_carpincho)->cant_hit);
	}
	
	void mostrar_miss(void *historico_carpincho) {
		log_info(logger, "Cantidad de TLB Miss carpincho %d: %d",
			((t_hit_miss_tlb *)historico_carpincho)->id_carpincho, ((t_hit_miss_tlb *)historico_carpincho)->cant_miss);
	}

	log_info(logger, "Cantidad de TLB Hit totales: %d", tlb.cant_hit);
	
	pthread_mutex_lock(&mutex_historico_hit_miss);
	
	log_info(logger, "Cantidad de carpinchos: %d", list_size(historico_hit_miss));
	
	list_iterate(historico_hit_miss, mostrar_hit);
	
	log_info(logger, "Cantidad de TLB Miss totales: %d", tlb.cant_miss);
	
	list_iterate(historico_hit_miss, mostrar_miss);
	
	pthread_mutex_unlock(&mutex_historico_hit_miss);
}

void obtener_control_tlb() {
	t_entrada_tlb *entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		entrada = tlb.mapa[i];
		pthread_mutex_lock(&entrada->mutex);
	}
}

void liberar_control_tlb() {
	t_entrada_tlb *entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		entrada = tlb.mapa[i];
		pthread_mutex_unlock(&entrada->mutex);
	}
}

void loggear_tlb() {
	log_info(logger, "Imprimo valores de tlb");
	// pthread_mutex_lock(&mutex_asignacion_tlb);
	obtener_control_tlb();
	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = tlb.mapa[i];
		if(entrada->id == 0){
			log_info(logger, "Entrada:%d\tEstado:Libre\tCarpincho:%c\tPagina:%c\tMarco:%c\n", i, '-', '-', '-');
		}
		else {
			log_info(logger, "Entrada:%d\tEstado:Ocupado\tCarpincho:%d\tPagina:%d\tMarco:%d\n", i, entrada->id, entrada->pagina, entrada->marco);
		}
	}
	liberar_control_tlb();
	// pthread_mutex_unlock(&mutex_asignacion_tlb);
}
