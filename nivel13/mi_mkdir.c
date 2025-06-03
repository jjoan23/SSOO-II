//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//mi_mkdir.c: programa que crea un directorio o fichero en un sistema de ficheros
#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: %s <disco> <permisos> </ruta>\n"RESET, argv[0]);
        return FALLO;
    }

    // Comprobar permisos
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED"Error: modo inválido: <<9>>\n"RESET);
        return FALLO;
    }

    // Determinar si es directorio o fichero según si termina en '/'
    char *ruta = argv[3];

    // Montar el disco
    if (bmount(argv[1]) < 0) {
        fprintf(stderr, "Error al montar el disco\n");
        return FALLO;
    }

    // Crear el directorio/fichero según corresponda
    int res = mi_creat(ruta, permisos);
    if (res < 0) {
        bumount();
        return FALLO;
    }

    bumount();
    return EXITO;
}