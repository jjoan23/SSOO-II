// semaforo_mutex_posix.h
#ifndef SEMAFORO_MUTEX_POSIX_H
#define SEMAFORO_MUTEX_POSIX_H

#include <semaphore.h>
#include <fcntl.h>      // Para O_CREAT
#include <sys/stat.h>   // Para S_IRWXU
#include <stddef.h>     // Para NULL
#include <unistd.h>     // Para close, si lo usas

#define SEM_NAME "/mymutex"
#define SEM_INIT_VALUE 1

sem_t *initSem();
void deleteSem();
void signalSem(sem_t *sem);
void waitSem(sem_t *sem);

#endif // SEMAFORO_MUTEX_POSIX_H
