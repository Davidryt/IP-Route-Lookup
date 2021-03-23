#include <stdint.h>
#include <stdio.h>
#include <math.h>

 /**
 * Estructura que almacena las 2 tablas principales, así como 2 indices
 */
typedef struct KitTablas {
 short *main_table;		/**< Tabla principal (Tamaño fijo 2^24) */
 short *aux_table;		/**< Tabla auxiliar (Tamaño dinámico) */
 unsigned int extensionesaux; /**< indice auxiliar */
 long ip_index; 	/**< Tabla de IPs */
} Tablas ;

  /**
  * Inicializa las variables globales
  */
void VarInit();
  /**
  * Inicializa las tablas
  * @param tabla Estructura que va a ser inicializada
  */
void TablasInit(Tablas* tabla);

  /**
  * Elimina y libera las tablas
  * @param tablas Estructura que va a ser eliminada
  */
void DeleteAllTables(Tablas* tablas);

  /**
  * REVISAR
  * @param tablas Estructura utilizada en la busqueda
  * @param IPToSearch IP sobre la cual se obtiene interfaz 
  * @param accesoextra Nos indica a cuantas tablas accedemos en la busqueda (1 o 2)
  * @param intf2 Interfaz a calcular
  */
void Search(int *accesoextra, Tablas* tablas, uint32_t *IPToSearch,  unsigned int *intf2);

  /**
  * Bucle encargado de la lectura de IPs del input
  */
void ReadIPs();

  /**
  * Calcula los tiempos y imprime la informacion por pantalla
  */
void SetRoutes();

  /**
  * REVISAR
  * Añade la ruta a las tablas
  * @param tablas Estructura en la cual se realiza la inserción
  * @param IPToAdd IP a insertar 
  * @param mascara de la ip
  * @param intf3 Interfaz de salida que se genera
  */
void AddRoute(Tablas* tablas, uint32_t *IPToAdd, int *mascara, int *intf3);



