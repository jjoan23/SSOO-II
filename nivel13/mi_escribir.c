// AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
// mi_escribir.c: programa que escribe un texto en un fichero de un disco virtual
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "Sintaxis: %s <disco> </ruta_fichero> <texto> <offset>\n", argv[0]);
        return FALLO;
    }

    char *nombre_disco = argv[1];
    char *ruta_fichero = argv[2];
    char *texto = argv[3];
    int offset = atoi(argv[4]);
    int nbytes = strlen(texto);
    int escritos;

    printf("longitud texto: %d\n", nbytes);

    char buffer[nbytes];
    strcpy(buffer, texto);

    bmount(nombre_disco);

    if ((escritos = mi_write(ruta_fichero, buffer, offset, nbytes)) < 0)
    {
        return FALLO;
    }

    printf("Bytes escritos: %d\n", escritos);

    bumount();

    return EXITO;
}