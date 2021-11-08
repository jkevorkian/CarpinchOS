#include "tlb.h"

void obtener_control_tlb();
void liberar_control_tlb();
entrada_tlb *es_entrada(uint32_t, uint32_t, uint32_t);

entrada_tlb *solicitar_entrada_tlb(uint32_t id_carpincho, uint32_t nro_pagina) {
	entrada_tlb *entrada;
	for(int i = 0; i < cant_entradas_tlb; i++) {
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

entrada_tlb *es_entrada(uint32_t nro_entrada, uint32_t id_car, uint32_t nro_pagina) {
	entrada_tlb *entrada = tabla_tlb[nro_entrada];
	if(entrada->id_car == id_car && entrada->pagina == nro_pagina)
		return entrada;
	else
		return NULL;
}
