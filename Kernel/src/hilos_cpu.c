#include "hilos_cpu.h"

void iniciar_hilos_cpu() {
	hilos_cpu = list_create();

	for(int i=0; i<grado_multiprocesamiento; i++) {
		pthread_t *hilo_cpu = malloc(sizeof(pthread_t));
		pthread_create(hilo_cpu, NULL, cpu, NULL);
		list_add(hilos_cpu, hilo_cpu);
	}
}

void* cpu() {
	while(1) {
		sem_wait(&carpinchos_running);
		carpincho* carp = quitar_running();

		char *tiempo_inicio = temporal_get_string_time("%H:%M:%S:%MS"); // "12:51:59:331"

		log_info(logger, "Carpincho %d empezando a trabajar a la hora %s", carp->id, tiempo_inicio);

		bool seguir = true;
		int posicion;

		//se ejecutan las tareas para cada carpincho

		while(seguir) {
			t_list* mensaje_in = recibir_mensaje(carp->socket_mateLib);

			if (!validar_mensaje(mensaje_in, logger)) {
				log_error(logger, "Carpincho %d desconectado :p", carp->id);
				seguir = false;
			} else {
				t_mensaje* mensaje_out;
				switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
					case MEM_ALLOC:
					case MEM_FREE:
					case MEM_READ:
					case MEM_WRITE:
						mensaje_out = crear_mensaje((int)list_get(mensaje_in, 0)); //creo el mismo mensaje que recibi
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
						iniciar_semaforo((char *)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));

						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(carp->socket_memoria, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
					case SEM_WAIT:
						log_info(logger, "Recibi mensaje del carpincho &d solicitando bloquearse", carp->id);
						/*
						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(carp->socket_memoria, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						*/
						agregar_blocked(carp);
						seguir = false;
						break;
					case SEM_POST:
						posicion = buscar(lista_semaforos, (char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							semaforo *sem = (semaforo*)list_get(lista_semaforos, posicion);
							sem->instancias_iniciadas++;

							pthread_mutex_lock(&sem->mutex_espera);
							carpincho *carp = queue_pop(sem->cola_espera);
							pthread_mutex_unlock(&sem->mutex_espera);

							desbloquear(carp);
							carp->responder_wait = true;
						} else
							log_error(logger, "El semaforo %s no se ha encontrado", (char *)list_get(mensaje_in, 1));

						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(carp->socket_memoria, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
					case SEM_DESTROY:
						posicion = buscar(lista_semaforos, (char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							pthread_mutex_lock(&mutex_lista_semaforos);
							semaforo *sem = (semaforo*)list_remove(lista_semaforos, posicion);
							pthread_mutex_unlock(&mutex_lista_semaforos);

							if(!queue_is_empty(sem->cola_espera))
								log_warning(logger, "El semaforo %s tiene carpinchos esperando desbloquearse", (char *)list_get(mensaje_in, 1));

							queue_destroy(sem->cola_espera);
							free(sem->nombre);
							free(sem);
						} else
							log_error(logger, "El semaforo %s no se ha encontrado", (char *)list_get(mensaje_in, 1));

						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(carp->socket_memoria, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
					case CALL_IO:
						posicion = buscar(lista_IO, (char *)list_get(mensaje_in, 1));

						if (posicion != -1) {
							IO *io = (IO*) list_get(lista_IO, posicion);

							pthread_mutex_lock(&io->mutex_espera);
							queue_push(io->cola_espera, carp);
							pthread_mutex_unlock(&io->mutex_espera);

							sem_post(&io->carpinchos_esperando);
						} else
							log_error(logger, "El dispositivo de IO %s no se ha encontrado", (char *)list_get(mensaje_in, 1));

						break;
					case MATE_CLOSE:
						break;
					default:
						log_info(logger, "LLego: %d", (int)list_get(mensaje_in, 0));
						break;
				}
				liberar_mensaje_in(mensaje_in);
			}
		}
		char *tiempo_fin = temporal_get_string_time("%H:%M:%S:%MS");//todo verificar si lo tengo que hacer si el carpincho se bloquea

		carp->rafaga_real_anterior = obtener_rafaga_real(tiempo_inicio, tiempo_fin);
		carp->estimacion_proxima_rafaga = obtener_estimacion_proxima_rafaga(carp->rafaga_real_anterior, carp->estimacion_proxima_rafaga);

		sem_post(&multiprocesamiento);

	}
}


