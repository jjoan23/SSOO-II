#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ficheros_basico.h"

int leer_sf(const char *nombre_dispositivo) {
    // Abrir el dispositivo virtual
    if (bmount(nombre_dispositivo) == -1) {
        printf("Error al montar el dispositivo virtual\n");
        return -1;
    }

    // Leer el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        printf("Error al leer el superbloque\n");
        return -1;
    }

    // Mostrar la información del superbloque
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %u\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %u\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %u\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %u\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %u\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %u\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %u\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %u\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %u\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %u\n", SB.cantInodosLibres);
    printf("totBloques = %u\n", SB.totBloques);
    printf("totInodos = %u\n", SB.totInodos);

    // Mostrar el tamaño del struct inodo
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    // Recorrer la lista enlazada de inodos libres
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    for (unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        struct inodo inodos[BLOCKSIZE / INODOSIZE];
        if (bread(i, inodos) == -1) {
            printf("Error al leer el bloque de inodos\n");
            return -1;
        }
        for (unsigned int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            printf("%u ", inodos[j].punterosDirectos[0]);
            contInodos++;
        }
    }
    printf("\n");

    // Desmontar el dispositivo virtual
    if (bumount() == -1) {
        printf("Error al desmontar el dispositivo virtual\n");
        return -1;
    }

    return 0;
}