#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: %s <disco> <permisos> </ruta>\n", argv[0]);
        return 1;
    }

    // Comprobar permisos
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Error: permisos debe ser un número entre 0 y 7\n");
        return 1;
    }

    // Determinar si es directorio o fichero según si termina en '/'
    char *ruta = argv[3];
    size_t len = strlen(ruta);
    char tipo = (ruta[len - 1] == '/') ? 'd' : 'f';

    // Montar el disco
    if (bmount(argv[1]) < 0) {
        fprintf(stderr, "Error al montar el disco\n");
        return 1;
    }

    // Crear el directorio/fichero según corresponda
    int res = mi_creat(ruta, permisos);
    if (res < 0) {
        fprintf(stderr, "Error al crear el %s\n", (tipo == 'd') ? "directorio" : "fichero");
        bumount();
        return 1;
    }

    bumount();
    printf("%s creado correctamente: %s\n", (tipo == 'd') ? "Directorio" : "Fichero", ruta);
    return 0;
}