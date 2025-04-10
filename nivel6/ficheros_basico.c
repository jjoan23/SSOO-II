#include "ficheros_basico.h"

#define DEBUG4 1 //Debug del nivel 4
#define DEBUG5 0 //Debug del nivel 4
#define DEBUG6 1 //Debug del nivel 6

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

    //Buffer
    unsigned char buff[BLOCKSIZE];
    memset(buff, 0, BLOCKSIZE);  //Todo a 0

    // Leemos el superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    // Tamaño MB
    int tamMB = SB.posUltimoBloqueMB - SB.posPrimerBloqueMB;

    //Inicializamos el mapa de bits
    for(int i = SB.posPrimerBloqueMB; i <= tamMB + SB.posPrimerBloqueMB; i++) {
        if(bwrite(i, buff) == -1) {
            return FALLO;
        }
    }

    // Ponemos a 1 los bits correspondientes
    for(unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++) {

        reservar_bloque();
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
    
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;
    mascara >>= posbit;
    
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
    
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;
    mascara >>= posbit;
    
    if (bufferMB[posbyte] & mascara) {
        return 1; // El bit está ocupado
    } else {
        return 0; // El bit está libre
    }
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
        fprintf(stderr, "Error: No hay bloques libres disponibles\n");
        return FALLO;
    }

    int nbloqueMB;
    for (nbloqueMB = 0; nbloqueMB < (SB.posUltimoBloqueMB - SB.posPrimerBloqueMB + 1); nbloqueMB++) {
        if (bread(SB.posPrimerBloqueMB + nbloqueMB, bufferMB) == -1) {
            perror("Error leyendo el mapa de bits");
            return FALLO;
        }
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
            break; // Encontramos un bloque con algún bit a 0
        }
    }

    int posbyte;
    for (posbyte = 0; posbyte < BLOCKSIZE; posbyte++) {
        if (bufferMB[posbyte] != 255) {
            break; // Encontramos un byte con algún bit a 0
        }
    }

    int posbit = 0;
    unsigned char mascara = 128; // 10000000
    while (bufferMB[posbyte] & mascara) {
        mascara >>= 1;
        posbit++;
    }

    int nbloque = (nbloqueMB * BLOCKSIZE * 8) + (posbyte * 8) + posbit;

    // Marcar el bloque como ocupado
    if (escribir_bit(nbloque, 1) == -1) {
        perror("Error escribiendo bit en el mapa de bits");
        return FALLO;
    }

    // Actualizar el contador de bloques libres en el superbloque
    SB.cantBloquesLibres--;
    if (bwrite(0, &SB) == -1) {
        perror("Error actualizando el superbloque");
        return FALLO;
    }

    // Limpiar el bloque en la zona de datos
    unsigned char bufferCero[BLOCKSIZE];
    memset(bufferCero, 0, BLOCKSIZE);
    if (bwrite(SB.posPrimerBloqueDatos + nbloque, bufferCero) == -1) {
        perror("Error limpiando el bloque de datos");
        return FALLO;
    }

    return nbloque;
}


int liberar_bloque(unsigned int nbloque) {

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }
    
    //Ponemos a 0 el bit del MB
    if(escribir_bit(nbloque, 0) == -1) {
        return FALLO;
    }

    //Incrementamos la cantidad de bloques libres
    SB.cantBloquesLibres++;

    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }

    //Devolver numero de bloque
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

    
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }

    
    unsigned int nbloque = ninodo / (BLOCKSIZE / INODOSIZE);
    unsigned int pos_inodo = ninodo % (BLOCKSIZE / INODOSIZE);

    if (bread(SB.posPrimerBloqueAI + nbloque, inodos) == FALLO) {
        return FALLO;
    }

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
    inodo.numBloquesOcupados=0;
    inodo.atime = inodo.mtime = inodo.ctime = inodo.btime = time(NULL);
    
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

    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Obtener el rango del bloque lógico
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;

    while (nivel_punteros > 0) {
        indice = obtener_indice(nblogico, nivel_punteros);

        if (ptr == 0) {
            if (reservar == 0) return -1; // No se puede traducir si no se permite reservar
            ptr = reservar_bloque();
            if (ptr == FALLO) return FALLO;

            // Incrementar el número de bloques ocupados
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            salvar_inodo = 1;

            if (nivel_punteros == nRangoBL) {
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
                #if DEBUG4
                    printf(GRAY"[traducir_bloque_inodo() → inodo.punterosIndirectos[%d] = %u (reservado BF %u para punteros_nivel%d)]\n"RESET,
                       nRangoBL - 1, ptr, ptr, nivel_punteros);
                #endif
            } else {
                memset(buffer, 0, BLOCKSIZE);
                buffer[indice] = ptr;
                if (bwrite(ptr_ant, buffer) == FALLO) return FALLO;
                #if DEBUG4
                    printf(GRAY"[traducir_bloque_inodo() → punteros_nivel%d[%d] = %u (reservado BF %u para punteros_nivel%d)]\n"RESET,
                       nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
                #endif
            }
        }

        // Leer el bloque de punteros si existe
        if (bread(ptr, buffer) == FALLO) return FALLO;

        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    if (ptr == 0) {
        if (reservar == 0) return -1; // No se puede traducir si no se permite reservar
        ptr = reservar_bloque();
        if (ptr == FALLO) return FALLO;

        // Incrementar el número de bloques ocupados
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);
        salvar_inodo = 1;

        if (nRangoBL == 0) {
            inodo.punterosDirectos[nblogico] = ptr;
            #if DEBUG4
                printf(GRAY"[traducir_bloque_inodo() → inodo.punterosDirectos[%u] = %u (reservado BF %u para BL %u)]\n"RESET,
                   nblogico, ptr, ptr, nblogico);
            #endif
        } else {
            memset(buffer, 0, BLOCKSIZE);
            buffer[indice] = ptr;
            if (bwrite(ptr_ant, buffer) == FALLO) return FALLO;
            #if DEBUG4
                printf(GRAY"[traducir_bloque_inodo() → punteros_nivel1[%d] = %u (reservado BF %u para BL %u)]\n"RESET,
                   indice, ptr, ptr, nblogico);
            #endif
        }
    }

    // Guardar el inodo si se realizaron cambios
    if (salvar_inodo) {
        if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    }

    return ptr;
}


