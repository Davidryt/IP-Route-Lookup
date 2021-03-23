/**
 * @mainpage ROUTE LOOKUP
 * @section Introduction
 * A continuación encontraras la documentacion del proyecto my_route_lookup. He decidido elaborarla de esta manera ya que
 * creo que resultara más sencillo (y estandarizado) para la comprensión del codigo desarrollado
 * @section About
 * Este programa leerá un conjunto de direcciones IP y, comparándolas con una tabla de enrutamiento determinada, generará un informe con el 
 * interfaz asignado y un resumen del uso de la memoria / CPU y el tiempo de ejecución
 * @section Software
 * Desarrollado con Doxygen 1.8.17 and C programming language.
 * @author      David Rico <100384036@alumnos.uc3m.es>
 */

#include "my_route_lookup.h"
#include "io.h"


 /** 
   * Estructura AllTables.
   * Esta estructura contiene las dos tablas que seran utilizadas para el route lookup, asi como un indice y extensionesaux.
   */
Tablas AllTables;
 /** 
   * Variable de Media de Accesos de Memoria.
   * Esta variable nos permite cuantificar el numero de accesos de memoria (a las tablas) que se han relizado.
   */
double *Accesos;
 /** 
   * Variable de paquetes procesados.
   * Esta variable nos permite cuantificar el numero de paquetes procesados (Direcciones IP procesadas).
   */
int *PaquetesOK; 
 /** 
   * Variable que almacena el tiempo de ejecución total.
   */
double *TiempoTotal; 
 /** 
   * Variable de error.
   * Esta variable se encarga de registrar un error en la ejecucion (i.e Durante la lectura de IPs).
   */
int ErrorCheck;	
 /** 
   * Estructura timespec de la libreria time.h.
   * Estas estructuras almacenarán el tiempo de inicio y fin de cierta funcion.
   */
struct timespec start, end;
 /** 
   * Variable IP Address.
   * Almacena la IP leida de manera temporal.
   */
uint32_t *IPAddress;  
 /** 
   * Variable Interfaz de Salida.
   * Aqui almsacenaremos de manera temporal la interfaz asignada a la IP.
   */
int *InterfazSalida;
 /** 
   * Variable de Prefijo.
   * Utilizaremos esta variable para controlar la longitud del prefijo.
   */
int *mascara;


  /**
  * Loop principal de nuestro algoritmo (MAIN).
  * @param argc Integer que nos indica el numero de argumentos recibidos + 1.
  * @param argv Array que contiene dichos argumentos.
  * @return 1 en caso de Éxito
  * @return -1 en caso de Error de ejecución
  */
int main( int argc, char *argv[] ){
  
  TablasInit(&AllTables);	/** Primero inicializamos nuestra estructura, asi como las dos tablas que contiene */
  
   VarInit();	/** Inicializamos las variables */

	if( argc == 3 ){	
		ErrorCheck = initializeIO(argv[1], argv[2]); 
		if(ErrorCheck != 0){
			printf("\nAttributes not valid:\t");
			printIOExplanationError(ErrorCheck);
			return -1;
		}
		ReadIPs();
		SetRoutes();
		printSummary(*PaquetesOK, (*Accesos / *PaquetesOK), (*TiempoTotal / *PaquetesOK));
		freeIO();
		free(PaquetesOK);
		free(Accesos);
		free(TiempoTotal);
		DeleteAllTables(&AllTables);


	}
	else{
		printf("\n Wrong Input FIles. Expected attributes : 3");
		return -1;
	}
	return 1;
}

  /**
  * @internal 
  *	Reservamos memoria de nuestras variables PaquetesOK, Accesos, TiempoTotal. 
  * Inicializamos la variable de Error a 0 (Sin error)
  * 
  * @endinternal
  */ 
void VarInit(){

	PaquetesOK  = calloc(1,sizeof(int));
	Accesos  = calloc(1,sizeof(double));
	TiempoTotal  = calloc(1,sizeof(double));
	ErrorCheck = 0;
	
}

  /**
  * @internal 
  *	Para la tabla principal, reservamos la memoria necesaria para su maximo valor (2^24) 
  * La tabla auxiliar (no inicializada todavia)
  * Las dos variables (indice y extensionesaux) se inicializan a valor 0 en primera instancia
  * 
  * @endinternal
  */
void TablasInit(Tablas* AllTables) {
   AllTables->main_table = calloc( pow(2,24) ,sizeof(short));
   AllTables->extensionesaux = 0;
   AllTables->ip_index = 0;
}

  /**
  * @internal 
  *	Liberamos la memoria de las dos tablas generadas (main y aux) 
  * 
  * @endinternal
  */
