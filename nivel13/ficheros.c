#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    unsigned int bytes_escritos = 0;
    int cambio_metadatos = 0;
    struct inodo inodo;

    // Leer inodo sin semáforo
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, RED"No hay permisos de escritura\n"RESET);
        return FALLO;
    }

    if (primerBL == ultimoBL) {
        // Un solo bloque
        mi_waitSem(); // traduce y reserva si hace falta
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem();
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes_escritos += nbytes;
    } else {
        // Primer bloque
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem();
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += BLOCKSIZE - desp1;

        // Bloques intermedios
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            mi_signalSem();
            if (nbfisico == FALLO) return FALLO;

            if (bwrite(nbfisico, buf_original + bytes_escritos) == FALLO) return FALLO;
            bytes_escritos += BLOCKSIZE;
        }

        // Último bloque
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        mi_signalSem();
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque, buf_original + bytes_escritos, desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += desp2 + 1;
    }

    // Sección crítica final para actualizar metadatos del inodo
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    if (offset + nbytes > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
        cambio_metadatos = 1;
    }

    inodo.mtime = time(NULL);
    if (cambio_metadatos) inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem(); // FIN sección crítica de metadatos

    return bytes_escritos;
}


int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    unsigned int bytes_leidos = 0;

    // Leer inodo sin semáforo (no lo modificamos aún)
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Comprobar permisos de lectura
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    // Si el offset está fuera del tamaño lógico, no hay nada que leer
    if (offset >= inodo.tamEnBytesLog) return 0;

    // Ajustar nbytes si excede el tamaño lógico del inodo
    if (offset + nbytes > inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    if (primerBL == ultimoBL) {
        // Todo en un solo bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        bytes_leidos += nbytes;
    } else {
        // Primer bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        bytes_leidos += BLOCKSIZE - desp1;

        // Intermedios
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 0);
            if (nbfisico != FALLO) {
                if (bread(nbfisico, buf_original + bytes_leidos) == FALLO) return FALLO;
            }
            bytes_leidos += BLOCKSIZE;
        }

        // Último bloque
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original + bytes_leidos, buf_bloque, desp2 + 1);
        }
        bytes_leidos += desp2 + 1;
    }

    // Sección crítica: solo para actualizar atime
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();

    return bytes_leidos;
}


int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;

    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->btime = inodo.btime;

    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    mi_waitSem(); // INICIO sección crítica

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem(); // FIN sección crítica
    return EXITO;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    

    //Leemos inodo
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Comprobamos que el inodo tenga permisos de escritura
    if((inodo.permisos & 2) != 2) {
        fprintf(stderr,RED "No tiene permiso de escritura.\n"RESET);
        return FALLO;
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