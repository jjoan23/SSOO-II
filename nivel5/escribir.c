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
    // Remove unused variable if not needed
    // int diferentes_inodos = atoi(argv[3]); 
    int ninodo;
    int offsets[NUM_OFFSETS] = {9000, 209000, 30725000, 409605000, 480000000};

    // Montar el dispositivo
    if (bmount(nombre_dispositivo) < 0) {
        perror("Error al montar el dispositivo");
        return EXIT_FAILURE;
    }

    // Reservar el inodo
    ninodo = reservar_inodo('f', 6); // tipo fichero, permisos rw-
    if (ninodo == FALLO) {
        fprintf(stderr, "Error al reservar el inodo\n");
        bumount();
        return EXIT_FAILURE;
    }

    printf("longitud texto: %ld\n\n", strlen(texto));
    
    // Escribir en diferentes offsets
    for(int i = 0; i < NUM_OFFSETS; i++) {
        printf("\nNº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);
        
        // Escribir
        int bytes = mi_write_f(ninodo, texto, offsets[i], strlen(texto));
        if (bytes == FALLO) {
            fprintf(stderr, "Error al escribir\n");
            bumount();
            return EXIT_FAILURE;
        }
        
        // Obtener información del inodo
        struct STAT stat;
        if (mi_stat_f(ninodo, &stat) == FALLO) {
            fprintf(stderr, "Error al obtener información del inodo\n");
            bumount();
            return EXIT_FAILURE;
        }
        
        printf("Bytes escritos: %d\n", bytes);
        printf("stat.tamEnBytesLog=%u\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados=%u\n", stat.numBloquesOcupados);
        printf("\n");

        // Force a sync of the inode
        struct inodo temp_inodo;
        if (leer_inodo(ninodo, &temp_inodo) == FALLO) {
            return EXIT_FAILURE;
        }
        if (escribir_inodo(ninodo, &temp_inodo) == FALLO) {
            return EXIT_FAILURE;
        }
    }

    if (bumount() == FALLO) {
        fprintf(stderr, "Error al desmontar el dispositivo\n");
        return FALLO;
    }

    return EXIT_SUCCESS;
}