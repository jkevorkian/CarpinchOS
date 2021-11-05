#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mateLib.h"

int main(int argc, char *argv[])
{
  // Lib instantiation
  mate_instance mate_ref;
  mate_init(&mate_ref, "../mateLib.config");

  int pepe = mate_memalloc(&mate_ref, 18);
  printf("Mem alloc terminado\n");
  printf("Obtuve %d", pepe);

  mate_close(&mate_ref);
  return 0;
}
