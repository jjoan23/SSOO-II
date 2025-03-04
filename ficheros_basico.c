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
    SB.cantBloquesLibres = nbloques - 1;
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




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////    NIVELL 3 ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int escribir_bit(unsigned int nbloque, unsigned int bit);

char leer_bit(unsigned int nbloque){

}

int reservar_bloque() {
    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE); // Llenamos el buffer auxiliar con bits a 1

    // Leer el superbloque
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Comprobar si quedan bloques libres
    if (SB.cantBloquesLibres == 0) {
        return FALLO; // No hay bloques libres
    }

    int nbloqueMB;
    for (nbloqueMB = 0; nbloqueMB < (SB.posUltimoBloqueMB - SB.posPrimerBloqueMB + 1); nbloqueMB++) {
        if (bread(SB.posPrimerBloqueMB + nbloqueMB, bufferMB) == -1) {
            perror("Error leyendo el mapa de bits");
            return FALLO;
        }
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
            break; // Encontramos un bloque con algún 0
        }
    }

    int posbyte;
    for (posbyte = 0; posbyte < BLOCKSIZE; posbyte++) {
        if (bufferMB[posbyte] != 255) {
            break; // Encontramos un byte con algún 0
        }
    }

    int posbit = 0;
    unsigned char mascara = 128; // 10000000
    while (bufferMB[posbyte] & mascara) {
        bufferMB[posbyte] <<= 1;
        posbit++;
    }

    int nbloque = (nbloqueMB * BLOCKSIZE * 8) + (posbyte * 8) + posbit;

    // Marcar el bloque como ocupado
    if (escribir_bit(nbloque, 1) == -1) {
        perror("Error escribiendo bit en el mapa de bits");
        return FALLO;
    }

    // Actualizar el superbloque
    SB.cantBloquesLibres--;
    if (bwrite(0, &SB) == -1) {
        perror("Error actualizando el superbloque");
        return FALLO;
    }

    // Limpiar el bloque en la zona de datos
    unsigned char bufferCero[BLOCKSIZE];
    memset(bufferCero, 0, BLOCKSIZE);
    if (bwrite(nbloque, bufferCero) == -1) {
        perror("Error limpiando el bloque de datos");
        return FALLO;
    }

    return nbloque;
}


int liberar_bloque(unsigned int nbloque) {
    struct superbloque SB;

    // Leer el superbloque
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Marcar el bloque como libre en el mapa de bits
    if (escribir_bit(nbloque, 0) == -1) {
        perror("Error escribiendo bit en el mapa de bits");
        return FALLO;
    }

    // Incrementar el contador de bloques libres en el superbloque
    SB.cantBloquesLibres++;
    
    // Guardar el superbloque actualizado
    if (bwrite(0, &SB) == -1) {
        perror("Error actualizando el superbloque");
        return -1;
    }

    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo);

int leer_inodo(unsigned int ninodo, struct inodo *inodo);

int reservar_inodo(unsigned char tipo, unsigned char permisos);
