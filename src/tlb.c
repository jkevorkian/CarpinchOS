#include "tlb.h"

void iniciar_tlb(t_config* config){
	char * algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	if(!strcmp(algoritmo_reemplazo, "FIFO")) {
		tlb.algoritmo_reemplazo = FIFO;
		tlb.puntero_fifo = 0;
	}
	if(!strcmp(algoritmo_reemplazo, "LRU"))
		tlb.algoritmo_reemplazo = LRU;

	tlb.cant_entradas = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
	tlb.path_dump = config_get_string_value(config, "PATH_DUMP_TLB");
	tlb.cant_hit = 0;
	tlb.cant_miss = 0;
	tlb.puntero_fifo = 0;
	tlb.mapa = calloc(tlb.cant_entradas, sizeof(t_entrada_tlb));
	tlb.hit_miss_proceso = list_create();

	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = malloc(sizeof(t_entrada_tlb));
		entrada->id_car = 0;
		entrada->pagina = -1;
		entrada->marco = -1;
		tlb.mapa[i] = entrada;
	} 

	log_info(logger,"TLB inicializada. Nro de entradas: %d", tlb.cant_entradas);
} 

uint32_t leer_tlb(uint32_t id_carpincho, uint32_t nro_pagina){
	t_entrada_tlb* entrada = solicitar_entrada_tlb(id_carpincho, nro_pagina);
	t_tlb_por_proceso* hit_miss = get_hit_miss_proceso(id_carpincho);

 	if(entrada == NULL){
		usleep(tlb.retardo_fallo * 1000);
		hit_miss->cant_miss += 1;
		tlb.cant_miss += 1;
		log_info(logger, "TLB Miss - Carpincho #%d, Número de página: %d", id_carpincho, nro_pagina);
		return -1;
	}
	else {
		usleep(tlb.retardo_acierto * 1000);
		hit_miss->cant_hit += 1;
		tlb.cant_hit += 1;
		entrada->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
		log_info(logger, "TLB Hit - Carpincho #%d, Número de página: %d, Número de marco: %d", id_carpincho, nro_pagina, entrada->marco);		
		return entrada->marco;
	}
}

