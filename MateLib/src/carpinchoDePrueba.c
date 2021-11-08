#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <matelib.h>

void* io_thread(void* mate_ref) {
    mate_sem_wait(mate_ref, "SEM1");
    printf("And this one second...\n");


    // Memory tooling usage
    mate_pointer ptr = mate_memalloc(mate_ref, sizeof(char) * 8);
    mate_memwrite(mate_ref, "PRINTER", ptr, sizeof(char) * 8);
    char *io_type = malloc(sizeof(char) * 8);
    mate_memread(mate_ref, ptr, io_type, sizeof(char) * 8);

    // IO Usage
    mate_call_io(mate_ref, io_type, "I'm content to print...");

    // Freeing Memory
    // Closing Lib
    free(io_type);
    mate_memfree(mate_ref, ptr);


    return 0;
}


int main(int argc, char *argv[])
{
  // Lib instantiation
  mate_instance mate_ref;
  mate_init(&mate_ref, "../mateLib.config");

  // Let's work with semaphores
  mate_sem_init(&mate_ref, "SEM1", 0);
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, &io_thread, &mate_ref);

  usleep(1000);
  printf("This message should go first...\n");
  mate_sem_post(&mate_ref, "SEM1");

  pthread_join(thread_id, NULL);

  mate_sem_destroy(&mate_ref, "SEM1");

  mate_close(&mate_ref);
  return 0;
}
