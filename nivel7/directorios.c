#include "directorios.h"
#define DEBUGN7 1

#define NUM_ENTRADAS (BLOCKSIZE / sizeof(struct entrada))

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


int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int n_entradas, i_entrada_inodo;

    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, sizeof(final));
    memset(&entrada, 0, sizeof(entrada));

    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error al leer el superbloque\n");
        return FALLO;
    }

    if (!strcmp(camino_parcial, "/")) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -2) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUGN7
        fprintf(stderr, GRAY "buscar_entrada() --> inicial: %s, final: %s, reservar: %d\n" RESET, inicial, final, reservar);
    #endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1) {
        return FALLO;
    }

    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    n_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    i_entrada_inodo = 0;

    struct entrada entradas[NUM_ENTRADAS];
    int offset = 0;
    int entrada_encontrada = 0;

    while (i_entrada_inodo < n_entradas && entrada_encontrada==0) {
        int leidos = mi_read_f(*p_inodo_dir, entradas, offset, sizeof(entradas));
        int num_entradas_leidas = leidos / sizeof(struct entrada);
        for (int i = 0; i < num_entradas_leidas; i++) {
            if (!strcmp(inicial, entradas[i].nombre)) {
                entrada = entradas[i];
                entrada_encontrada = 1;
                break;
            }
            i_entrada_inodo++;
        }
        offset += leidos;
    }

    if (entrada_encontrada==0) {
        if (!reservar) {
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        }

        if (inodo_dir.tipo != 'd') {
            return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
        }

        if ((inodo_dir.permisos & 2) != 2) {
            return ERROR_PERMISO_ESCRITURA;
        }

        struct entrada nueva_entrada;
        memset(&nueva_entrada, 0, sizeof(struct entrada));
        strncpy(nueva_entrada.nombre, inicial, sizeof(nueva_entrada.nombre));

        if (tipo == 'd') {
            if (strcmp(final, "/") == 0) {
                nueva_entrada.ninodo = reservar_inodo('d', permisos);
            } else {
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            }
        } else {
            nueva_entrada.ninodo = reservar_inodo('f', permisos);
        }

        if (mi_write_f(*p_inodo_dir, &nueva_entrada, n_entradas * sizeof(struct entrada), sizeof(struct entrada)) == -1) {
            liberar_inodo(nueva_entrada.ninodo);
            return FALLO;
        }

        entrada = nueva_entrada;
        i_entrada_inodo = n_entradas;

        #if DEBUGN7
            fprintf(stderr, GRAY "buscar_entrada()-->creada entrada: %s, inodo: %d\n" RESET, entrada.nombre, entrada.ninodo);
        #endif
    }

    if (!strcmp(final, "/") || !strcmp(final, "")) {
        *p_inodo = entrada.ninodo;
        *p_entrada = i_entrada_inodo;
        if (reservar && entrada_encontrada) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        return EXITO;
    }

    *p_inodo_dir = entrada.ninodo;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
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
