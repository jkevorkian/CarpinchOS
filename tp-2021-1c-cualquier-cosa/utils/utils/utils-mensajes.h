/*
*	utils-mensajes.h
*
*	Created on: 1 jun. 2021
*	Author: cualquier-cosa
*/

#ifndef _UTILS_MENSAJES_H_
#define _UTILS_MENSAJES_H_

#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/collections/list.h>
#include<commons/log.h>

#include<stdio.h>
#include<errno.h>

typedef enum {
	INIT_P,	// Iniciar patota
		// INIT_P | cant_tareas [int] | (tareas)
	
	INIT_T,	// Iniciar tripulante
		// INIT_T | posicion_x [int] | posicion_y [int]
	
	DATA_T,	// Conocer tripulante
		// DATA_T | id_tripulante [int] | id_patota [int]
	
	SHOW_T,	// Mostrar información de tripulante
		// SHOW_T | estado [int] | posicion_x [int] | posicion_y [int]
	
	ELIM_T,	// Expulsar tripulante
		// ELIM_T | id_tripulante [int] | id_patota [int]
	
	NEW_PO,	// Solicitar Puerto
		// NEW_PO
	
	SND_PO,	// Enviar puerto
		// SND_PO | puerto [int]
	
	ACTU_P,	// Actualizar posición
		// ACTU_T | posicion_x [int] | posicion_y [int]
	
	ACTU_E,	// Actualizar estado
		// ACTU_T | estado [char]
	
	NEXT_T,	// Próxima tarea
		// NEXT_T
	
	TASK_T,	// Enviar tarea
		// TASK_T | largo_tarea [int] | tarea [str]
	
	BITA_D,	// Obtener bitácora
		// BITA_D | id_trip [int] | id_patota [int]
	
	BITA_T,	// Actualizar bitácora
		// Por definir
	
	BITA_C,	// Mostrar Bitácora
		// BITA_C | cant_lineas [int] | (líneas)
	
	TAR_ES,	// Tarea E/S
		// Por definir
	
	SABO_P,	// Nuevo Sabotaje
		// SABO_P | id_patota [int] | id_trip [int] | posicion_x [int] | posicion_y [int]
	
	TODOOK,	// Validación correcta
		// TODOOK
	
	NO_SPC,	// Error, no hay espacio
		// NO_SPC
	
	ER_MSJ,	// Error, no aplica protocolo
		// ER_MSJ
	
	ER_RCV,	// Error descononocido al recibir un mensaje
		// ER_RCV
	
	ER_SOC,	// Error al recibir mensaje, el socket remoto se desconectó
		// ER_SOC
	// ADAPTANDO CON DISCORDIADOR
	INIT_S,
	SABO_I,
	SABO_F,
	GEN_OX,
	CON_OX,
	GEN_CO,
	CON_CO,
	GEN_BA,
	DES_BA,
	EXEC_1,
	EXEC_0,
} protocolo_msj;

typedef enum {
	ENTERO,
	BUFFER
} tipo_msj;

typedef struct {
	int tamanio;
	void* contenido;
} t_buffer;

typedef struct {
	protocolo_msj op_code;
	t_buffer* buffer;
} t_mensaje;

// Funciones para manejo de mensajes
/**
* @NAME: crear_mensaje
* @DESC: Crea una estructura t_mensaje con un protocolo definido por el parametro
* @OUTP: Devuelve un puntero a la estructura t_mensaje creada
*/
t_mensaje* crear_mensaje(protocolo_msj cod_op);

/**
* @NAME: agregar_parametro_a_mensaje
* @DESC: Permite agregar un parametro a un mensaje
* @NOTA: Se debe explicitar cuál es el tipo del parametro
*/
void agregar_parametro_a_mensaje(t_mensaje* mensaje, void* parametro, tipo_msj tipo_parametro);

/**
* @NAME: enviar_mensaje
* @DESC: Envía un mensaje a través de un socket
*/
void enviar_mensaje(int socket, t_mensaje* mensaje);

/**
* @NAME: recibir_mensaje
* @DESC: Espera un mensaje desde un socket y devuelve una lista con los parámetros de este
* @OUTP: Devuelve una lista de parametros que corresponden con el protocolo y los parametros definidos por el emisor
		antes de que haya enviado el mensaje
* @NOTA: Si no hay conexion, CREO que devuelve NULL
*/
t_list* recibir_mensaje(int socket);

/**
* @NAME: liberar_mensaje_out
* @DESC: Libera la memoria correspondiente a un mensaje de salida
*/
void liberar_mensaje_out(t_mensaje* mensaje);

/**
* @NAME: liberar_mensaje_in
* @DESC: Libera la memoria correspondiente a un mensaje de entrada
*/
void liberar_mensaje_in(t_list* mensaje);

/**
* @NAME: validar_mensaje
* @DESC: Recibe la salida de un recibir_mensaje e informa si hubo errores
* @OUTP: Devuelve true si el mensaje es válido
*/
bool validar_mensaje(t_list* mensaje_in, t_log* logger);

#endif /* _UTILS_MENSAJES_H_ */
