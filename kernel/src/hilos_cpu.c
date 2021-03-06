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

		if(carp->esperar_cliente) {
			log_info(logger, "Carpincho %d creando conexion exclusiva", carp->id);
			int socket_m = esperar_cliente(carp->socket_mateLib);
			close(carp->socket_mateLib);
			carp->socket_mateLib = socket_m;
			carp->esperar_cliente = false;
		}

		char *tiempo_inicio = temporal_get_string_time("%H:%M:%S:%MS"); // "12:51:59:331"

		log_info(logger, "Carpincho %d empezando a trabajar en el instante %s", carp->id, tiempo_inicio);

		bool seguir = true;
		bool carpincho_finalizado = false;
		int posicion;
		t_mensaje* mensaje_out;//

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
				int parametro = (int)list_get(mensaje_in, 0);
				log_warning(logger, "Carpincho %d envio un %s", carp->id, string_desde_mensaje(parametro));

				switch(parametro) { // protocolo del mensaje
					case MEM_ALLOC:
					case MEM_FREE:
					case MEM_READ:
					case MEM_WRITE:
						if (MEMORIA_ACTIVADA) {
							log_info(logger, "Carpincho %d entrando a la comunicacion con la memoria", carp->id);

							mensaje_out = crear_mensaje(parametro);									   //creo el mismo mensaje que recibi
							agregar_a_mensaje(mensaje_out, "%d", (int)list_get(mensaje_in, 1));		   //le agrego el parametro recibido

							if (parametro == MEM_WRITE)											   //si es un MEM_WRITE mateLib manda un parametro extra,
								agregar_a_mensaje(mensaje_out, "%sd", list_get(mensaje_in, 2), list_get(mensaje_in, 3)); //asi que tengo que agregarlo
							else if (parametro == MEM_READ)
								agregar_a_mensaje(mensaje_out, "%d", list_get(mensaje_in, 2));

							log_info(logger, "Carpincho %d creo el mensaje %s", carp->id,  string_desde_mensaje(parametro));

							enviar_mensaje(carp->socket_memoria, mensaje_out);
							liberar_mensaje_out(mensaje_out);

							log_info(logger, "Carpincho %d esperando respuesta", carp->id);

							t_list *mensaje_mateLib = recibir_mensaje(carp->socket_memoria); //espero la respuesta de la ram

							int codigo_respuesta = (int)list_get(mensaje_mateLib, 0);

							mensaje_out = crear_mensaje(codigo_respuesta); //tal cual llega genero el mismo mensaje para enviarselo a mateLib
							if (codigo_respuesta == DATA_PAGE)					//si es un MEM_READ la memoria me devuelve el mensaje DATA con un parametro, asi que en ese caso tengo que agregarlo
								agregar_a_mensaje(mensaje_out, "%sd", list_get(mensaje_mateLib, 1), list_get(mensaje_mateLib, 2));
							if (codigo_respuesta == DATA_INT || codigo_respuesta == SEG_FAULT)					//si es un MEM_ALLOC la memoria me devuelve el mensaje DATA con un parametro, asi que en ese caso tengo que agregarlo
								agregar_a_mensaje(mensaje_out, "%d", (char *)list_get(mensaje_mateLib, 1));

							enviar_mensaje(carp->socket_mateLib, mensaje_out);
							log_info(logger, "Carpincho %d devolvio la respuesta proporcionada por la ram (%s)", carp->id, string_desde_mensaje(codigo_respuesta));
							liberar_mensaje_out(mensaje_out);

							if (parametro == MEM_WRITE)
								free(list_get(mensaje_in, 3));
							if (codigo_respuesta == DATA_PAGE)
								free(list_get(mensaje_mateLib, 2));

							liberar_mensaje_in(mensaje_mateLib);
						}else {
							if (parametro == MEM_READ) {
								mensaje_out = crear_mensaje(DATA_CHAR);
								agregar_a_mensaje(mensaje_out, "%s", "Memoria desactivada, respondo datos de prueba");
							} else if (parametro == MEM_ALLOC) {
								mensaje_out = crear_mensaje(DATA_INT);
								agregar_a_mensaje(mensaje_out, "%d", 9);
							} else {
								if (parametro == MEM_WRITE)
									log_info(logger, "Se recibio para escribir: %s", (char *)list_get(mensaje_in, 2));
								mensaje_out = crear_mensaje(TODOOK);
							}

							log_info(logger, "Carpincho %d devolvio %s", carp->id, string_desde_mensaje(mensaje_out->op_code));
							enviar_mensaje(carp->socket_mateLib, mensaje_out);
							liberar_mensaje_out(mensaje_out);
						}
						break;
					case SEM_INIT:
						iniciar_semaforo((char *)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));  //busca el semaforo, si no existe lo crea y lo agrega a la lista

						mensaje_out = crear_mensaje(TODOOK);
						enviar_mensaje(carp->socket_mateLib, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						free(list_get(mensaje_in, 1));
						break;
					case SEM_WAIT:
						posicion = buscar_semaforo((char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							semaforo *sem = (semaforo*)list_get(lista_semaforos, posicion);

							log_info(logger, "Encontrado el semaforo %s", sem->nombre);

							if(sem->instancias_iniciadas > 0) { //si hay instancias disponibles no se bloquea
								log_info(logger, "Habian %d instancias disponibles del semaforo %s, el carpincho %d no se bloquea", sem->instancias_iniciadas, (char *)list_get(mensaje_in, 1), carp->id);
								sem->instancias_iniciadas--;

								int pos_s = buscar_sem_en_lista(carp->semaforos_asignados,sem->nombre);
								//deadlock
								if(pos_s == -1) {
									if(LOGUEAR_MENSAJES_DEADLOCK)
										log_info(logger, "Se le agrega el sem a los asignados del carpincho %d", carp->id);

									sem_deadlock* sem_d = malloc(sizeof(sem_deadlock));
									sem_d->sem = sem;
									sem_d->cantidad_asignada = 1;
									list_add(carp->semaforos_asignados, sem_d);
								} else {
									if(LOGUEAR_MENSAJES_DEADLOCK)
										log_info(logger, "El carpincho %d ya habia tenia asignado el sem, no se le agrega", carp->id);

									sem_deadlock* aux = list_get(carp->semaforos_asignados, pos_s);
									aux->cantidad_asignada++;
								}

								mensaje_out = crear_mensaje(TODOOK);
								enviar_mensaje(carp->socket_mateLib, mensaje_out);
								liberar_mensaje_out(mensaje_out);
							}
							else {
								log_info(logger, "No habian instancias disponibles del semaforo %s (cantidad en lista de espera: %d)", (char *)list_get(mensaje_in, 1), queue_size(sem->cola_espera));
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
						free(list_get(mensaje_in, 1));
						break;
					case SEM_POST:
						posicion = buscar_semaforo((char *)list_get(mensaje_in, 1));

						if(posicion != -1) {
							semaforo *sem = (semaforo*)list_get(lista_semaforos, posicion);
							log_info(logger, "Cantidad de carpinchos esperando en el semaforo %s: %d", (char *)list_get(mensaje_in, 1), queue_size(sem->cola_espera));

							pthread_mutex_lock(&sem->mutex_espera);
								if(queue_is_empty(sem->cola_espera)) {
									sem->instancias_iniciadas++;

									int pos_sem = buscar_sem_en_lista(carp->semaforos_asignados, sem->nombre);
									if(pos_sem != -1) {
										sem_deadlock* aux = list_get(carp->semaforos_asignados, pos_sem);
										aux->cantidad_asignada--;

										if(aux->cantidad_asignada <= 0)
											list_remove(carp->semaforos_asignados, pos_sem);
									}
								} else {
									carpincho *carp_desbloquear;
									bool continuar_buscando = true;

									while(continuar_buscando) {
										carp_desbloquear = queue_pop(sem->cola_espera);

										if(carp_desbloquear->id == -1) {
											log_error(logger, "El carpincho ya no existe, intentando con el siguiente");
											free(carp_desbloquear);

											if(queue_is_empty(sem->cola_espera)) {
												sem->instancias_iniciadas++;

												int pos_sem = buscar_sem_en_lista(carp->semaforos_asignados, sem->nombre);
												if(pos_sem != -1) {
													sem_deadlock* aux = list_get(carp->semaforos_asignados, pos_sem);
													aux->cantidad_asignada--;

													if(aux->cantidad_asignada <= 0)
														list_remove(carp->semaforos_asignados, pos_sem);
												}
												continuar_buscando = false;
												log_info(logger, "No habia mas carpinchos esperando, agregando una instancia disponible");
											}
										} else {
											continuar_buscando = false;
											desbloquear(carp_desbloquear);
											carp_desbloquear->responder = true;
											carp_desbloquear->id_semaforo_bloqueante = -1;
										}
									}
								}
							pthread_mutex_unlock(&sem->mutex_espera);

							mensaje_out = crear_mensaje(TODOOK);
						} else {
							log_error(logger, "No se ha encontrado el semaforo %s", (char *)list_get(mensaje_in, 1));

							mensaje_out = crear_mensaje(SEG_FAULT);
						}

						enviar_mensaje(carp->socket_mateLib, mensaje_out);
						liberar_mensaje_out(mensaje_out);
						free(list_get(mensaje_in, 1));
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
						free(list_get(mensaje_in, 1));
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
						free(list_get(mensaje_in, 1));
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
						carpincho_finalizado = true;
						seguir = false;
						mate_close++;
						sem_post(&multiprogramacion);
						grado_multiprogramacion++;
						//log_info(logger, "El carpincho ha sido finalizado (%d, %d)", grado_multiprogramacion, mate_close);
						log_info(logger, "El carpincho ha sido finalizado");
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
		grado_multiprocesamiento++;
	}
}


