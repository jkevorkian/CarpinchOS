#include "hilos_cpu.h"

void iniciar_hilos_cpu() {
	hilos_cpu = list_create();

	for(int i=0; i<grado_multiprocesamiento; i++) {
		pthread_t *hilo_cpu = malloc(sizeof(pthread_t));
		pthread_create(hilo_cpu, NULL, cpu, NULL);
		list_add(hilos_cpu, hilo_cpu);
	}

	if(LOGUEAR_MENSAJES_INICIALIZADOR)
		log_info(logger, "\tIniciados los %d hilos de CPU", grado_multiprocesamiento);
}

void* cpu() {
	while(1) {
		sem_wait(&carpinchos_running);
		carpincho* carp = quitar_running();

		char *tiempo_inicio = temporal_get_string_time("%H:%M:%S:%MS"); // "12:51:59:331"

		log_info(logger, "Carpincho %d empezando a trabajar en el instante %s", carp->id, tiempo_inicio);

		bool seguir = true;
		bool carpincho_finalizado = false;
		int posicion;
		t_mensaje* mensaje_out;

		if(carp->responder) {
			log_info(logger, "Carpincho %d devolviendo un TODOOK luego de ser bloqueado", carp->id);
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(carp->socket_mateLib, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			carp->responder = false;
		}
		//se ejecutan las tareas para cada carpincho

		while(seguir) {
			t_list* mensaje_in = recibir_mensaje(carp->socket_mateLib);

			if (!validar_mensaje(mensaje_in, logger)) {
				log_error(logger, "Carpincho %d desconectado :p", carp->id);
				seguir = false;
			} else {
				log_warning(logger, "Carpincho %d envio un %s", carp->id, string_desde_mensaje((int)list_get(mensaje_in, 0)));

				int parametro = (int)list_get(mensaje_in, 0);

				//if (carp->debe_morir)
				//	parametro = MATE_CLOSE;

				switch(parametro) { // protocolo del mensaje
					case MEM_ALLOC:
					case MEM_FREE:
					case MEM_READ:
					case MEM_WRITE:
						if (MEMORIA_ACTIVADA) {
							log_info(logger, "Carpincho %d entrando a la comunicacion con la memoria", carp->id);

							mensaje_out = crear_mensaje(parametro);									   //creo el mismo mensaje que recibi
							agregar_a_mensaje(mensaje_out, "%d", (int)list_get(mensaje_in, 1));		   //le agrego el parametro recibido

							if (parametro == MEM_WRITE)												   //si es un MEM_WRITE mateLib manda un parametro extra,
								agregar_a_mensaje(mensaje_out, "%s", (char *)list_get(mensaje_in, 2)); //asi que tengo que agregarlo

							log_info(logger, "Carpincho %d creo el mensaje %s, le agrego el parametro %d y lo va a enviar al socket %d", carp->id,  string_desde_mensaje(parametro),  (int)list_get(mensaje_in, 1), carp->socket_memoria);
							enviar_mensaje(carp->socket_memoria, mensaje_out);
							liberar_mensaje_out(mensaje_out);

							log_info(logger, "Carpincho %d esperando respuesta", carp->id);
							t_list *mensaje_mateLib = recibir_mensaje(carp->socket_memoria); //espero la respuesta de la ram
							int codigo_respuesta = (int)list_get(mensaje_mateLib, 0);

							mensaje_out = crear_mensaje((int)list_get(mensaje_mateLib, 0)); //tal cual llega genero el mismo mensaje para enviarselo a mateLib
							if (codigo_respuesta == DATA_CHAR)					//si es un MEM_READ la memoria me devuelve el mensaje DATA con un parametro, asi que en ese caso tengo que agregarlo
								agregar_a_mensaje(mensaje_out, "%s", (char *)list_get(mensaje_mateLib, 1));
							if (codigo_respuesta == DATA_INT || codigo_respuesta == SEG_FAULT)					//si es un MEM_ALLOC la memoria me devuelve el mensaje DATA con un parametro, asi que en ese caso tengo que agregarlo
								agregar_a_mensaje(mensaje_out, "%d", (char *)list_get(mensaje_mateLib, 1));

							enviar_mensaje(carp->socket_mateLib, mensaje_out);
							liberar_mensaje_out(mensaje_out);

							liberar_mensaje_in(mensaje_mateLib);
						}else {
							if ((int)list_get(mensaje_in, 0) == MEM_READ) {
								mensaje_out = crear_mensaje(DATA_CHAR);
								agregar_a_mensaje(mensaje_out, "%s", "Memoria desactivada, respondo datos de prueba");
							} else {
								if ((int)list_get(mensaje_in, 0) == MEM_WRITE)
									log_info(logger, "Se recibio para escribir: %s", (char *)list_get(mensaje_in, 2));
								mensaje_out = crear_mensaje(TODOOK);
							}

							enviar_mensaje(carp->socket_mateLib, mensaje_out);
							liberar_mensaje_out(mensaje_out);
						}
						break;
					case SEM_INIT:
						iniciar_semaforo((char *)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));  //busca el semaforo, si no existe lo crea y lo agrega a la lista

						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(carp->socket_mateLib, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
					case SEM_WAIT:
						posicion = buscar_semaforo((char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							semaforo *sem = (semaforo*)list_get(lista_semaforos, posicion);

							log_info(logger, "Encontrado el semaforo %s", sem->nombre);

							if(sem->instancias_iniciadas > 0) { //si hay instancias disponibles no se bloquea
								log_info(logger, "Habian %d instancias disponibles del semaforo %s, el carpincho %d no se bloquea", sem->instancias_iniciadas, (char *)list_get(mensaje_in, 1), carp->id);
								sem->instancias_iniciadas--;
								list_add(carp->semaforos_asignados, sem);
								mensaje_out = crear_mensaje(TODOOK);
								enviar_mensaje(carp->socket_mateLib, mensaje_out);
								liberar_mensaje_out(mensaje_out);
							}
							else {
								log_info(logger, "No habian instancias disponibles del semaforo %s (cantidad en lista de espera: %d)", (char *)list_get(mensaje_in, 1), queue_size(sem->cola_espera), carp->id);
								pthread_mutex_lock(&sem->mutex_espera);
								queue_push(sem->cola_espera, carp);
								pthread_mutex_unlock(&sem->mutex_espera);
								carp->id_semaforo_bloqueante = sem->id;
								agregar_blocked(carp);
								seguir = false;
							}
						} else {
							log_error(logger, "No se ha encontrado el semaforo %s", (char *)list_get(mensaje_in, 1));

							mensaje_out = crear_mensaje(SEG_FAULT);
							enviar_mensaje(carp->socket_mateLib, mensaje_out);
							liberar_mensaje_out(mensaje_out);
						}
						break;
					case SEM_POST:
						posicion = buscar_semaforo((char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							semaforo *sem = (semaforo*)list_get(lista_semaforos, posicion);

							pthread_mutex_lock(&sem->mutex_espera);
								if(queue_is_empty(sem->cola_espera))
									sem->instancias_iniciadas++;
								else {
									carpincho *carp = queue_pop(sem->cola_espera);
									list_remove(carp->semaforos_asignados, buscar_sem_en_lista(carp->semaforos_asignados, sem->nombre));
									desbloquear(carp);
									carp->responder = true;
								}
							pthread_mutex_unlock(&sem->mutex_espera);

							mensaje_out = crear_mensaje(TODOOK);
						} else {
							log_error(logger, "No se ha encontrado el semaforo %s", (char *)list_get(mensaje_in, 1));

							mensaje_out = crear_mensaje(SEG_FAULT);
						}

						enviar_mensaje(carp->socket_mateLib, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
					case SEM_DESTROY:
						posicion = buscar_semaforo((char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							pthread_mutex_lock(&mutex_lista_semaforos);
							semaforo *sem = (semaforo*)list_remove(lista_semaforos, posicion);
							pthread_mutex_unlock(&mutex_lista_semaforos);

							if(!queue_is_empty(sem->cola_espera)) {
								log_error(logger, "El semaforo %s tiene carpinchos esperando desbloquearse", (char *)list_get(mensaje_in, 1));

								mensaje_out = crear_mensaje(SEG_FAULT);
							} else {
								free(sem->nombre);
								queue_destroy(sem->cola_espera);
								pthread_mutex_destroy(&sem->mutex_espera);
								free(sem);

								mensaje_out = crear_mensaje(TODOOK);
							}
						} else {
							log_error(logger, "No se ha encontrado el semaforo %s", (char *)list_get(mensaje_in, 1));

							mensaje_out = crear_mensaje(SEG_FAULT);
						}

						enviar_mensaje(carp->socket_mateLib, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
					case CALL_IO:
						posicion = buscar_io((char *)list_get(mensaje_in, 1));

						if (posicion != -1) {
							IO *io = (IO*) list_get(lista_IO, posicion);

							log_info(logger, "Encontrado el dispositivo %s en la posicion %d", io->nombre, posicion);

							pthread_mutex_lock(&io->mutex_espera);
							queue_push(io->cola_espera, carp);
							pthread_mutex_unlock(&io->mutex_espera);
							agregar_blocked(carp);
							sem_post(&io->carpinchos_esperando);
							seguir = false;
						} else {
							log_error(logger, "El dispositivo de IO %s no se ha encontrado", (char *)list_get(mensaje_in, 1));

							mensaje_out = crear_mensaje(SEG_FAULT);
							enviar_mensaje(carp->socket_mateLib, mensaje_out);
							liberar_mensaje_out(mensaje_out);
						}
						break;
					case MATE_CLOSE:
						if(MEMORIA_ACTIVADA) {
							t_mensaje* mensaje_out = crear_mensaje(MATE_CLOSE);
							enviar_mensaje(carp->socket_memoria, mensaje_out);
							liberar_mensaje_out(mensaje_out);
						}

						close(carp->socket_mateLib);
						close(carp->socket_memoria);
						free(carp->tiempo_llegada);
						free(carp);
						grado_multiprogramacion++;
						carpincho_finalizado = true;
						seguir = false;
						break;
					default:
						log_info(logger, "Carpincho %d devolviendo un SEG_FAULT", carp->id);
						mensaje_out = crear_mensaje(SEG_FAULT);
						enviar_mensaje(carp->socket_mateLib, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						break;
				}
				liberar_mensaje_in(mensaje_in);
			}
		}

		if(!carpincho_finalizado) {
			char *tiempo_fin = temporal_get_string_time("%H:%M:%S:%MS");//todo verificar si lo tengo que hacer si el carpincho se bloquea

			log_info(logger, "Carpincho %d termino de trabajar en el instante %s", carp->id, tiempo_fin);

			carp->rafaga_real_anterior = obtener_rafaga_real(tiempo_inicio, tiempo_fin);
			carp->estimacion_proxima_rafaga = obtener_estimacion_proxima_rafaga(carp->rafaga_real_anterior, carp->estimacion_proxima_rafaga);

			free(tiempo_fin);
		}

		free(tiempo_inicio);
		sem_post(&multiprocesamiento);
		grado_multiprocesamiento--;
	}
}


