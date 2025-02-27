#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    int tamMB = (nbloques / 8) / BLOCKSIZE;
    if ((nbloques / 8) % BLOCKSIZE != 0) {
        tamMB++;
    }
    return tamMB;
}
int tamAI(unsigned int ninodos) {
    int tamAI = (ninodos * INODOSIZE) / BLOCKSIZE;
    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0) {
        tamAI++;
    }
    return tamAI;
}
int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;

    SB.posPrimerBloqueMB = 1;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    bwrite(0, &SB);

    return 0;
}
int initMB() {
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0, BLOCKSIZE); // Inicializamos el buffer a 0
    
    // Cargar superbloque para obtener la cantidad de bloques y los metadatos
    struct superbloque SB;
    if (bread(0, &SB) == FALLO)
    {
        perror("Error leyendo el superbloque");
        return FALLO;
    }
    
    int tamMetadatos = (SB.posUltimoBloqueAI - SB.posPrimerBloqueMB) + 1;
    int bitsMetadatos = tamMetadatos;
    int bytesMetadatos = bitsMetadatos / 8;
    int restoBits = bitsMetadatos % 8;
    
    // Inicializamos bloques completos del MB si ocupa más de un bloque
    int bloquesCompletos = bytesMetadatos / BLOCKSIZE;
    
    for (int i = 0; i < bloquesCompletos; i++)
    {
        memset(bufferMB, 255, BLOCKSIZE);
        if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO)
        {
            perror("Error escribiendo bloque completo del MB");
            return FALLO;
        }
    }
    
    // Último bloque parcial si hay bytes sobrantes
    memset(bufferMB, 0, BLOCKSIZE);
    
    for (int i = 0; i < (bytesMetadatos % BLOCKSIZE); i++)
    {
        bufferMB[i] = 255;
    }
    
    if (restoBits > 0)
    {
        bufferMB[bytesMetadatos % BLOCKSIZE] = (uint8_t)(255 << (8 - restoBits));
    }
    
    if (bwrite(SB.posPrimerBloqueMB + bloquesCompletos, bufferMB) == FALLO)
    {
        perror("Error escribiendo el último bloque parcial del MB");
        return FALLO;
    }
    
    // Actualizar el superbloque con los bloques libres restantes
    SB.cantBloquesLibres -= tamMetadatos;
    
    if (bwrite(0, &SB) == FALLO)
    {
        perror("Error actualizando el superbloque");
        return FALLO;
    }
    
    return EXITO;
}
int initAI() {
    struct superbloque SB;
    bread(0, &SB);

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;

    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);

        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            inodos[j].tipo = 'l';

            if (contInodos < SB.totInodos) {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        bwrite(i, inodos);
    }

    return 0;
}