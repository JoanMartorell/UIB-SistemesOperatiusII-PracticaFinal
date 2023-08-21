/*
 * verificacion.h -> declaración de funciones y constantes
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "simulacion.h"

struct INFORMACION {
  int pid;
  unsigned int nEscrituras; //validadas
  struct REGISTRO PrimeraEscritura;
  struct REGISTRO UltimaEscritura;
  struct REGISTRO MenorPosicion;
  struct REGISTRO MayorPosicion;
};

