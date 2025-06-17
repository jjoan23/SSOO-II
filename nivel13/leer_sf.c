// AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
// leer_sf.c: Implementación de un programa que lee el superbloque de un sistema de ficheros y muestra su contenido.
#include "directorios.h"
#define DEBUGSTRUCT 0
#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN7 0

void mostrar_buscar_entrada(char *camino, char reservar)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    return;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    { // Comprobamso sintaxis
        if (bmount(argv[1]) == -1)
        { // MOntamos el disco
            fprintf(stderr, "Error al montar el dispositivo\n");
            return FALLO;
        }

        struct superbloque SB;
        if (bread(posSB, &SB) == -1)
        { // Leemos el superbloque del disco
            fprintf(stderr, "Error al leer el superbloque\n");
            return FALLO;
        }

        printf("DATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB = %i\n", SB.posPrimerBloqueMB);
        printf("posUltimoBloqueMB = %i\n", SB.posUltimoBloqueMB);
        printf("posPrimerBloqueAI = %i\n", SB.posPrimerBloqueAI);
        printf("posUltimoBloqueAI = %i\n", SB.posUltimoBloqueAI);
        printf("posPrimerBloqueDatos = %i\n", SB.posPrimerBloqueDatos);
        printf("posUltimoBloqueDatos = %i\n", SB.posUltimoBloqueDatos);
        printf("posInodoRaiz = %i\n", SB.posInodoRaiz);
        printf("posPrimerInodoLibre = %i\n", SB.posPrimerInodoLibre);
        printf("cantBloquesLibres = %i\n", SB.cantBloquesLibres);
        printf("cantInodosLibres = %i\n", SB.cantInodosLibres);
        printf("totalBloques = %i\n", SB.totBloques);
        printf("totInodos = %i\n", SB.totInodos);

#if DEBUGSTRUCT
        printf("sizeof struct superbloque: %li\n", sizeof(struct superbloque));
        printf("sizeof struct inodo: %li\n", sizeof(struct inodo));
#endif

#if DEBUGN2
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
        struct inodo inodos[BLOCKSIZE / INODOSIZE];
        for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
        { // Recorremos toda la lista enlazada
            if (bread(i, &inodos) == -1)
            {
                fprintf(stderr, "Error al leer el bloque de inodos\n");
                return FALLO;
            }
            for (int i = 0; i < (BLOCKSIZE / INODOSIZE); i++)
            {
                printf("%i ", inodos[i].punterosDirectos[0]);
            }
        }
#endif

#if DEBUGN3
        printf("RESERVAMOS UN BLOQUE Y LUEGO LIBERAMOS\n");
        int reserva = reservar_bloque(); // Reservamos un bloque
        bread(posSB, &SB);

        printf("Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB\n", reserva);
        printf("SB.cantBloquesLibres = %i\n", SB.cantBloquesLibres);
        liberar_bloque(reserva); // LIberamos el bloque anterior
        if (bread(posSB, &SB) == -1)
        { // Leemos el superbloque
            fprintf(stderr, "Error al leer el superbloque\n");
            return FALLO;
        }
        printf("Liberamos ese bloque y despues SB.cantBloquesLibres = %i\n", SB.cantBloquesLibres);

        printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
        int bit = leer_bit(posSB);
        printf("[posSB: %i-> leer_bit(%i) = %i\n", posSB, posSB, bit);

        // Por simplicidad vamos a imprimir los bits del MB de los inicios de cada zona
        bit = leer_bit(SB.posPrimerBloqueMB);
        printf("[SB.posPrimerBloqueMB: %i-> leer_bit(%i)=%i\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, bit);
        bit = leer_bit(SB.posUltimoBloqueMB);
        printf("[SB.posUltimoBloqueMB: %i -> leer_bit(%i)=%i\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, bit);

        bit = leer_bit(SB.posPrimerBloqueAI);
        printf("[SB.posPrimerBloqueAI: %i -> leer_bit(%i)=%i\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, bit);
        bit = leer_bit(SB.posUltimoBloqueAI);
        printf("[SB.posUltimoBloqueAI: %i -> leer_bit(%i)=%i\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, bit);

        bit = leer_bit(SB.posPrimerBloqueDatos);
        printf("[SB.posPrimerBloqueDAtos: %i -> leer_bit(%i)=%i\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, bit);
        bit = leer_bit(SB.posUltimoBloqueDatos);
        printf("[SB.posUltimoBloqueDatos: %i -> leer_bit(%i)=%i\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, bit);

        printf("DATOS DEL DIRECTORIO RAIZ\n");
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];
        char btime[80];

        struct inodo inodo;
        int ninodo = 0; // Inodo raiz!

        leer_inodo(ninodo, &inodo);
        printf("tipo: %c\n", inodo.tipo);
        printf("permisos: %i\n", inodo.permisos);
        ts = localtime(&inodo.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        ts = localtime(&inodo.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        ts = localtime(&inodo.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        ts = localtime(&inodo.btime);
        strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        printf("ID: %d ATIME: %s MTIME: %s CTIME: %s BTIME: %s\n", ninodo, atime, mtime, ctime, btime);
        printf("nlinks: %i\n", inodo.nlinks);
        printf("tamEnBytesLog: %i\n", inodo.tamEnBytesLog);
        printf("numBloquesOcupados: %i\n", inodo.numBloquesOcupados);

#endif

#if DEBUGN4
        int inodo = reservar_inodo('f', 6);
        if (inodo == -1)
        {
            fprintf(stderr, "Error al reservar inodo\n");
            return FALLO;
        }

        struct inodo inodoReservado;
        leer_inodo(inodo, &inodoReservado);
        printf("INODO TRADUCCIÓN DE LOS BLOQUES LÓGICOS 8, 204, 30.004, 400.004 y468.750\n");

        // Traducimos los BL 8, 204, 30.004, 400.004 y468.750
        traducir_bloque_inodo(&inodoReservado, 8, 1);
        traducir_bloque_inodo(&inodoReservado, 204, 1);
        traducir_bloque_inodo(&inodoReservado, 30004, 1);
        traducir_bloque_inodo(&inodoReservado, 400004, 1);
        traducir_bloque_inodo(&inodoReservado, 468750, 1);
        escribir_inodo(inodo, &inodoReservado);

        printf("DATOS DEL INODO RESERVADO\n");
        struct inodo inodoR;
        leer_inodo(inodo, &inodoR);
        printf("tipo: %c\n", inodoR.tipo);
        printf("permisos: %i\n", inodoR.permisos);

        // Fechas y horas!!
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];
        char btime[80];
        ts = localtime(&inodoR.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        ts = localtime(&inodoR.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        ts = localtime(&inodoR.btime);
        strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        ts = localtime(&inodoR.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S\n", ts);
        printf("ID: %d ATIME: %s MTIME: %s CTIME: %s BTIME: %s\n", inodo, atime, mtime, ctime, btime);

        printf("nlinks: %i\n", inodoR.nlinks);
        printf("tamEnBytesLog: %i\n", inodoR.tamEnBytesLog);
        printf("numBloquesOcupados: %i\n", inodoR.numBloquesOcupados);
        bread(posSB, &SB);
        printf("SB.posPrimerInodoLibre = %i\n", SB.posPrimerInodoLibre);

#endif
#if DEBUGN7
        // Mostrar creación directorios y errores
        mostrar_buscar_entrada("pruebas/", 1);           // ERROR_CAMINO_INCORRECTO
        mostrar_buscar_entrada("/pruebas/", 0);          // ERROR_NO_EXISTE_ENTRADA_CONSULTA
        mostrar_buscar_entrada("/pruebas/docs/", 1);     // ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
        mostrar_buscar_entrada("/pruebas/", 1);          // creamos /pruebas/
        mostrar_buscar_entrada("/pruebas/docs/", 1);     // creamos /pruebas/docs/
        mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // creamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
        // ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
        mostrar_buscar_entrada("/pruebas/", 1);          // ERROR_ENTRADA_YA_EXISTENTE
        mostrar_buscar_entrada("/pruebas/docs/doc1", 0); // consultamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // creamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/casos/", 1);    // creamos /pruebas/casos/
        mostrar_buscar_entrada("/pruebas/docs/doc2", 1); // creamos /pruebas/docs/doc2
#endif
        if (bumount())
        { // Desmontamos el disco
            fprintf(stderr, "Error al desmontar el dispositivo\n");
            return FALLO;
        }
    }
    else
    {
        fprintf(stderr, "Error de sintaxis: ./leer_sf <nombre_dispositivo> \n");
        return FALLO;
    }
}