#include "ficheros.h"


int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;
    printf("offset: %d\n", offset);
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "No hay permisos de escritura\n");
        return FALLO;
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    unsigned int bytes_escritos = 0;

    if (primerBL == ultimoBL) {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes_escritos += nbytes;
    } else {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes_escritos += BLOCKSIZE - desp1;

        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            if (nbfisico == FALLO) return FALLO;

            if (bwrite(nbfisico, buf_original + bytes_escritos) == FALLO) return FALLO;
            bytes_escritos += BLOCKSIZE;
        }

        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque, buf_original + bytes_escritos, desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes_escritos += desp2 + 1;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    if (offset + nbytes > inodo.tamEnBytesLog) inodo.tamEnBytesLog = offset + nbytes;
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;

    return bytes_escritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Comprobar permisos de lectura
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, "No hay permisos de lectura\n");
        return FALLO;
    }

    // Si el offset está fuera del tamaño lógico, no hay nada que leer
    if (offset >= inodo.tamEnBytesLog) return 0;

    // Ajustar nbytes si excede el tamaño lógico del inodo
    if (offset + nbytes > inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    unsigned int bytes_leidos = 0;

    if (primerBL == ultimoBL) {
        // Caso: todo el rango de lectura está dentro de un único bloque lógico
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        bytes_leidos += nbytes;
    } else {
        // Caso: el rango de lectura abarca múltiples bloques lógicos

        // Leer el primer bloque lógico
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        bytes_leidos += BLOCKSIZE - desp1;

        // Leer los bloques intermedios
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 0);
            if (nbfisico != FALLO) {
                if (bread(nbfisico, buf_original + bytes_leidos) == FALLO) return FALLO;
            }
            bytes_leidos += BLOCKSIZE;
        }

        // Leer el último bloque lógico
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original + bytes_leidos, buf_bloque, desp2 + 1);
        }
        bytes_leidos += desp2 + 1;
    }

    // Actualizar el tiempo de acceso del inodo
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;

    return bytes_leidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;

    return EXITO;
}

/*int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        perror("Error leyendo el inodo");
        return FALLO;
    }

    // Comprobar permisos de escritura
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "No hay permisos de escritura\n");
        return FALLO;
    }

    // No se puede truncar más allá del tamaño lógico actual
    if (nbytes > inodo.tamEnBytesLog) {
        fprintf(stderr, "No se puede truncar más allá del tamaño lógico actual\n");
        return FALLO;
    }

    unsigned int primerBL;
    if (nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    // Liberar bloques a partir del primer bloque lógico a truncar
    int bloques_liberados = liberar_bloques_inodo(primerBL, &inodo);
    if (bloques_liberados == FALLO) {
        perror("Error liberando bloques del inodo");
        return FALLO;
    }

    // Actualizar el inodo
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= bloques_liberados;
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        perror("Error escribiendo el inodo");
        return FALLO;
    }

    return bloques_liberados;
}
    */
    
    int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {

        //Leemos inodo
        struct inodo inodo;
        if(leer_inodo(ninodo, &inodo) == -1) {
            return FALLO;
        }
    
        //Comprobamos que el inodo tenga permisos de escritura
        if((inodo.permisos & 2) != 2) {
            fprintf(stderr, "No tiene permiso de escritura.\n");
            return EXIT_FAILURE;
        }
    
        //Comprobamos que no se trunquen mas allá del tamaño de bytes lógicos
        if(nbytes > inodo.tamEnBytesLog) {
            return FALLO;
        }
    
        //Obtenemos el bloque lógico
        int primerBL;
        if(nbytes % BLOCKSIZE == 0) {
            primerBL = nbytes / BLOCKSIZE;
    
        } else {
            primerBL = (nbytes / BLOCKSIZE) + 1;
        }
    
        //Liberamos bloques
        int liberados = liberar_bloques_inodo(primerBL, &inodo);
    
        //Actulizamos información
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.tamEnBytesLog = nbytes;
        inodo.numBloquesOcupados -= liberados;
    
        //Guardamos inodo
        if(escribir_inodo(ninodo, &inodo) == -1) {
            return FALLO;
        }
    
        return liberados;
    }