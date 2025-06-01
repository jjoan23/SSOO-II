#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include "directorios.h"

#define REGMAX 500000
#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define TAMRUTA 128

struct REGISTRO {
    time_t fecha;       // Fecha de la escritura
    pid_t pid;          // PID del proceso que escribe
    int nEscritura;     // Número de escritura (1 a NUMESCRITURAS)
    int nRegistro;      // Posición del registro (0 a REGMAX-1)
};

