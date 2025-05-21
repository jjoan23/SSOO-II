#include "directorios.h"

#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) // máximo de 1000 entradas

int main(int argc, char **argv) {
    char *disco, *ruta;
    int extended = 0;
    char buffer[TAMBUFFER]; // Buffer para almacenar el listado

    // Validar argumentos
    if (argc < 3 || argc > 4) {
        fprintf(stderr, RED"Sintaxis: ./mi_ls <disco> </ruta> o\n"RESET);
        fprintf(stderr, RED"          ./mi_ls -l <disco> </ruta>\n"RESET);
        return FALLO;
    }

    // Comprobar si se solicita formato extendido
    if (strcmp(argv[1], "-l") == 0) {
        if (argc != 4) {
            fprintf(stderr, RED"Sintaxis con -l: ./mi_ls -l <disco> </ruta>\n"RESET);
            return FALLO;
        }
        extended = 1;
        disco = argv[2];
        ruta = argv[3];
    } else {
        disco = argv[1];
        ruta = argv[2];
    }

    // Montar el dispositivo
    if (bmount(disco) == -1) {
        fprintf(stderr, RED"Error en bmount\n"RESET);
        return FALLO;
    }

    // Determinar si es fichero o directorio según la ruta
    char tipo;
    if (strcmp(ruta, "/") == 0) {
        // Caso especial: directorio raíz
        tipo = 'd';
    } else {
        tipo = (ruta[strlen(ruta)-1] == '/') ? 'd' : 'f';
    }

    // Inicializar buffer
    memset(buffer, 0, sizeof(buffer));

    // Obtener listado
    int total = mi_dir(ruta, buffer, tipo);
    if (total < 0) {
        fprintf(stderr, RED"Error al listar el directorio/fichero\n"RESET);
        bumount();
        return FALLO;
    }

    // Mostrar resultados
    if (tipo == 'd') {
        if (total > 0) {
            printf("Total: %d\n", total);
            if (extended) {
                printf("Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
                printf("--------------------------------------------------------------------------------------------\n");
            }
        }
    } else if (extended) {
        printf("Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------------------------------------\n");
    }

    // Imprimir buffer
    printf("%s", buffer);

    // Desmontar dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error en bumount\n");
        return FALLO;
    }

    return EXITO;
}