t_entrada_tlb *leer_tlb2(uint32_t id_carpincho, uint32_t nro_pagina){

	t_entrada_tlb* solicitar_entrada_tlb2(uint32_t id_carpincho, uint32_t nro_pagina) {
		t_entrada_tlb* entrada;
		for(int i = 0; i < tlb.cant_entradas; i++) {
			if((entrada = es_entrada(i, id_carpincho, nro_pagina)))
				break;
		}
		return entrada;
	}
	
	t_entrada_tlb* entrada = solicitar_entrada_tlb2(id_carpincho, nro_pagina);
	t_tlb_por_proceso* hit_miss = get_hit_miss_proceso(id_carpincho);

 	if(entrada == NULL){
		usleep(tlb.retardo_fallo * 1000);
		hit_miss->cant_miss += 1;
		tlb.cant_miss += 1;
		log_info(logger, "TLB Miss - Carpincho #%d, Número de página: %d", id_carpincho, nro_pagina);
		return NULL;
	}
	else {
		usleep(tlb.retardo_acierto * 1000);
		hit_miss->cant_hit += 1;
		tlb.cant_hit += 1;
		entrada->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
		log_info(logger, "TLB Hit - Carpincho #%d, Número de página: %d, Número de marco: %d", id_carpincho, nro_pagina, entrada->marco);
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

t_entrada_tlb* asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	// mutex de asignación tlb
	//t_entrada_tlb* entrada = obtener_entrada_libre()
	// mutex de asignación tlb
	// if(entrada)
		//	return  entrada;
	obtener_control_tlb();
	t_entrada_tlb* entrada;

	for(int i = 0; i < tlb.cant_entradas; i++) {
		entrada = tlb.mapa[i];
		if(entrada->id_car == 0){
			entrada_nueva(id_carpincho, nro_pagina, entrada);
			liberar_control_tlb();
			return entrada;
		}
	}

	if(tlb.algoritmo_reemplazo == FIFO){
		entrada = tlb.mapa[tlb.puntero_fifo]; 
		entrada_nueva(id_carpincho, nro_pagina, entrada);

		if(tlb.puntero_fifo + 1 == tlb.cant_entradas) tlb.puntero_fifo = 0;
		else tlb.puntero_fifo = tlb.puntero_fifo + 1;
	}
	else if(tlb.algoritmo_reemplazo == LRU){
		bool result;
		t_entrada_tlb* entrada_menor = tlb.mapa[0];

		for(int i = 0; i < tlb.cant_entradas; i++) {
			entrada = tlb.mapa[i];
			if(entrada->tiempo_lru){
				result = es_mas_vieja(entrada, entrada_menor);
				if(result) entrada_menor = entrada;
				//printf("menor: %d", entrada_menor->pagina);
			}
		}

		entrada_nueva(id_carpincho, nro_pagina, entrada_menor);
	}

	liberar_control_tlb();
	return entrada;
}

/////////////////////////////////////////////////////////////////////////////////////
// Pato
void actualizar_entrada_tlb(t_entrada_tlb* entrada_tlb, uint32_t id_carpincho, uint32_t nro_pagina) {
	// t_entrada_tlb* entrada_tlb = solicitar_entrada_tlb(id_viejo, nro_pagina/*_vieja*/);
	// pthread_mutex_lock(&entrada_tlb->mutex);
	// entrada_vieja->tiempo_lru = ...;
	// entrada_vieja_tlb->id_car = id_carpincho;
	// entrada_vieja_tlb->pagina = nro_pagina;
	// entrada_vieja->marco = marco->nro_real;		// Debería ser igual
	// pthread_mutex_lock(&entrada_tlb->mutex);
}
/////////////////////////////////////////////////////////////////////////////////////

void entrada_nueva(uint32_t id_carpincho, uint32_t nro_pagina, t_entrada_tlb* entrada){
	// t_carpincho* carpincho = carpincho_de_lista(id_carpincho);
	// t_entrada_tp* pagina = (t_entrada_tp*) list_get(carpincho->tabla_paginas, nro_pagina);
	t_entrada_tp* pagina = pagina_de_carpincho(id_carpincho, nro_pagina);

	entrada->id_car = id_carpincho;
	entrada->pagina = nro_pagina;
	entrada->marco = pagina->nro_marco;
	entrada->tiempo_lru = temporal_get_string_time("%H:%M:%S:%MS");
}

void borrar_pagina_carpincho_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_entrada_tlb* entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		if((entrada = es_entrada(i, id_carpincho, nro_pagina))) {
			borrar_entrada_tlb(i);
			return;
		}
	}
}

void borrar_entrada_tlb(uint32_t nro_entrada) {
	obtener_control_tlb();
	t_entrada_tlb* entrada = tlb.mapa[nro_entrada];
	entrada->id_car = 0;
	liberar_control_tlb();
}

t_tlb_por_proceso* get_hit_miss_proceso(uint32_t id_carpincho){
	bool encontrar_carpincho(void* item){
		t_tlb_por_proceso* entrada = (t_tlb_por_proceso*) item;
		return entrada->id_proceso == id_carpincho;
	}

	t_tlb_por_proceso* entrada = (t_tlb_por_proceso*) list_find(tlb.hit_miss_proceso, encontrar_carpincho);
	
	if(entrada == NULL){
		t_tlb_por_proceso* hit_miss = malloc(sizeof(t_tlb_por_proceso));
		hit_miss->id_proceso = id_carpincho;
		hit_miss->cant_hit = 0;
		hit_miss->cant_miss = 0;
		list_add(tlb.hit_miss_proceso, hit_miss);
		return hit_miss;
	}
	
	return entrada;
}

t_entrada_tlb* es_entrada(uint32_t nro_entrada, uint32_t id_car, uint32_t nro_pagina) {
	t_entrada_tlb* entrada = tlb.mapa[nro_entrada];
	return entrada->id_car == id_car && entrada->pagina == nro_pagina ? entrada : NULL;
}

