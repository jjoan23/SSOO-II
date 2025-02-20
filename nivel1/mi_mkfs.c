#include "bloques.h"


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Uso: %s <nombre_dispositivo> <nbloques>\n", argv[0]);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    int nbloques = atoi(argv[2]);

    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == -1) {
        printf("Error al montar el dispositivo virtual\n");
        return FALLO;
    }

    // Inicializar el buffer de memoria
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);

    // Escribir el número de bloques necesarios
    for (int i = 0; i < nbloques; i++) {
        if (bwrite(i, buffer) == -1) {
            printf("Error al escribir en el dispositivo virtual\n");
            return FALLO;
        }
    }

    // Desmontar el dispositivo virtual
    if (bumount() == -1) {
        printf("Error al desmontar el dispositivo virtual\n");
        return FALLO;
    }

    printf("Dispositivo virtual formateado correctamente\n");
    return EXITO;
}