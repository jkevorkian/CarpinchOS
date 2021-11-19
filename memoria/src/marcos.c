#include "marcos.h"

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	// lo mismo con los marcos que se pidan para inificar el heapMetaData

	// uint32_t nro_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	t_marco* marco;
	t_carpincho* mi_carpincho = carpincho_de_lista(id_carpincho);
	// t_entrada_tp *entrada_tp = (t_entrada_tp *)list_get(mi_carpincho->tabla_paginas, nro_pagina);
	t_entrada_tp *entrada_tp = (t_entrada_tp *)list_get(mi_carpincho->tabla_paginas, nro_pagina);
	if(entrada_tp->presencia) {
		marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
		reservar_marco(marco);
	}
	else {
		// Page fault
		marco = realizar_algoritmo_reemplazo(id_carpincho);
		reasignar_marco(id_carpincho, nro_pagina, marco);
	 }
	return marco;
}

void actualizar_info_algoritmo(t_marco *marco_auxiliar, bool modificado) {
	if(modificado)
		marco_auxiliar->bit_modificado = true;

	if(config_memoria.algoritmo_reemplazo == LRU)
		marco_auxiliar->temporal = temporal_get_string_time("%H:%M:%S");
	else
		marco_auxiliar->bit_uso = true;
}

void reasignar_marco(uint32_t id_carpincho, uint32_t nro_pagina, t_marco* marco) {
	// actualizar_tp();
	// t_carpincho * nuevo_carpincho = list_get(lista_carpinchos, id_carpincho - 1);
	// t_carpincho * viejo_carpincho = list_get(lista_carpinchos, marco->duenio - 1);

	pthread_mutex_lock(&marco->mutex);
	if(id_carpincho) {
		// enviar pagina a swamp
		// swap_out(viejo_carpincho->id, nro_pagina);
		// viejo_carpincho->tabla_paginas[marco->pagina_duenio] = NULL; // ??
	}

	// nuevo_carpincho->tabla_paginas[nro_pagina] debería ser NULL al no estar asignado
	// nuevo_carpincho->tabla_paginas[nro_pagina] = marco;
	marco->duenio = id_carpincho;
	marco->pagina_duenio = nro_pagina;
	marco->bit_modificado = false;
	marco->bit_uso = true;

	// pedir pagina a swap
	// void * buffer = swap_in(id_carpincho, nro_pagina);
	//memcpy(inicio_memoria(marco->nro_real, 0), buffer, config_memoria.tamanio_pagina);
	// pthread_mutex_lock(&marco->mutex);

	//actualizar_tlb();


	// Actualizar tabla de paginas del que perdio el marco
}

void soltar_marco(t_marco *marco_auxiliar) {
	// pthread_mutex_unlock(&marco_auxiliar->mutex);
}

void reservar_marco(t_marco *marco_auxiliar) {
	// pthread_mutex_lock(&marco_auxiliar->mutex);
}

t_marco* obtener_marco_libre() {
    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			marco->libre = false; //mutex
			return marco;
		}
    }

    /*
    t_marco* marco = pedir_frame_swap();
    */

	return NULL;
}

uint32_t cant_marcos_necesarios(uint32_t tamanio) {
	div_t nro_marcos = div(tamanio, config_memoria.tamanio_pagina);
	uint32_t nro_marcos_q = nro_marcos.quot;

	if(nro_marcos.rem > 0) nro_marcos_q++;
	return nro_marcos_q;
}

bool tengo_marcos_suficientes(uint32_t necesarios){
	// no entiendo si yo aca devuelvo que tengo marcos necesarios pero despues mientras le asigno me quedo sin marcos porque los uso otro proceso que pasa?
    uint32_t contador_necesarios = necesarios;

    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			contador_necesarios--;
		}
        if(contador_necesarios == 0) return true;
    }

    //seguir buscando en swap

    return false;
}

t_marco* asignar_marco_libre(uint32_t nro_marco, uint32_t id) {
	pthread_mutex_lock(&memoria_ram.mutex_mapa);	// no se si hace falta
	t_marco* marco_nuevo = memoria_ram.mapa_fisico[nro_marco];
	pthread_mutex_unlock(&memoria_ram.mutex_mapa);
	
	// mutex ?
	marco_nuevo->duenio = id;
	marco_nuevo->pagina_duenio = nro_marco;
    marco_nuevo->libre = false;				// to remove o cambiar lo de duenio

	return marco_nuevo;
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco, t_carpincho* carpincho){
	t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
	list_add(carpincho->tabla_paginas, pagina);
	pagina->nro_marco = nro_marco;
	pagina->presencia = true;

	// -> Esto agregaría yo, si el marco está en memoria
	t_marco* marco_nuevo = asignar_marco_libre(nro_marco, carpincho->id);
	marco_nuevo->pagina_duenio = list_size(carpincho->tabla_paginas) - 1;
	// <- */

	log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
	// log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);

	return pagina;
}