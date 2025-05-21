#include "directorios.h"
#include <time.h>

int main(int argc, char **argv) {
    // Validar argumentos
    if (argc != 3) {
        fprintf(stderr, RED"Sintaxis: ./mi_stat <disco> </ruta>\n"RESET);
        return FALLO;
    }

    const char *disco = argv[1];
    const char *ruta = argv[2];
    struct STAT stat;
    int error;

    // Montar el dispositivo
    if (bmount(disco) == -1) {
        fprintf(stderr, RED"Error en bmount\n"RESET);
        return FALLO;
    }

    // Verificar si la ruta es válida
    if (ruta[strlen(ruta) - 1] == '/' && strlen(ruta) > 1) {
        // Es un directorio
        error = mi_stat(ruta, &stat);
        if (error < 0) {
            fprintf(stderr, RED"Error: No existe el directorio %s\n"RESET, ruta);
            bumount();
            return FALLO;
        }
    } else {
        // Es un fichero
        error = mi_stat(ruta, &stat);
        if (error < 0) {
            fprintf(stderr, RED"Error: No existe el fichero %s\n"RESET, ruta);
            bumount();
            return FALLO;
        }
    }

    // Mostrar información formateada
    printf("Nº de inodo: %d\n", error); // El nº de inodo lo devuelve mi_stat
    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %d\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("nlinks: %u\n", stat.nlinks);
    printf("tamEnBytesLog: %u\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", stat.numBloquesOcupados);

    // Desmontar dispositivo
    if (bumount() == -1) {
        fprintf(stderr, RED"Error en bumount\n"RESET);
        return FALLO;
    }

    return EXITO;
}

