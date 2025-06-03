//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//mi_chmod.c: programa que cambia los permisos de un fichero o directorio en un sistema de ficheros simulado.
#include "directorios.h"

// Declaración de la función mi_chmod (la implementarás después)
int mi_chmod(const char *camino, unsigned char permisos);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, RED"Sintaxis: %s <disco> <permisos> </ruta>\n"RESET, argv[0]);
        return FALLO;
    }

    // Montar el disco (asumiendo que existe una función bmount)
    if (bmount(argv[1]) < 0) {
        fprintf(stderr, "Error al montar el disco\n");
        return FALLO;
    }

    // Validar permisos
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Permisos no válidos. Deben estar entre 0 y 7.\n");
        bumount();
        return FALLO;
    }

    // Cambiar permisos
    int resultado = mi_chmod(argv[3], (unsigned char)permisos);
    if (resultado < 0) {
        fprintf(stderr, "Error al cambiar permisos: %s\n", strerror(errno));
        bumount();
        return FALLO;
    }

    bumount();
    return EXITO;
}