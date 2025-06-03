//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//mi_rm.c: programa que borra ficheros o directorios vacíos en un sistema de ficheros simulado.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directorios.h"

/*
 * Hemos decidido implementar el método mi_rm.c como un comando general para borrar tanto ficheros como directorios vacíos,
 * llamando a la función mi_unlink() de la capa de directorios.
 * No permite borrar el directorio raíz.
 * Si se trata de un directorio, mi_unlink() comprobará que esté vacío antes de borrarlo.
 * Opcionalmente, para borrar directorios no vacíos o de forma recursiva, se podría implementar un comando adicional (no incluido aquí).
 * Explicacion del archivo
 */

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: %s <disco> </ruta>\n", argv[0]);
        return FALLO;
    }

    char *nombre_disco = argv[1];
    char *ruta = argv[2];

    if (strcmp(ruta, "/") == 0) {
        fprintf(stderr, "Error: No se puede borrar el directorio raíz.\n");
        return FALLO;
    }

    if (bmount(nombre_disco) == -1) {
        perror("Error montando el disco");
        return FALLO;
    }

    int resultado = mi_unlink(ruta);
    if (resultado < 0) {
        bumount();
        return FALLO;
    }


    bumount();
    return EXITO;
}