void print_tlb() {
	char* timestamp = temporal_get_string_time("%d/%m/%y %H:%M:%S");
    char* filename = string_from_format("%s/Dump_<%s>.dmp", tlb.path_dump, temporal_get_string_time("%d_%m_%y-%H_%M_%S"));
    FILE* dump_file = fopen(filename, "w");

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

void flush_proceso_tlb(uint32_t id_carpincho) {
	bool encontrar_carpincho(void* item){
		t_tlb_por_proceso* entrada = (t_tlb_por_proceso*) item;
		return entrada->id_proceso == id_carpincho;
	}

	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = tlb.mapa[i];
		if(entrada->id_car == id_carpincho){
			borrar_entrada_tlb(i);
		}
	}

	list_remove_by_condition(tlb.hit_miss_proceso, encontrar_carpincho);
}

void resetear_tlb() {
	tlb.cant_hit = 0;
	tlb.cant_miss = 0;
	list_clean(tlb.hit_miss_proceso);
	for(int i = 0; i < tlb.cant_entradas; i++) {
		borrar_entrada_tlb(i);
	}
	log_info(logger, "TLB reseteada.");
}

void print_hit_miss(){
	log_info(logger, "Cantidad de TLB Hit totales: %d", tlb.cant_hit);
	list_iterate(tlb.hit_miss_proceso, cant_hit_carpincho);
	log_info(logger, "Cantidad de TLB Miss totales: %d", tlb.cant_miss);
	list_iterate(tlb.hit_miss_proceso, cant_miss_carpincho);
}

void cant_hit_carpincho(void* item){
	t_tlb_por_proceso* entrada = (t_tlb_por_proceso*) item;
	log_info(logger, "Cantidad de TLB Hit de carpincho #%d: %d", entrada->id_proceso, entrada->cant_hit);
}

void cant_miss_carpincho(void* item){
	t_tlb_por_proceso* entrada = (t_tlb_por_proceso*) item;
	log_info(logger, "Cantidad de TLB Miss de carpincho #%d: %d", entrada->id_proceso, entrada->cant_miss);
}

void obtener_control_tlb() {
	t_carpincho *aux;
	for(int i = 0; i < list_size(lista_carpinchos); i++) {
		aux = list_get(lista_carpinchos, i);
		sem_wait(aux->sem_tlb);
	}
}

void liberar_control_tlb() {
	t_carpincho *aux;
	for(int i = 0; i < list_size(lista_carpinchos); i++) {
		aux = list_get(lista_carpinchos, i);
		sem_post(aux->sem_tlb);
	}
}

// para time_t
/* void print_tiempo(t_entrada_tlb* entrada){
	struct tm *ts;
	char buf[80];
	ts = localtime(&entrada->tiempo_lru);
	strftime(buf, sizeof(buf), "%H:%M:%S", ts);
	printf("Pagina: %d Tiempo: %s\n", entrada->pagina, buf);
} */

bool es_mas_vieja(t_entrada_tlb* entrada1, t_entrada_tlb* entrada2) {
	return tiempo_a_milisegundos(entrada1) < tiempo_a_milisegundos(entrada2);
}

uint32_t tiempo_a_milisegundos(t_entrada_tlb* entrada) {
	return obtener_tiempo_lru('H', entrada) * 3600000 + obtener_tiempo_lru('M', entrada) * 60000 + obtener_tiempo_lru('S', entrada) * 1000 + obtener_tiempo_lru('m', entrada);
}

uint32_t obtener_tiempo_lru(char tipo, t_entrada_tlb* entrada){
	// Formato temporal para LRU de tlb: HH:MM:SS:mmm
	char tiempo[2];
	char tiempo_ms[3] = {0,0,0};
	switch(tipo) {
	case 'H':
		memcpy(tiempo, entrada->tiempo_lru, 2);
		break;
	case 'M':
		memcpy(tiempo, entrada->tiempo_lru + 3, 2);
		break;
	case 'S':
		memcpy(tiempo, entrada->tiempo_lru + 6, 2);
		break;
	case 'm':
		memcpy(tiempo_ms, entrada->tiempo_lru + 9, 3);
		return atoi(tiempo_ms);
	}

	return atoi(tiempo);
}