#include "tlb.h"
// TODO: agregar path al nombre de dump

void iniciar_tlb(){
	char * algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	if(!strcmp(algoritmo_reemplazo, "FIFO"))
		tlb.algoritmo_reemplazo = FIFO;
	if(!strcmp(algoritmo_reemplazo, "LRU"))
		tlb.algoritmo_reemplazo = LRU;

	tlb.cant_entradas = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
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
		entrada = asignar_entrada_tlb(id_carpincho, nro_pagina);
		log_info(logger, "TLB Miss - Carpincho #%d, Número de página: %d", id_carpincho, nro_pagina);
	}
	else {
		usleep(tlb.retardo_acierto * 1000);
		hit_miss->cant_hit += 1;
		tlb.cant_hit += 1;
		entrada->tiempo_lru = time(0);
		log_info(logger, "TLB Hit - Carpincho #%d, Número de página: %d, Número de marco: %d", id_carpincho, nro_pagina, entrada->marco);		
	}

	return entrada->marco;
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
	obtener_control_tlb();
	t_entrada_tlb* entrada;

	for(int i = 0; i < tlb.cant_entradas; i++) {
		entrada = tlb.mapa[i];
		if(entrada->id_car == 0){
			entrada_nueva(id_carpincho, nro_pagina, entrada);
			liberar_control_tlb();
			return entrada;
		};
	} 

	if(tlb.algoritmo_reemplazo == FIFO){
		entrada = tlb.mapa[tlb.puntero_fifo]; 
		entrada_nueva(id_carpincho, nro_pagina, entrada);

		if(tlb.puntero_fifo + 1 == tlb.cant_entradas) tlb.puntero_fifo = 0;
		else tlb.puntero_fifo = tlb.puntero_fifo + 1;
	}
	else if(tlb.algoritmo_reemplazo == LRU){
		// DUDA: tipo double puede traer problemas? si lo casteo a uint32_t no funciona result < 0
		double result;
		t_entrada_tlb* entrada_menor = tlb.mapa[0];

		for(int i = 0; i < tlb.cant_entradas; i++) {
			entrada = tlb.mapa[i];
			if(entrada->tiempo_lru){
				result = difftime(entrada->tiempo_lru, entrada_menor->tiempo_lru);
				if(result < 0) entrada_menor = entrada;
			};
		} 

		entrada_nueva(id_carpincho, nro_pagina, entrada_menor);
	}

	liberar_control_tlb();
	return entrada;
}

void entrada_nueva(uint32_t id_carpincho, uint32_t nro_pagina, t_entrada_tlb* entrada){
	t_carpincho* carpincho = carpincho_de_lista(id_carpincho);
	t_entrada_tp* pagina = (t_entrada_tp*) list_get(carpincho->tabla_paginas, nro_pagina);

	entrada->id_car = id_carpincho;
	entrada->pagina = nro_pagina;
	entrada->marco = pagina->nro_marco;
	entrada->tiempo_lru = time(0);
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
	char* timestamp = temporal_get_string_time("%d/%m/%y %H:%M:%S:%MS");
    char* filename = string_from_format("Dump_<%s>.dmp", temporal_get_string_time("%d_%m_%y-%H_%M_%S"));
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

void resetear_entradas_proceso(uint32_t id_carpincho) {
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
	for(int i = 0; i < list_size(lista_carpinchos) - 1; i++) {
		aux = list_get(lista_carpinchos, i);
		sem_wait(aux->sem_tlb);
	}
}

void liberar_control_tlb() {
	t_carpincho *aux;
	for(int i = 0; i < list_size(lista_carpinchos) - 1; i++) {
		aux = list_get(lista_carpinchos, i);
		sem_post(aux->sem_tlb);
	}
}

void print_tiempo(t_entrada_tlb* entrada){
	struct tm *ts;
	char buf[80];
	ts = localtime(&entrada->tiempo_lru);
	strftime(buf, sizeof(buf), "%H:%M:%S", ts);
	printf("Pagina: %d Tiempo: %s\n", entrada->pagina, buf);
}