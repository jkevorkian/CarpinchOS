#ifndef _MAIN_H_
#define _MAIN_H_

#include "servidor.h"
#include <signal.h>

void signal_handler_1(int);
void signal_handler_2(int);
void signal_handler_3(int);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(t_log*, t_config*);

#endif /* _MAIN_H_ */
