#include "directorios.h"
#define DEBUGN7 1

#define NUM_ENTRADAS (BLOCKSIZE / sizeof(struct entrada))

<<<<<<< HEAD
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // Verificar que el camino comienza con '/'
    if (camino == NULL || camino[0] != '/') {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Encontrar la segunda '/' después del primer carácter
    const char *segunda_barra = strchr(camino + 1, '/');

    if (segunda_barra) {
        // Caso directorio: hay una segunda '/'
        size_t longitud = segunda_barra - (camino + 1);

        // Validar longitud del nombre
        if (longitud >= TAMNOMBRE) {
            return ERROR_CAMINO_INCORRECTO;
        }

        // Copiar parte inicial (nombre del directorio)
        strncpy(inicial, camino + 1, longitud);
        inicial[longitud] = '\0';

        // Copiar resto del camino (final)
        strcpy(final, segunda_barra);
        *tipo = 'd';

    } else {
        // Caso fichero: no hay segunda '/'
        size_t longitud = strlen(camino + 1);

        // Validar longitud del nombre
        if (longitud >= TAMNOMBRE) {
            return ERROR_CAMINO_INCORRECTO;
        }

        // Copiar nombre del fichero
        strcpy(inicial, camino + 1);
        final[0] = '\0'; // Cadena vacía
        *tipo = 'f';
    }

    return EXITO;
=======
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
struct superbloque SB;
return 0;
>>>>>>> dbdbaa53d4cc0854bb85fab3ede3fa36a5ee9470
}

if ((error = extraer_camino(camino_parcial, inicial, resto, &tipo)) < 0)
return ERROR_CAMINO_INCORRECTO;

<<<<<<< HEAD
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
struct superbloque SB;
return 0;
}

if ((error = extraer_camino(camino_parcial, inicial, resto, &tipo)) < 0)
return ERROR_CAMINO_INCORRECTO;

=======
>>>>>>> dbdbaa53d4cc0854bb85fab3ede3fa36a5ee9470
printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, resto, reservar);

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

printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n",
nueva.ninodo, (tipo == 'd') ? 'd' : 'f', permisos, inicial);

if (mi_write_f(*p_inodo_dir, &nueva, num_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
return -1;

printf("[buscar_entrada()→ creada entrada: %s, %d]\n", inicial, nueva.ninodo);

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