int liberar_inodo(unsigned int ninodo) {

    //Leemos inodo
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Liberamos todos los bloques del inodo
    int bloquesLiberados = liberar_bloques_inodo(0, &inodo);
    inodo.numBloquesOcupados -= bloquesLiberados;

    //Marcamos el inodo como tipo libre y tamEnBytesLog=0
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    //Incluimos el inodo que queremos liberar en la lista de inodos libres
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;

    //Incrementamos la cantidad de inodos libres
    SB.cantInodosLibres++;

    //Escribimos el inodo actualizado
    if(escribir_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Guardamos el superbloque
    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }


    //Devolvemos el nº del inodo liberado
    return ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo) {

    //Declaraciones
    unsigned int nivel_punteros, indice, ptr = 0, nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS];  //array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE]; //para llenar de 0s y comparar
    int ptr_nivel[3];  //punteros a bloques de punteros de cada nivel
    int indices[3];  //indices de cada nivel
    int liberados = 0;  // nº de bloques liberados

    if(inodo->tamEnBytesLog == 0) { //el fichero está vacío
        return 0;
    }

    if((inodo->tamEnBytesLog % BLOCKSIZE) == 0) {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE) - 1;

    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);

    #if DEBUG6
        printf(CYAN"[liberar_bloques_inodo()→ primerBL: %d, ultimoBL: %d]\n"RESET, primerBL, ultimoBL);
    #endif
    
    for(nBL = primerBL; nBL <= ultimoBL; nBL++) {  //recorrido BLs

        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); //0:D, 1:I0, 2:I1, 3:I2
        if(nRangoBL < 0) {
            return FALLO;
        }
        nivel_punteros = nRangoBL; //el nivel_punteros +alto cuelga del inodo

        while(ptr > 0 && nivel_punteros > 0) {  //cuelgan bloques de punteros

            indice = obtener_indice(nBL, nivel_punteros);
            if(indice == 0 || nBL == primerBL) {
                //solo hay que leer del dispositivo si no está ya cargado previamente en un buffer    
                bread(ptr, bloques_punteros[nivel_punteros - 1]);
            }             

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if(ptr > 0) { //si existe bloque de datos
            liberar_bloque(ptr);
            liberados++;

            if(nRangoBL == 0) { //es un puntero Directo
                inodo->punterosDirectos[nBL] = 0;

                #if DEBUG6
                    printf(GRAY"[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n"RESET, ptr, nBL);
                #endif

            } else {
                nivel_punteros = 1;

                #if DEBUG6
                    printf(GRAY"[liberar_bloques_inodo()→ liberado BF %i de datos para BL: %i]\n"RESET, ptr, nBL);
                #endif

                while(nivel_punteros <= nRangoBL) {

                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel [nivel_punteros - 1]; 

                    if(memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {  
                        //No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        if(liberar_bloque(ptr) == -1) {
                            return FALLO;
                        }
                        liberados++;

                        #if DEBUG6
                            printf(GRAY"[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n"RESET, ptr, nivel_punteros, nBL);
                        #endif

                        //Incluir mejora saltando los bloques que no sea necesario explorar al eliminar bloque de punteros
                        if(nivel_punteros == nRangoBL) {
                            inodo->punterosDirectos[nRangoBL -1] = 0;

                            switch(nRangoBL) {
                                case 1:
                                    nBL = INDIRECTOS0;
                                    break;
                                
                                case 2:
                                    nBL = INDIRECTOS1;
                                    break;

                                case 3:
                                    nBL = INDIRECTOS2;
                                    break;
                            }
                        }

                        nivel_punteros++;
                        
                    } else {  //escribimos en el dispositivo el bloque de punteros modificado
                        if(bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == -1) {
                            return FALLO;
                        }
                        // hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }

    #if DEBUG6
        printf("[liberar_bloques_inodo()→ total bloques liberados: %d]\n", liberados);
    #endif

    return liberados;
}

