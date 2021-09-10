/*
 * conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef _UTILS_CLIENT_H_
#define _UTILS_CLIENT_H_

#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

/**
* @NAME: crear_conexion_cliente
* @DESC: Crea un socket cliente que se conectará a un socket servidor que se ubicará en la ip y puerto definidos por parámetro.
* @OUTP: Devuelve el socket del cliente
*/
int crear_conexion_cliente(char* ip, char* puerto);

#endif /* _UTILS_CLIENT_H_ */