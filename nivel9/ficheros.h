#include "ficheros_basico.h" // Para funciones de bajo nivel (lectura/escritura de inodos)
#include <stdint.h>
#include <time.h>
#include <limits.h>



// Definición de la estructura STAT (igual que struct inodo pero sin punteros)
struct STAT {
    unsigned char tipo;
    unsigned char permisos;
    unsigned int nlinks;
    unsigned int tamEnBytesLog;
    time_t atime;
    time_t mtime;
    time_t ctime;
    time_t btime;
    unsigned int numBloquesOcupados;
};

// Prototipos de las funciones
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);