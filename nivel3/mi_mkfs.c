#include "ficheros_basico.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <nombre_fichero> <nbloques>\n", argv[0]);
        return -1;
    }

    char *nombre_fichero = argv[1];
    unsigned int nbloques = atoi(argv[2]);

    // Calcular el número de inodos
    unsigned int ninodos = nbloques / 4;

    // Crear el fichero
    if (bmount(nombre_fichero) == -1) {
        fprintf(stderr, "Error al montar el fichero\n");
        return -1;
    }

    // Inicializar el superbloque
    initSB(nbloques, ninodos);

    // Inicializar el mapa de bits
    initMB();

    // Inicializar el array de inodos
    initAI();

    // Desmontar el fichero
    bumount();

    return 0;
}