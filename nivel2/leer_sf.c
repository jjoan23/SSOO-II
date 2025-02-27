#include <stdio.h>
#include <stdlib.h>
#include "bloques.h"
#include "ficheros_basico.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Montar el dispositivo
    if (bmount(argv[1]) == -1) {
        perror("Error montando el dispositivo");
        return EXIT_FAILURE;
    }

    // Leer el superbloque
    struct superbloque SB;
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        bumount();
        return EXIT_FAILURE;
    }

    // Mostrar los datos del superbloque
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
    
    // Mostrar el tamaño de las estructuras
    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    // Recorrer la lista de inodos libres
    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int posInodo = SB.posPrimerInodoLibre;

    while (posInodo != UINT_MAX) {
        unsigned int bloqueAI = SB.posPrimerBloqueAI + (posInodo / (BLOCKSIZE / INODOSIZE));
        if (bread(bloqueAI, inodos) == -1) {
            perror("Error leyendo bloque de inodos");
            bumount();
            return EXIT_FAILURE;
        }
        printf("%u ", posInodo);
        posInodo = inodos[posInodo % (BLOCKSIZE / INODOSIZE)].punterosDirectos[0];
    }
    printf("-1\n");

    // Desmontar el dispositivo
    bumount();
    return EXIT_SUCCESS;
}
