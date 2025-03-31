#include "ficheros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: %s <nombre_dispositivo> <ninodo> <nbytes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *nombre_dispositivo = argv[1];
    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == FALLO) {
        perror("Error montando el dispositivo virtual");
        return EXIT_FAILURE;
    }

    int bloques_liberados;
    if (nbytes == 0) {
        // Llamar a liberar_inodo()
        bloques_liberados = liberar_inodo(ninodo);
        if (bloques_liberados == FALLO) {
            perror("Error liberando el inodo");
            bumount();
            return EXIT_FAILURE;
        }
        printf("Inodo %u liberado. Bloques liberados: %d\n", ninodo, bloques_liberados);
    } else {
        // Llamar a mi_truncar_f()
        bloques_liberados = mi_truncar_f(ninodo, nbytes);
        if (bloques_liberados == FALLO) {
            perror("Error truncando el inodo");
            bumount();
            return EXIT_FAILURE;
        }
        printf("Inodo %u truncado a %u bytes. Bloques liberados: %d\n", ninodo, nbytes, bloques_liberados);
    }

    // Obtener información del inodo para verificar
    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == FALLO) {
        perror("Error obteniendo información del inodo");
        bumount();
        return EXIT_FAILURE;
    }

    printf("Estado del inodo %u:\n", ninodo);
    printf(" - Tamaño en bytes lógicos: %u\n", stat.tamEnBytesLog);
    printf(" - Número de bloques ocupados: %u\n", stat.numBloquesOcupados);

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO) {
        perror("Error desmontando el dispositivo virtual");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}