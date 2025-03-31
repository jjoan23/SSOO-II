#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ficheros.h" // Asegúrate de incluir el archivo de cabecera adecuado

#define NUM_OFFSETS 5

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <\"texto\"> <diferentes_inodos>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *nombre_dispositivo = argv[1];
    const char *texto = argv[2];
    int diferentes_inodos = atoi(argv[3]);
    int ninodo;
    int offsets[NUM_OFFSETS] = {9000, 209000, 30725000, 409605000, 480000000};
    int tam_texto = strlen(texto);
    char buf_original[tam_texto];
    memset(buf_original, 0, tam_texto);

    // Montar el dispositivo
    if (bmount(nombre_dispositivo) < 0) {
        perror("Error al montar el dispositivo");
        return EXIT_FAILURE;
    }

    // Escritura en los offsets
    for (int i = 0; i < NUM_OFFSETS; i++) {
        if (diferentes_inodos || i == 0) {
            // Reservar un nuevo inodo si diferentes_inodos = 1 o si es el primer inodo
            ninodo = reservar_inodo('f', 6);
            if (ninodo < 0) {
                perror("Error al reservar inodo");
                bumount();
                return EXIT_FAILURE;
            }
            printf("Inodo reservado: %d\n", ninodo);
        }
        // Escribir el texto en el offset correspondiente
        int escritos = mi_write_f(ninodo, texto, offsets[i], tam_texto);
        if (escritos < 0) {
            perror("Error al escribir en el fichero");
            bumount();
            return EXIT_FAILURE;
        }
        printf("Bytes escritos en el offset %d: %d\n", offsets[i], escritos);

        // Leer el contenido para comprobar
        memset(buf_original, 0, tam_texto);
        int leidos = mi_read_f(ninodo, buf_original, offsets[i], tam_texto);
        if (leidos < 0) {
            perror("Error al leer el fichero");
            bumount();
            return EXIT_FAILURE;
        }
        printf("Contenido leído en el offset %d: %s\n", offsets[i], buf_original);

        // Obtener información del inodo
        struct STAT stat;
        if (mi_stat_f(ninodo, &stat) < 0) {
            perror("Error al obtener información del inodo");
            bumount();
            return EXIT_FAILURE;
        }
        printf("Tamaño lógico del inodo: %d bytes\n", stat.tamEnBytesLog);
        printf("Bloques ocupados: %d\n", stat.numBloquesOcupados);
    }

    // Desmontar el dispositivo
    bumount();

    return EXIT_SUCCESS;
}