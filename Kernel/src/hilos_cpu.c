#include "hilos_cpu.h"

void iniciar_hilos_cpu() {
	hilos_cpu = list_create();

	for(int i=0; i<grado_multiprocesamiento; i++) {
		pthread_t *hilo_cpu;
		list_add(hilos_cpu, hilo_cpu);
		pthread_create(hilo_cpu, NULL, cpu, NULL);
	}
}

void* cpu() {
	sem_wait(&carpinchos_running);
	carpincho* carp = quitar_running();

	bool seguir = true;

	//se ejecutan las tareas para cada carpincho

	while(seguir) {
		t_list* mensaje_in = recibir_mensaje(carp->socket_mateLib);

		if (!validar_mensaje(mensaje_in, logger)) {
			log_error(logger, "Carpincho desconectado");
			seguir = false;
		} else {
			switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
				case MEM_ALLOC:
				case MEM_FREE:
				case MEM_READ:
				case MEM_WRITE:
					t_mensaje* mensaje_out = crear_mensaje((int)list_get(mensaje_in, 0)); //creo el mismo mensaje que recibi
					agregar_a_mensaje(mensaje_out, "%d", (int)list_get(mensaje_in, 1)); //le agrego el parametro recibido
					enviar_mensaje(carp->socket_memoria, mensaje_out);

					liberar_mensaje_out(mensaje_out);

					t_list* mensaje_mateLib = recibir_mensaje(carp->socket_memoria); //espero la respuesta de la ram

					mensaje_out = crear_mensaje((int)list_get(mensaje_mateLib, 0)); //tal cual llega genero el mismo mensaje para enviarselo a mateLib
					if((int)list_get(mensaje_mateLib, 0) == DATA) 					//si es un MEM_READ la memoria me devuelve el mensaje DATA con un parametro, asi que en ese caso tengo que agregarlo
						agregar_a_mensaje(mensaje_out, "%s", (int)list_get(mensaje_mateLib, 1));
					enviar_mensaje(carp->socket_mateLib, mensaje_out);

					liberar_mensaje_out(mensaje_out);
					liberar_mensaje_in(mensaje_mateLib);
					break;
				case SEM_INIT:
					break;
				case SEM_WAIT:
					break;
				case SEM_POST:
					break;
				case SEM_DESTROY:
					break;
				case CALL_IO:
					break;
				case MATE_CLOSE:
					break;
			}
			liberar_mensaje_in(mensaje_in);
		}
	}
	return 0;
}

