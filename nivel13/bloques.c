#include "bloques.h"
#include "semaforo_mutex_posix.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

// Variable global para el descriptor del dispositivo
static int descriptor = 0;

// Variables globales para el semáforo
static sem_t *mutex = NULL;
static unsigned int inside_sc = 0;

int bmount(const char *camino){
    umask(0000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if (descriptor == -1) {
        perror("Error en bmount");
        return FALLO;
    }

    // Inicializar el semáforo solo una vez
    if (!mutex) {
        mutex = initSem();
        if (mutex == SEM_FAILED) {
            return FALLO;
        }
    }

    return descriptor;
}

int bumount() {
    int i = close(descriptor);
    if (i == -1) {
        perror("Error en bumount");
        return FALLO;
    }

    deleteSem();    // Eliminar el semáforo
    mutex = NULL;   // Evita punteros colgantes

    return EXITO;
}

// Funciones para gestionar el semáforo de exclusión mutua
void mi_waitSem() {
    if (!inside_sc) {
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}

int bwrite(unsigned int nbloque, const void *buf) {
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        perror("Error en bwrite (lseek)");
        return FALLO;
    }
    ssize_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);
    if (bytes_escritos == -1) {
        perror("Error en bwrite (write)");
        return FALLO;
    }
    return bytes_escritos;
}

int bread(unsigned int nbloque, void *buf) {
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        perror("Error en bread (lseek)");
        return FALLO;
    }
    ssize_t nbytes = read(descriptor, buf, BLOCKSIZE);
    if (nbytes == -1) {
        perror("Error en bread (read)");
        return FALLO;
    }
    return nbytes;
}
