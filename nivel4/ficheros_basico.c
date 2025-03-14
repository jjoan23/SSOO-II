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

    // Cargar superbloque
    struct superbloque SB;
    if (bread(0, &SB) == FALLO) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Corregimos el cálculo de los metadatos
    int tamMetadatos = (SB.posUltimoBloqueAI - SB.posPrimerBloqueMB) + 2; // Incluye superbloque
    int bitsMetadatos = tamMetadatos;
    int bytesMetadatos = bitsMetadatos / 8;
    int restoBits = bitsMetadatos % 8;

    int bloquesCompletos = bytesMetadatos / BLOCKSIZE;

    for (int i = 0; i < bloquesCompletos; i++) {
        memset(bufferMB, 255, BLOCKSIZE);
        if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO) {
            perror("Error escribiendo bloque completo del MB");
            return FALLO;
        }
    }

    // Último bloque parcial
    memset(bufferMB, 0, BLOCKSIZE);
    for (int i = 0; i < (bytesMetadatos % BLOCKSIZE); i++) {
        bufferMB[i] = 255;
    }

    if (restoBits > 0) {
        bufferMB[bytesMetadatos % BLOCKSIZE] |= (uint8_t)(255 << (8 - restoBits));
    }

    if (bwrite(SB.posPrimerBloqueMB + bloquesCompletos, bufferMB) == FALLO) {
        perror("Error escribiendo el último bloque parcial del MB");
        return FALLO;
    }

    // Debug: Verificar que el bit 3138 se marca correctamente
    printf("Marcando bloque %d en MB\n", 3138);
    escribir_bit(3138, 1); // Asegurar que se marque explícitamente

    // Actualizar superbloque
    SB.cantBloquesLibres -= tamMetadatos;
    if (bwrite(0, &SB) == FALLO) {
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


int escribir_bit(unsigned int nbloque, unsigned int bit) {
    struct superbloque SB;
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }
    
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == -1) {
        perror("Error leyendo el mapa de bits");
        return FALLO;
    }
    
    posbyte %= BLOCKSIZE;
    unsigned char mascara = 128 >> posbit;
    
    if (bit == 1) {
        bufferMB[posbyte] |= mascara;
    } else {
        bufferMB[posbyte] &= ~mascara;
    }
    
    if (bwrite(nbloqueabs, bufferMB) == -1) {
        perror("Error escribiendo en el mapa de bits");
        return FALLO;
    }
    
    return EXITO;
}

char leer_bit(unsigned int nbloque) {
    struct superbloque SB;
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }
    
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == -1) {
        perror("Error leyendo el mapa de bits");
        return FALLO;
    }
    
    posbyte %= BLOCKSIZE;
    unsigned char mascara = 128 >> posbit;
    
    return (bufferMB[posbyte] & mascara) ? 1 : 0;
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

int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leer el superbloque
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Calcular el número de bloque del array de inodos y su posición absoluta
    unsigned int nbloqueAI = ninodo / (BLOCKSIZE / INODOSIZE);
    unsigned int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    // Leer el bloque que contiene el inodo
    if (bread(nbloqueabs, inodos) == -1) {
        perror("Error leyendo el bloque del array de inodos");
        return FALLO;
    }

    // Calcular la posición del inodo dentro del bloque y escribirlo
    unsigned int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[posinodo] = *inodo;

    // Escribir el bloque modificado en el dispositivo
    if (bwrite(nbloqueabs, inodos) == -1) {
        perror("Error escribiendo el bloque del array de inodos");
        return FALLO;
    }

    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leer el superbloque
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Calcular el número de bloque del array de inodos y su posición absoluta
    unsigned int nbloqueAI = ninodo / (BLOCKSIZE / INODOSIZE);
    unsigned int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    // Leer el bloque que contiene el inodo
    if (bread(nbloqueabs, inodos) == -1) {
        perror("Error leyendo el bloque del array de inodos");
        return FALLO;
    }

    // Calcular la posición del inodo dentro del bloque y devolverlo
    unsigned int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[posinodo];

    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    struct superbloque SB;
    struct inodo inodo;

    // Leer el superbloque
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Comprobar si hay inodos libres
    if (SB.cantInodosLibres == 0) {
        return FALLO; // No hay inodos libres
    }

    // Guardar la posición del primer inodo libre
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;

    // Leer el inodo libre
    if (leer_inodo(posInodoReservado, &inodo) == -1) {
        perror("Error leyendo el inodo");
        return FALLO;
    }

    // Actualizar el superbloque para que apunte al siguiente inodo libre
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];

    // Inicializar el inodo reservado
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = inodo.mtime = inodo.ctime = time(NULL);
    inodo.numBloquesOcupados = 0;
    memset(inodo.punterosDirectos, 0, sizeof(inodo.punterosDirectos));
    memset(inodo.punterosIndirectos, 0, sizeof(inodo.punterosIndirectos));

    // Escribir el inodo actualizado
    if (escribir_inodo(posInodoReservado, &inodo) == -1) {
        perror("Error escribiendo el inodo");
        return FALLO;
    }

    // Decrementar la cantidad de inodos libres y actualizar el superbloque
    SB.cantInodosLibres--;
    if (bwrite(0, &SB) == -1) {
        perror("Error actualizando el superbloque");
        return FALLO;
    }

    return posInodoReservado;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  NIVELL 4  //////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {
    if (nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    } else if (nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } else if (nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    } else if (nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        fprintf(stderr, "Error: Bloque lógico fuera de rango\n");
        return FALLO;
    }
}


int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) {
        return nblogico;
    } else if (nblogico < INDIRECTOS0) {
        return nblogico - DIRECTOS;
    } else if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        } else if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return FALLO;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar) {
    unsigned int ptr = 0, ptr_ant = 0, salvar_inodo = 0;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    struct inodo inodo;

    leer_inodo(ninodo, &inodo);
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;

    while (nivel_punteros > 0) {
        if (ptr == 0) {
            if (reservar == 0) return -1;
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            salvar_inodo = 1;
            if (nivel_punteros == nRangoBL) {
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
            } else {
                buffer[indice] = ptr;
                bwrite(ptr_ant, buffer);
            }
            memset(buffer, 0, BLOCKSIZE);
        } else {
            bread(ptr, buffer);
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    if (ptr == 0) {
        if (reservar == 0) return -1;
        ptr = reservar_bloque();
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);
        salvar_inodo = 1;
        if (nRangoBL == 0) {
            inodo.punterosDirectos[nblogico] = ptr;
        } else {
            buffer[indice] = ptr;
            bwrite(ptr_ant, buffer);
        }
    }

    if (salvar_inodo) escribir_inodo(ninodo, &inodo);
    return ptr;
}
