#include "carpincho.h"

void *rutina_carpincho(void* info_carpincho) {
	t_list* mensaje_in;
	bool seguir = true;
	int socket_carpincho = ((data_carpincho *)info_carpincho)->socket;
	// en asignación fija, reservar paginas
	while(seguir) {
		mensaje_in = recibir_mensaje(socket_carpincho);
		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case MEM_ALLOC:
			// int tamanio = (int)list_get(mensaje_in, 1);
			// mem_alloc(id_carpincho, tamanio);
			break;
		case MEM_FREE:
			// mem_free(id_carpincho, dir_logica);
			break;
		case MEM_READ:
			// mem_read(id_carpincho, dir_logica);
			break;
		case MEM_WRITE:
			// mem_write(id_carpincho, dir_logica, data);
			break;
		case SUSPEND:
			// ...;
			break;
		}
	}
	return NULL;
}

void mem_alloc(uint32_t id_car, uint32_t tamanio) {

}
