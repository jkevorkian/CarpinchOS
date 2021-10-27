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
					t_mensaje* mensaje_out = crear_mensaje(MEM_ALLOC);

					agregar_a_mensaje(mensaje_out, "%d", (int)list_get(mensaje_in, 1));
					enviar_mensaje(carp->socket_memoria, mensaje_out);

					liberar_mensaje_out(mensaje_out);

					t_list* mensaje_mateLib = recibir_mensaje(carp->socket_memoria);

					mensaje_out = crear_mensaje(TODOOK);
					enviar_mensaje(carp->socket_mateLib, mensaje_out);

					liberar_mensaje_out(mensaje_out);
					liberar_mensaje_in(mensaje_mateLib);
					break;
				case MEM_FREE:
					break;
				case MEM_READ:
					break;
				case MEM_WRITE:
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

