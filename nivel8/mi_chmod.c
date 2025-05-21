
#include "directorios.h"

// Declaración de la función mi_chmod (la implementarás después)
int mi_chmod(const char *camino, unsigned char permisos);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, RED"Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n"RESET);
        return 1;
    }

    // Montar el disco (asumiendo que existe una función bmount)
    if (bmount(argv[1]) < 0) {
        fprintf(stderr, "Error al montar el disco\n");
        return 1;
    }

    // Validar permisos
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Permisos no válidos. Deben estar entre 0 y 7.\n");
        bumount();
        return 1;
    }

    // Cambiar permisos
    int resultado = mi_chmod(argv[3], (unsigned char)permisos);
    if (resultado < 0) {
        fprintf(stderr, "Error al cambiar permisos: %s\n", strerror(errno));
        bumount();
        return 1;
    }

    bumount();
    return 0;
}