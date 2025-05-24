#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: %s <disco> </ruta_fichero_original> </ruta_enlace>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *nombre_disco = argv[1];
    char *ruta_original = argv[2];
    char *ruta_enlace = argv[3];

    if (bmount(nombre_disco) == -1) {
        perror("Error montando el disco");
        return EXIT_FAILURE;
    }

    // Comprobar que la ruta original es un fichero
    struct STAT stat;
    if (mi_stat(ruta_original, &stat) < 0) {
        fprintf(stderr, "Error: El fichero original no existe o no se puede acceder\n");
        bumount();
        return EXIT_FAILURE;
    }
    if (stat.tipo != 'f') {
        fprintf(stderr, "Error: Solo se pueden crear enlaces a ficheros, no a directorios\n");
        bumount();
        return EXIT_FAILURE;
    }

    // Intentar crear el enlace
    int resultado = mi_link(ruta_original, ruta_enlace);
    if (resultado < 0) {
        fprintf(stderr, "Error al crear el enlace\n");
        bumount();
        return EXIT_FAILURE;
    }

    printf("Enlace creado correctamente.\n");

    bumount();
    return EXIT_SUCCESS;
}