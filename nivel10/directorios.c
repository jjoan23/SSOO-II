#include "directorios.h"
#define DEBUGN7 0
#define DEBUGN9 1

struct UltimaEntrada UltimaEntradaEscritura;
struct UltimaEntrada UltimaEntradaLectura;


//Obtiene primer directorio o fichero del path
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
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


int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir,
    unsigned int *p_inodo, unsigned int *p_entrada,
    char reservar, unsigned char permisos) {

struct entrada entrada;
struct inodo inodo_dir;
char inicial[TAMNOMBRE];
char resto[strlen(camino_parcial)];
char tipo;
unsigned int num_entrada = 0, num_entradas;
int error;

if (strcmp(camino_parcial, "/") == 0) {
    return 0;
}

if ((error = extraer_camino(camino_parcial, inicial, resto, &tipo)) < 0)
return ERROR_CAMINO_INCORRECTO;
#if DEBUGN7
    fprintf(stderr, GREY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n"RESET, inicial, resto, reservar);
#endif
if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1)
return -1;

if (!(inodo_dir.permisos & 4)) return ERROR_PERMISO_LECTURA;

num_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

// Buscar entrada
while (num_entrada < num_entradas) {
if (mi_read_f(*p_inodo_dir, &entrada, num_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
return -1;

if (strcmp(inicial, entrada.nombre) == 0) {
// Entrada encontrada
if (resto[0] == '\0' || strcmp(resto, "/") == 0) {
 if (reservar) {
     printf("Error: El archivo ya existe.\n");
     return ERROR_ENTRADA_YA_EXISTENTE;
 }
 *p_inodo = entrada.ninodo;
 *p_entrada = num_entrada;
 return 0;
}

// Si hay más camino, verificar que es un directorio
struct inodo inodo_entrada;
if (leer_inodo(entrada.ninodo, &inodo_entrada) == -1)
 return -1;

if (inodo_entrada.tipo != 'd') return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

*p_inodo_dir = entrada.ninodo;
return buscar_entrada(resto, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

num_entrada++;
}

// Entrada no encontrada
if (!reservar) return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

// Si hay más camino pero no existe esta entrada: error de directorio intermedio
if (resto[0] != '\0' && strcmp(resto, "/") != 0)
return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;

if (!(inodo_dir.permisos & 2)) return ERROR_PERMISO_ESCRITURA;

// Crear nueva entrada
struct entrada nueva;
strcpy(nueva.nombre, inicial);
nueva.ninodo = reservar_inodo((tipo == 'd') ? 'd' : 'f', permisos);
if (nueva.ninodo == -1) return -1;

#if DEBUGN7
    fprintf(stderr, GREY"[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n"RESET, nueva.ninodo, (tipo == 'd') ? 'd' : 'f', permisos, inicial);
#endif

if (mi_write_f(*p_inodo_dir, &nueva, num_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
return -1;
#if DEBUGN7
    fprintf(stderr, GREY"[buscar_entrada()→ creada entrada: %s, %d]\n"RESET, inicial, nueva.ninodo);
#endif

if (resto[0] == '\0' || strcmp(resto, "/") == 0) {
*p_inodo = nueva.ninodo;
*p_entrada = num_entrada;
return 0;
}

// Continuar recursión
*p_inodo_dir = nueva.ninodo;


return buscar_entrada(resto, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

void mostrar_error_buscar_entrada(int error){
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

int mi_creat(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0; // Directorio raíz 
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    if (!strcmp(camino, "/")) { // Si es el directorio raíz
        return ERROR_ENTRADA_YA_EXISTENTE;
    }

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    return EXIT_SUCCESS;
}
int mi_dir(const char *camino, char *buffer, char tipo) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    // Caso especial para el directorio raíz
    if (strcmp(camino, "/") == 0) {
        p_inodo = 0; // El inodo raíz siempre es 0
    } else {
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return error;
        }
    }

    // Leemos el inodo
    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == -1) {
        return -1;
    }

    // Verificar que es del tipo solicitado
    if (inodo.tipo != tipo) {
        fprintf(stderr, RED"Error: La sintaxis no concuerda con el tipo\n"RESET);
        return -1;
    }

    struct entrada entrada;
    struct inodo inodo_entrada;
    struct tm *tm;
    char tmp[100];
    int num_entrada = 0;
    int num_entradas_total = inodo.tamEnBytesLog / sizeof(struct entrada);

    // Leer cada entrada del directorio
    while (num_entrada < num_entradas_total) {
        // Leer entrada
        if (mi_read_f(p_inodo, &entrada, num_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            return -1;
        }

        // Leer inodo asociado a la entrada
        if (leer_inodo(entrada.ninodo, &inodo_entrada) == -1) {
            return -1;
        }

        // Formatear la línea de salida
        sprintf(tmp, "%c\t", inodo_entrada.tipo);
        strcat(buffer, tmp);

        // Permisos
        sprintf(tmp, "%c%c%c\t",
            (inodo_entrada.permisos & 4) ? 'r' : '-',
            (inodo_entrada.permisos & 2) ? 'w' : '-',
            (inodo_entrada.permisos & 1) ? 'x' : '-');
        strcat(buffer, tmp);

        // Fecha
        tm = localtime(&inodo_entrada.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);

        // Tamaño
        sprintf(tmp, "%d\t", inodo_entrada.tamEnBytesLog);
        strcat(buffer, tmp);

        // Nombre
        sprintf(tmp, "%s\n", entrada.nombre);
        strcat(buffer, tmp);

        num_entrada++;
    }

    return num_entrada;
}

int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0; // Directorio raíz
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    // Buscar la entrada sin reservar (0)
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // Cambiar permisos usando la función de ficheros
    return mi_chmod_f(p_inodo, permisos);
}

int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    // Buscar la entrada
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }
    if (mi_stat_f(p_inodo, p_stat) < 0){
        fprintf(stderr, "mi_stat()--> Error al mostrar el nº de inodo");
        return -1;
    }

    return p_inodo;
}

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0, p_entrada , p_inodo;
    int error;
    struct inodo inodo;

    // Comprobar si la última entrada coincide con el camino
    if (strcmp(camino, UltimaEntradaEscritura.camino) == 0) {
        p_inodo = UltimaEntradaEscritura.p_inodo;
        #if DEBUGN9
            fprintf(stderr,CYAN"[mi_write() --> Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n"RESET);
        #endif
    } else {
        // Buscar la entrada para obtener el inodo
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return error;
        }
        // Actualizar la caché
        #if DEBUGN9
            fprintf(stderr,ORANGE"[mi_write() --> Actualizamos la caché de escritura]\n"RESET);
        #endif
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }

    leer_inodo(p_inodo, &inodo);

    if (inodo.tipo == 'd'){
        fprintf(stderr, "mi_write()--> El camino no es ningún fichero\n");
        return FALLO;
    }

    int escritos;
    if ((escritos = mi_write_f(p_inodo, buf, offset, nbytes)) < 0) {
        fprintf(stderr, "Bytes escritos: 0\n");
        return FALLO;
    }

    return escritos; 
}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    static struct UltimaEntrada UltimaEntradaLectura = {"", -1};
    int p_inodo;
    unsigned int p_inodo_dir = 0, p_entrada = 0;
    int error;

    // Comprobar si la última entrada coincide con el camino
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0) {
        #if DEBUGN9
            fprintf(stderr,CYAN"[mi_read() --> Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n"RESET);
        #endif
        p_inodo = UltimaEntradaLectura.p_inodo;
    } else {
        // Buscar la entrada para obtener el inodo
        error = buscar_entrada(camino, &p_inodo_dir, (unsigned int *)&p_inodo, &p_entrada, 0, 0);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }
        // Actualizar la caché
         #if DEBUGN9
            fprintf(stderr,ORANGE"[mi_read() --> Actualizamos la caché de escritura]\n"RESET);
        #endif
        strncpy(UltimaEntradaLectura.camino, camino, sizeof(UltimaEntradaLectura.camino) - 1);
        UltimaEntradaLectura.camino[sizeof(UltimaEntradaLectura.camino) - 1] = '\0';
        UltimaEntradaLectura.p_inodo = p_inodo;
    }

    // Llamar a la función de ficheros para leer
    int bytes_leidos = mi_read_f(p_inodo, buf, offset, nbytes);
    return bytes_leidos;
}