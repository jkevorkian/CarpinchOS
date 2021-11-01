#include "tlb.h"

void iniciar_tlb(t_config* config){
	char * algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	if(!strcmp(algoritmo_reemplazo, "FIFO"))
		tlb.algoritmo_reemplazo = FIFO;
	if(!strcmp(algoritmo_reemplazo, "LRU"))
		tlb.algoritmo_reemplazo = LRU;

	tlb.cant_entradas = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
	tlb.mapa = calloc(tlb.cant_entradas, sizeof(t_entrada_tlb));

	printf("TLB inicializada. Nro de entradas: %d \n", tlb.cant_entradas);
}

t_entrada_tlb *solicitar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	t_entrada_tlb *entrada;
	for(int i = 0; i < tlb.cant_entradas; i++) {
		if((entrada = es_entrada(i, id_carpincho, nro_pagina)))
			break;
	}
	return entrada;
}

void asignar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	obtener_control_tlb();
	// asignar_entrada(id_carpincho, nro_pagina);
	liberar_control_tlb();
}

void borrar_entrada_tlb(uint32_t nro_entrada) {
	obtener_control_tlb();
	// borrar_entrada(nro_entrada);
	liberar_control_tlb();
}

void obtener_control_tlb() {
	t_carpincho *aux;
	for(int i = 0; i < cant_carpinchos; i++) {
		aux = list_get(lista_carpinchos, i);
		sem_wait(aux->sem_tlb);
	}
}

void liberar_control_tlb() {
	t_carpincho *aux;
	for(int i = 0; i < cant_carpinchos; i++) {
		aux = list_get(lista_carpinchos, i);
		sem_post(aux->sem_tlb);
	}
}

t_entrada_tlb *es_entrada(uint32_t nro_entrada, uint32_t id_car, uint32_t nro_pagina) {
	t_entrada_tlb *entrada = tlb.mapa[nro_entrada];
	if(entrada->id_car == id_car && entrada->pagina == nro_pagina)
		return entrada;
	else
		return NULL;
}
