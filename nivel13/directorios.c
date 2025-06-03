//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//directorios.c: Implementación de funciones para manejar directorios y ficheros
#include "directorios.h"
#define DEBUGN7 0
#define DEBUGN9 0

struct UltimaEntrada UltimaEntradaEscritura;
struct UltimaEntrada UltimaEntradaLectura;


// Extrae el primer componente (directorio o fichero) de una ruta
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    const char bar = '/';
    char *dir;
    if (camino[0] != bar){
        return ERROR_CAMINO_INCORRECTO; //El primer carácter no es el separador '/' o camino vacío
    }

    camino++;
    dir = strchr(camino, bar);
    if (dir != NULL){
        strncpy(inicial,camino,(dir-camino));
        camino = dir; 
        strcpy(final,camino);
        *tipo = 'd';
    }
    else{
        strcpy(inicial,camino);
        strcpy(final, "");
        *tipo = 'f';
    }
    return EXITO;
}


int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[TAMNOMBRE];
    char resto[strlen(camino_parcial) + 1];
    char tipo;
    unsigned int num_entrada = 0, num_entradas;
    int error;

    if (strcmp(camino_parcial, "/") == 0) {
        return EXITO;  
    }
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(resto, 0, strlen(camino_parcial)+1);
    if ((error = extraer_camino(camino_parcial, inicial, resto, &tipo)) < 0) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUGN7
        fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" 
                RESET, inicial, resto, reservar);
    #endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1) {
        return FALLO;
    }

    if (!(inodo_dir.permisos & 4)) {
        return ERROR_PERMISO_LECTURA;
    }
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    num_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    while (num_entrada < num_entradas) {
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada * sizeof(struct entrada), 
                     sizeof(struct entrada)) < 0) {
            return FALLO; 
        }

        if (strcmp(inicial, entrada.nombre) == 0) {
            if (resto[0] == '\0' || strcmp(resto, "/") == 0) {
                if (reservar) {
                    return ERROR_ENTRADA_YA_EXISTENTE;
                }
                *p_inodo = entrada.ninodo;
                *p_entrada = num_entrada;
                return EXITO;
            }

            struct inodo inodo_entrada;
            if (leer_inodo(entrada.ninodo, &inodo_entrada) == -1) {
                return FALLO;
            }

            if (inodo_entrada.tipo != 'd') {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            *p_inodo_dir = entrada.ninodo;
            return buscar_entrada(resto, p_inodo_dir, p_inodo, p_entrada, 
                                reservar, permisos);
        }
        num_entrada++;
    }

    if (!reservar) {
        return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
    }

    if (resto[0] != '\0' && strcmp(resto, "/") != 0) {
        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
    }

    if (!(inodo_dir.permisos & 2)) {
        return ERROR_PERMISO_ESCRITURA;
    }

    struct entrada nueva;
    strcpy(nueva.nombre, inicial);
    nueva.ninodo = reservar_inodo((tipo == 'd') ? 'd' : 'f', permisos);
    
    if (nueva.ninodo == -1) {
        return FALLO;
    }

    #if DEBUGN7
        fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" 
                RESET, nueva.ninodo, (tipo == 'd') ? 'd' : 'f', permisos, inicial);
    #endif

    if (mi_write_f(*p_inodo_dir, &nueva, num_entrada * sizeof(struct entrada), 
                   sizeof(struct entrada)) < 0) {
        return FALLO;
    }

    #if DEBUGN7
        fprintf(stderr, GRAY "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, inicial, nueva.ninodo);
    #endif

    if (resto[0] == '\0' || strcmp(resto, "/") == 0) {
        *p_inodo = nueva.ninodo;
        *p_entrada = num_entrada;
        return EXITO;
    }

    *p_inodo_dir = nueva.ninodo;
    return buscar_entrada(resto, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

// Muestra mensajes de error específicos para la función buscar_entrada
void mostrar_error_buscar_entrada(int error) {
    switch (error){
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}

// Crea un nuevo archivo o directorio con los permisos especificados
int mi_creat(const char *camino, unsigned char permisos) {
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    if (strcmp(camino, "/") == 0) {
        mi_signalSem();
        return ERROR_ENTRADA_YA_EXISTENTE;
    }

    
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    

    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    mi_signalSem();
    return EXITO;
}

// Lista el contenido de un directorio
int mi_dir(const char *camino, char *buffer, char tipo, int extendido) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    if (strcmp(camino, "/") == 0) {
        p_inodo = 0;  
    } else {
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return error;
        }
    }

    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == -1) {
        return FALLO;
    }

    if (inodo.tipo != tipo) {
        fprintf(stderr, RED"Error: La sintaxis no concuerda con el tipo\n"RESET);
        return FALLO;
    }

    struct entrada entrada;
    struct inodo inodo_entrada;
    struct tm *tm;
    char tmp[100];
    int num_entrada = 0;
    int num_entradas_total = inodo.tamEnBytesLog / sizeof(struct entrada);

    while (num_entrada < num_entradas_total) {
        if (mi_read_f(p_inodo, &entrada, num_entrada * sizeof(struct entrada), 
                     sizeof(struct entrada)) < 0) {
            return FALLO;
        }

        if (leer_inodo(entrada.ninodo, &inodo_entrada) == -1) {
            return FALLO;
        }

        if (extendido) {
            sprintf(tmp, "%c\t", inodo_entrada.tipo);
            strcat(buffer, tmp);

            sprintf(tmp, "%c%c%c\t",
                (inodo_entrada.permisos & 4) ? 'r' : '-',
                (inodo_entrada.permisos & 2) ? 'w' : '-',
                (inodo_entrada.permisos & 1) ? 'x' : '-');
            strcat(buffer, tmp);

            tm = localtime(&inodo_entrada.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);

            sprintf(tmp, "%d\t", inodo_entrada.tamEnBytesLog);
            strcat(buffer, tmp);
        }

        sprintf(tmp, "%s\n", entrada.nombre);
        strcat(buffer, tmp);

        num_entrada++;
    }

    return num_entrada;
}

