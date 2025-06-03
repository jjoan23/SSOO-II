//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//verificacion.h
//cabecera de la verificación de la simulación
#include "simulacion.h"


struct INFORMACION {
  int pid;
  unsigned int nEscrituras; //validadas 
  struct REGISTRO PrimeraEscritura;
  struct REGISTRO UltimaEscritura;
  struct REGISTRO MenorPosicion;
  struct REGISTRO MayorPosicion;
};
