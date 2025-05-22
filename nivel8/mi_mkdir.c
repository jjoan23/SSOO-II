#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: %s <disco> <permisos> </ruta>\n"RESET, argv[0]);
        return 1;
    }

    // Comprobar permisos
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED"Error: modo inválido: <<9>>\n"RESET);
        return 1;
    }

    // Determinar si es directorio o fichero según si termina en '/'
    char *ruta = argv[3];

    // Montar el disco
    if (bmount(argv[1]) < 0) {
        fprintf(stderr, "Error al montar el disco\n");
        return 1;
    }

    // Crear el directorio/fichero según corresponda
    int res = mi_creat(ruta, permisos);
    if (res < 0) {
        bumount();
        return 1;
    }

    bumount();
    return 0;
}