// AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//ficheros.c : implementación de las funciones de acceso a ficheros del sistema de ficheros
#include "ficheros.h"

// Escribe datos en un inodo a partir de un offset dado, reservando bloques si es necesario
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

    // Leer el inodo para verificar permisos y acceder a sus campos
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Comprobar permisos de escritura
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, RED"No hay permisos de escritura\n"RESET);
        return FALLO;
    }

    if (primerBL == ultimoBL) {
        // Escritura en un único bloque
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem();
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes_escritos += nbytes;
    } else {
        // Escritura que abarca múltiples bloques

        // Primer bloque (parcial)
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem();
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += BLOCKSIZE - desp1;

        // Bloques intermedios (completos)
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            mi_signalSem();
            if (nbfisico == FALLO) return FALLO;

            if (bwrite(nbfisico, buf_original + bytes_escritos) == FALLO) return FALLO;
            bytes_escritos += BLOCKSIZE;
        }

        // Último bloque (parcial)
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        mi_signalSem();
        if (nbfisico == FALLO) return FALLO;

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque, buf_original + bytes_escritos, desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        bytes_escritos += desp2 + 1;
    }

    // Actualización de metadatos del inodo
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

    mi_signalSem();
    return bytes_escritos;
}

// Lee datos de un inodo desde un offset dado, hasta un número de bytes
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    unsigned int bytes_leidos = 0;

    // Leer inodo para comprobar permisos y tamaño lógico
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    if (offset >= inodo.tamEnBytesLog) return EXITO;

    if (offset + nbytes > inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    if (primerBL == ultimoBL) {
        // Lectura dentro del mismo bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        bytes_leidos += nbytes;
    } else {
        // Primer bloque (parcial)
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        bytes_leidos += BLOCKSIZE - desp1;

        // Bloques intermedios (completos)
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 0);
            if (nbfisico != FALLO) {
                if (bread(nbfisico, buf_original + bytes_leidos) == FALLO) return FALLO;
            }
            bytes_leidos += BLOCKSIZE;
        }

        // Último bloque (parcial)
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original + bytes_leidos, buf_bloque, desp2 + 1);
        }
        bytes_leidos += desp2 + 1;
    }

    // Actualizar atime (último acceso)
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

// Rellena una estructura STAT con los metadatos de un inodo
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Copiar campos del inodo a la estructura STAT
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

// Cambia los permisos de un inodo
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    mi_waitSem();
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    // Cambiar los permisos y actualizar ctime
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();
    return EXITO;
}

// Trunca un inodo a un número de bytes determinado
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    if((inodo.permisos & 2) != 2) {
        fprintf(stderr,RED "No tiene permiso de escritura.\n"RESET);
        return FALLO;
    }

    if(nbytes > inodo.tamEnBytesLog) {
        return FALLO;
    }

    // Calcular el primer bloque lógico a liberar
    int primerBL = (nbytes % BLOCKSIZE == 0) ? nbytes / BLOCKSIZE : (nbytes / BLOCKSIZE) + 1;

    // Liberar bloques desde primerBL
    int liberados = liberar_bloques_inodo(primerBL, &inodo);

    // Actualizar metadatos
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;

    if(escribir_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    return liberados;
}