// Modifica los permisos de un archivo o directorio
int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    return mi_chmod_f(p_inodo, permisos);
}

// Obtiene información del inodo de un archivo o directorio
int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    if (mi_stat_f(p_inodo, p_stat) < 0) {
        fprintf(stderr, "mi_stat()--> Error al mostrar el nº de inodo");
        return FALLO;
    }

    return p_inodo;
}

// Escribe datos en un archivo en la posición especificada
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada=0;
    unsigned int p_inodo=0;
    int error;
    struct inodo inodo;

    if (strcmp(camino, UltimaEntradaEscritura.camino) == 0) {
        p_inodo = UltimaEntradaEscritura.p_inodo;
        #if DEBUGN9
            fprintf(stderr, CYAN "[mi_write() --> Utilizamos la caché de escritura en vez de buscar_entrada()]\n" RESET);
        #endif
    } else {
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return error;
        }

        #if DEBUGN9
            fprintf(stderr, ORANGE "[mi_write() --> Actualizamos la caché de escritura]\n" RESET);
        #endif
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }

    if (leer_inodo(p_inodo, &inodo) == -1) {
        return FALLO;
    }

    if (inodo.tipo == 'd') {
        fprintf(stderr, "Error: No se puede escribir en un directorio\n");
        return FALLO;
    }

    int bytes_escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    if (bytes_escritos < 0) {
        fprintf(stderr, "Error: No se han podido escribir bytes\n");
        return FALLO;
    }

    return bytes_escritos;
}

// Lee datos de un archivo desde la posición especificada
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    static struct UltimaEntrada UltimaEntradaLectura = {"", -1};
    int p_inodo=0;
    unsigned int p_inodo_dir = 0, p_entrada = 0;
    int error;

    if (strcmp(UltimaEntradaLectura.camino, camino) == 0) {
        p_inodo = UltimaEntradaLectura.p_inodo;
        #if DEBUGN9
            fprintf(stderr, CYAN "[mi_read() --> Using read cache instead of buscar_entrada()]\n" RESET);
        #endif
    } else {
        error = buscar_entrada(camino, &p_inodo_dir, (unsigned int *)&p_inodo, 
                             &p_entrada, 0, 0);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }

        #if DEBUGN9
            fprintf(stderr, ORANGE "[mi_read() --> Updating read cache]\n" RESET);
        #endif

        strncpy(UltimaEntradaLectura.camino, camino, 
                sizeof(UltimaEntradaLectura.camino) - 1);
        UltimaEntradaLectura.camino[sizeof(UltimaEntradaLectura.camino) - 1] = '\0';
        UltimaEntradaLectura.p_inodo = p_inodo;
    }

    return mi_read_f(p_inodo, buf, offset, nbytes);
}

// Crea un enlace entre dos archivos (link)
int mi_link(const char *camino1, const char *camino2) {
    mi_waitSem();
    
    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0;
    unsigned int p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;
    int error;
    struct inodo inodo1;
    struct entrada entrada2;

    error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }

    if (leer_inodo(p_inodo1, &inodo1) == -1) {
        mi_signalSem();
        return FALLO;
    }

    if (inodo1.tipo != 'f') {
        fprintf(stderr, "Error: Solo se pueden enlazar ficheros.\n");
        mi_signalSem();
        return FALLO;
    }

    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }

    if (mi_read_f(p_inodo_dir2, &entrada2, 
                  p_entrada2 * sizeof(struct entrada), 
                  sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    entrada2.ninodo = p_inodo1;

    if (mi_write_f(p_inodo_dir2, &entrada2, 
                   p_entrada2 * sizeof(struct entrada), 
                   sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    if (liberar_inodo(p_inodo2) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    if (leer_inodo(p_inodo1, &inodo1) == -1) {
        mi_signalSem();
        return FALLO;
    }

    inodo1.nlinks++;
    inodo1.ctime = time(NULL);
    
    if (escribir_inodo(p_inodo1, &inodo1) == -1) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();
    return EXITO;
}

// Elimina un archivo o directorio vacío
int mi_unlink(const char *camino) {
    mi_waitSem();
    
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;
    struct inodo inodo, inodo_dir;
    struct entrada entrada_ult;
    
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }

    if (leer_inodo(p_inodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }

    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) {
        fprintf(stderr, RED"Error: El directorio %s no está vacío.\n"RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    if (leer_inodo(p_inodo_dir, &inodo_dir) == -1) {
        mi_signalSem();
        return FALLO;
    }
    
    int num_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    if ((int)p_entrada != num_entradas - 1) {
        if (mi_read_f(p_inodo_dir, &entrada_ult, 
            (num_entradas - 1) * sizeof(struct entrada), 
            sizeof(struct entrada)) < 0) {
            mi_signalSem();
            return FALLO;
        }
        
        if (mi_write_f(p_inodo_dir, &entrada_ult,
            p_entrada * sizeof(struct entrada),
            sizeof(struct entrada)) < 0) {
            mi_signalSem();
            return FALLO;
        }
    }

    if (mi_truncar_f(p_inodo_dir, 
        (num_entradas - 1) * sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    if (leer_inodo(p_inodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }
    inodo.nlinks--;

    if (inodo.nlinks == 0) {
        if (liberar_inodo(p_inodo) == FALLO) {
            mi_signalSem();
            return FALLO;
        }
    } else {
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, &inodo) == -1) {
            mi_signalSem();
            return FALLO;
        }
    }

    mi_signalSem();
    return EXITO;
}