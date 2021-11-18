#include "tlb.h"

void iniciar_tlb(){
	char * algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	if(!strcmp(algoritmo_reemplazo, "FIFO"))
		tlb.algoritmo_reemplazo = FIFO;
	if(!strcmp(algoritmo_reemplazo, "LRU"))
		tlb.algoritmo_reemplazo = LRU;

	tlb.cant_entradas = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
	tlb.cant_hit = 0;
	tlb.cant_miss = 0;
	tlb.mapa = calloc(tlb.cant_entradas, sizeof(t_entrada_tlb));
	tlb.hit_miss_proceso = list_create();

	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = malloc(sizeof(t_entrada_tlb));
		entrada->id_car = 0;
		entrada->pagina = 0;
		entrada->marco = 0;
		tlb.mapa[i] = entrada;
	} 

	log_info(logger,"TLB inicializada. Nro de entradas: %d", tlb.cant_entradas);
	// TEST
 	//crear_carpincho(1);
 	//crear_carpincho(2);
	//asignar_entrada_tlb(1, 1);
	//asignar_entrada_tlb(1, 2);
	//asignar_entrada_tlb(2, 1);
	//leer_tlb(1, 1);
	//leer_tlb(1, 3);
	//print_hit_miss();
	//print_tlb(); 

} 

uint32_t leer_tlb(uint32_t id_carpincho, uint32_t nro_pagina){
	t_entrada_tlb* entrada = solicitar_entrada_tlb(id_carpincho, nro_pagina);
	t_tlb_por_proceso* hit_miss = get_hit_miss_proceso(id_carpincho);

 	if(entrada == NULL){
		hit_miss->cant_miss = hit_miss->cant_miss + 1;
		tlb.cant_miss = tlb.cant_miss + 1;
		log_info(logger, "TLB Miss - Carpincho #%d, Número de página: %d", id_carpincho, nro_pagina);
		// TODO: reemplazar nueva entrada
		return 0;
	}
	else {
		hit_miss->cant_hit = hit_miss->cant_hit + 1;
		tlb.cant_hit = tlb.cant_hit + 1;
		log_info(logger, "TLB Hit - Carpincho #%d, Número de página: %d, Número de marco: %d", id_carpincho, nro_pagina, entrada->marco);
		return entrada->marco;
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

void asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	obtener_control_tlb();
	t_entrada_tlb* entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		entrada = tlb.mapa[i];
		if(entrada->id_car == 0){
			entrada->id_car = id_carpincho;
			entrada->pagina = nro_pagina;
			t_carpincho* carpincho = carpincho_de_lista(id_carpincho);
			t_entrada_tp* pagina = (t_entrada_tp*) list_get(carpincho->tabla_paginas, nro_pagina);
			entrada->marco = pagina->nro_marco;
			break;
		};
	} 
	//TODO: algoritmo de reemplazo
	liberar_control_tlb();
}

void borrar_entrada_tlb(uint32_t nro_entrada) {
	obtener_control_tlb();
	t_entrada_tlb* entrada = tlb.mapa[nro_entrada];
	entrada->id_car = 0;
	liberar_control_tlb();
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
	if(entrada->id_car == id_car && entrada->pagina == nro_pagina)
		return entrada;
	else
		return NULL;
}

void print_tlb() {
	char* timestamp = temporal_get_string_time("%d/%m/%y %H:%M:%S");
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


