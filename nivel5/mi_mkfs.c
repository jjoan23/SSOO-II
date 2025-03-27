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

    // Crear el directorio raíz
    //CREC QUE ES RESERVAR INODO ('f', 6) I NO RESERVAR INODO ('d', 7) esta malament mirar nivel 3 tipo crec que s'error es a un aaltra banda pero aixo ho arregla
    if (reservar_inodo('f', 6) == -1) { // Cambiar 'd' a 'f' y 7 a 6
        fprintf(stderr, "Error al reservar el inodo para el directorio raíz\n");
        bumount();
        return -1;
    }

    // Desmontar el fichero
    bumount();

    return 0;
}