void DeleteAllTables(Tablas* AllTables) {
		free(AllTables->aux_table);
		free(AllTables->main_table);
}

  /**
  * @internal 
  *	Primero reservamos memoria para las variables IPAddress, mascara, InterfazSalida. 
  * El bucle while:
  * @verbatim 
  while(ErrorCheck == 0){  
	AddRoute(&AllTables, IPAddress,mascara,InterfazSalida);
	ErrorCheck = readFIBLine(IPAddress,mascara,InterfazSalida);
  }
   @endverbatim
  * Añade una ruta a las tablas y le la siguiente lina, siempre que no haya error (error de lectura o EOF)
  * Por ultimo liberamos la memoria de las variables temporales
  * @endinternal
  */
void ReadIPs(){
	 mascara = calloc(1,sizeof(int));
  InterfazSalida = calloc(1,sizeof(int));
  IPAddress = calloc(1,sizeof(int));
  
  while(ErrorCheck == 0){  
    ErrorCheck = readFIBLine(IPAddress,mascara,InterfazSalida);
	if(ErrorCheck == 0){
    AddRoute(&AllTables, IPAddress,mascara,InterfazSalida);
	}
  }
  free(IPAddress);
  free(mascara);
  free(InterfazSalida);
}

  /**
  * @internal 
  *	Este metodo realiza la busqueda de cierta IP previamente insertada en las Tablas y cuantidfica el numero de accesos
  * 
  * @endinternal
  */
void Search(int *accesoextra, Tablas* AllTables, uint32_t *IPToSearch,  unsigned int *intf2){
	*intf2 = AllTables->main_table[*IPToSearch>>8]; /** Recortamos el valor que se encuentre en la tabla principal para esa IP (Usamos un shift de 8 para recortar la IP de 32 a 24 ya que la tabla main solo acepta 24 bits)*/
	int ExisteAux = *intf2>>15; /** Para saber si pertenece o no a la tabla auxiliar, utilizabamos el bit extra de la interfaz (nº 15) para ponerlo a 0 (si no tenia tabla auxiliar correspondiente) o a 1 (Si existe tabla auziliar para este) */
	if(ExisteAux == 0){  /** No existe aux */
		*accesoextra = 1;  /** De encontrarse directamente en la tabla principal, 1 acceso y nada mas que hacer */
		return;
	}else{
		*accesoextra = 2;
		*intf2 = AllTables->aux_table[(*intf2 & 0x7FFF)*256 + (*IPToSearch & 0x000000FF)];	/** Sin embargo, en la auxiliar sera necesario volver a buscar, y los accesos serán 2 */
		return;
	}
	return;
}

  /**
  * @internal 
  *	Primero declaramos reservamos memoria para las variables temporales IPToSearch, intf, searching_time, accesoextra. (Usamos variables temporales ya que la información almacenada en estas sera printeada por esta misma función)  --> IMPORTANTE: Liberar
  * 
  * @endinternal
  */
void SetRoutes(){
	uint32_t *IPToSearch = calloc(1,sizeof(uint32_t));
	unsigned int *intf = calloc(1,sizeof(unsigned int));
	double *searching_time = calloc(1,sizeof(double));
	int *accesoextra = calloc(1,sizeof(int));
	ErrorCheck=0;   	/** Importante tener en cuenta que ErrorCheck viene de ReadIPs con un valor =! 0 (final del archivo que se estava leyendo) y es importante restaurar a su valor por defecto (me ha dado serios problemas este detalle) */
	while(ErrorCheck == 0){			/** Clasico Bucle Mientaras no halla errores */
		ErrorCheck = readInputPacketFileLine(IPToSearch);	/** Leer linea e actializar ErrorCheck*/
		if(ErrorCheck == 0){		/** Segunda comprobación de ErrorCheck para evitar errores (Cuenta un paquete de mas asi como accesos etc) */
			clock_gettime(CLOCK_MONOTONIC_RAW, &start);		/**  Realizamos la primera medicion del Clock como se nos indica en io.h */									
			Search(accesoextra, &AllTables, IPToSearch,  intf);		/**  Ejectuamos el acceso a la IP de la cual se quiere medir el tiempo de busqueda 
																	*	@see Search()
																	*/		
			clock_gettime(CLOCK_MONOTONIC_RAW, &end);		/**   Realizamos la segunda medicion del Clock como se nos indica en io.h  */
			printOutputLine(*IPToSearch, *intf, &start, &end,searching_time, *accesoextra);	/**   Imprimimos los datos obtenidos por pantalla usando la estructura IP;INT;Tiempo  usando el metodo dado */
			*PaquetesOK = *PaquetesOK + 1;
			*Accesos  = *Accesos + *accesoextra;
			*TiempoTotal  = *TiempoTotal + *searching_time;
			/** Aumentamos las variables en funcion de lo obtenido en Search y almacenamos en las variables globales, que luego sera printadas
			*	@see printSummary(*PaquetesOK, (*Accesos / *PaquetesOK), (*TiempoTotal / *PaquetesOK)
			*/
		}
	}
	/** Liberamos la memoria de las variable (SOLO LAS TEMPORALES) */
	free(intf);
	free(searching_time);
	free(IPToSearch);
	free(accesoextra);
}

  /**
  * @internal 
  *	Esta función, como su nombre indica, se encarga de añadir una determinada IP a nuestras tablas.
  * Esto propone 2 casos: Al igual que en la función Search(), debemos saber si hemos de insertar en la 1ª o 2ª Tabla (main o aux)
  * Sin embargo, a diferencia que en search obtenemos la mascara como argumento, por lo tanto, si esta en menor o igual que 24, sera la tabla 1
  * 
  * @endinternal
  */
