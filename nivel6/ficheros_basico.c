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

    // Cargar superbloque
    struct superbloque SB;
    if (bread(0, &SB) == FALLO) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }
/* 
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
 */
    

    // Actualizar superbloque
    int tamMetadatos = (SB.posUltimoBloqueAI - SB.posPrimerBloqueMB) + 2; // Incluye superbloque
    for (size_t i = 0; i < tamMetadatos; i++)
    {
        escribir_bit(i, 1);
    }
    
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

    // Read superblock
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }

    // Calculate the block number that contains the inode
    unsigned int nbloque = ninodo / (BLOCKSIZE / INODOSIZE);
    unsigned int pos_inodo = ninodo % (BLOCKSIZE / INODOSIZE);

    // Read the block containing the inode
    if (bread(SB.posPrimerBloqueAI + nbloque, inodos) == FALLO) {
        return FALLO;
    }

    // Copy the inode data
    *inodo = inodos[pos_inodo];

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

    // Initialize the inode
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = inodo.mtime = inodo.ctime = inodo.btime = time(NULL);
    
    //NO SE SI HI HA DE HAVER AIXO 
    inodo.numBloquesOcupados = SB.posUltimoBloqueMB;
    
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
        // This is the critical change - properly get the level 3 indirect pointer
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
        return nblogico; // Bloques directos
    } else if (nblogico < INDIRECTOS0) {
        return nblogico - DIRECTOS; // Bloques indirectos de nivel 0
    } else if (nblogico < INDIRECTOS1) { // Bloques indirectos de nivel 1
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) { // Bloques indirectos de nivel 2
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        } else if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS1) / NPUNTEROS) % NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS1) % NPUNTEROS;
        }
    }
    fprintf(stderr, "Error: Índice fuera de rango (nblogico=%u, nivel_punteros=%d)\n", nblogico, nivel_punteros);
    return FALLO; // En caso de error
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar) {
    unsigned int ptr = 0, ptr_ant = 0, salvar_inodo = 0, indice = 0;
    int nRangoBL, nivel_punteros;
    unsigned int buffer[NPUNTEROS];
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;
    while (nivel_punteros > 0) {
        indice = obtener_indice(nblogico, nivel_punteros);
        
        if (ptr == 0) {
            if (reservar == 0) return -1;
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            salvar_inodo = 1;

            if (nivel_punteros == nRangoBL) {
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
                printf("[traducir_bloque_inodo() → inodo.punterosIndirectos[%d] = %u (reservado BF %u para punteros_nivel%d)]\n",
                       nRangoBL - 1, ptr, ptr, nivel_punteros);
            } else {
                buffer[indice] = ptr;
                if (bwrite(ptr_ant, buffer) == FALLO) return FALLO;
                printf("[traducir_bloque_inodo() → punteros_nivel%d[%d] = %u (reservado BF %u para punteros_nivel%d)]\n",
                       nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
            }
            memset(buffer, 0, BLOCKSIZE);
        } 
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
            printf("[traducir_bloque_inodo() → inodo.punterosDirectos[%u] = %u (reservado BF %u para BL %u)]\n",
                   nblogico, ptr, ptr, nblogico);
        } else {
            buffer[indice] = ptr;
            if (bwrite(ptr_ant, buffer) == FALLO) return FALLO;
            printf("[traducir_bloque_inodo() → punteros_nivel1[%d] = %u (reservado BF %u para BL %u)]\n",
                   indice, ptr, ptr, nblogico);
        }
    }

    if (salvar_inodo) {
        if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    }

    return ptr;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo) {
    unsigned int nBL, ultimoBL, ptr = 0;
    int nRangoBL, nivel_punteros, liberados = 0;
    unsigned int bloques_punteros[3][NPUNTEROS];
    unsigned int bufAux_punteros[NPUNTEROS];
    unsigned int ptr_nivel[3];
    int indices[3];
    memset(bufAux_punteros, 0, BLOCKSIZE);

    if (inodo->tamEnBytesLog == 0) return liberados; // El fichero está vacío

    // Calcular el último bloque lógico ocupado
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    for (nBL = primerBL; nBL <= ultimoBL; nBL++) {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0) return FALLO;
        nivel_punteros = nRangoBL;

        while (ptr > 0 && nivel_punteros > 0) {
            int indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL) {
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == FALLO) return FALLO;
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0) {
            liberar_bloque(ptr);
            liberados++;
            if (nRangoBL == 0) {
                inodo->punterosDirectos[nBL] = 0;
            } else {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL) {
                    int indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {
                        liberar_bloque(ptr);
                        liberados++;
                        if (nivel_punteros == nRangoBL) {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    } else {
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == FALLO) return FALLO;
                        break;
                    }
                }
            }
        }
    }
    return liberados;
}

int liberar_inodo(unsigned int ninodo) {
    struct superbloque SB;
    struct inodo inodo;
    int bloques_liberados;

    // Leer el superbloque
    if (bread(0, &SB) == FALLO) {
        perror("Error leyendo el superbloque");
        return FALLO;
    }

    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        perror("Error leyendo el inodo");
        return FALLO;
    }

    // Liberar los bloques ocupados por el inodo
    bloques_liberados = liberar_bloques_inodo(0, &inodo);
    inodo.numBloquesOcupados -= bloques_liberados;

    // Marcar el inodo como libre
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    inodo.ctime = time(NULL);

    // Actualizar la lista enlazada de inodos libres
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;

    // Incrementar la cantidad de inodos libres
    SB.cantInodosLibres++;

    // Escribir el superbloque actualizado
    if (bwrite(0, &SB) == FALLO) {
        perror("Error escribiendo el superbloque");
        return FALLO;
    }

    // Escribir el inodo actualizado
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        perror("Error escribiendo el inodo");
        return FALLO;
    }

    return ninodo;
}