void AddRoute(Tablas* AllTables, uint32_t *IPToAdd, int *mascara, int *intf3){
    long int IPsAModificar = 0;	
	int ExisteAux;
    if(*mascara <= 24){	/** En la tabla main: */
        IPsAModificar = pow(2,24 - *mascara);	/** Calculamos cuantas IPs hemos de modificar su interfaz para la nueva mascara  SIEMPRE sera total de IPs menos la mascara  */
		for(AllTables->ip_index = 0; AllTables->ip_index < IPsAModificar; AllTables->ip_index++){ /** Un for sencillo que recorre la tabla de 0 hasta el valor de IPsAModificar */
			AllTables->main_table[(*IPToAdd>>8) + AllTables->ip_index] = *intf3; /** Realizamos un shift sobre la IP para obtener solo los bits que importan (24 bits ya que como hemos declarado en el if, la mascara es menor que 24, por lo tanto nos da igual que venga despues del bit 24) y asignamos la interfaz*/
		}
	}else{
		ExisteAux = (AllTables->main_table[*IPToAdd>>8]>>15); /** De la misma manera que en search(), comprobamos si existe una tabla aux*/
	    IPsAModificar = pow(2,32 - *mascara);	/** Calculamos cuantas IPs hemos de modificar su interfaz para la nueva mascara SIEMPRE sera total de IPs (ahora es 2^32 ya que estamos en la tabla auxiliar, que es mas grande que la main pero no estatica)menos la mascara  */
		if(ExisteAux == 0) {	 /** No existe aux */
			AllTables->aux_table = realloc(AllTables->aux_table, 256*(AllTables->extensionesaux + 1)*2);	/** En el caso de no existir, hemos de crearla! No olvidar usar MEMORIA DINAMICA*/
			/** El tamaño del realloc lo calculamos como el nuevo bloque (256)+ los bloques ya existentes (256*existensiones)*/
			for(AllTables->ip_index = 0; AllTables->ip_index <= 255; AllTables->ip_index++){
				AllTables->aux_table[AllTables->extensionesaux*256 + AllTables->ip_index] = AllTables->main_table[*IPToAdd>>8];	/** Con un for de 0 a 255 (256 posiciones) recorremos la nueva extension de la tabla rellenándola */
			}
			for(AllTables->ip_index = (*IPToAdd & 0xFF); AllTables->ip_index < IPsAModificar + (*IPToAdd & 0xFF); AllTables->ip_index++){
				AllTables->aux_table[AllTables->extensionesaux*256 + AllTables->ip_index] = *intf3;	/** Rellenamos los espacios creados con la interfaz correspondiente */ /** Se puede simplificar? */
			}
			AllTables->main_table[*IPToAdd>>8] = AllTables->extensionesaux | 0b1000000000000000;	/** Es  importante indicar en la tabla principal que ya hemos creado una extension de la tabla auxiliar. Al principio no hacia esto, y se generaban extensiones de la tabla por cada IP resultando en un absoluto gasto de memoria */
			/** Para realizar este cambio hemos utilizado operadores bitwise.  So we use bit 15 as a flag @see Check <https://stackoverflow.com/questions/37948920/changing-the-most-significant-bit-in-a-int16> for a detailed example  */
		    AllTables->extensionesaux++;	/** al igual que lo indicamos en la tabla principal, tambien aumentamos en uno el numero de extensiones */ /** Si se aumenta el valor antes de cambiarlo en la tabla, resulta en error */
		}else{ 
		    for(AllTables->ip_index = (*IPToAdd & 0xFF); AllTables->ip_index < IPsAModificar + (*IPToAdd & 0xFF); AllTables->ip_index++){
		        AllTables->aux_table[(AllTables->main_table[*IPToAdd>>8] & 0x7FFF)*256 + AllTables->ip_index] = *intf3; /** Rellenamos los espacios creado con la interfaz */  /** Simplificar ?*/
		    }
		}
	}
}





