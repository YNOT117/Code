#include <Arduino.h>
/** Configuracion de internet **/
#define SSID    "Chekov"//"Rectoria" //"Discovery117_2.4Gnormal" //"Chekov" //"Enterprise117"
#define PASS    "ChinoLoko69" //"ChinoLoko69" //"recsec2015"

/* Configuracion de la base de datos en tiempo real*/
#define SECRET_DB "gNZew0dQoUAOq9bxTC1ikLvbQeRd5uKSP7My6Qf1"
#define DATABASE_URL "testesp32-fab17-default-rtdb.firebaseio.com" 

/* Host para hacer ping y verificar la conexion a internet */
#define HOST "www.google.com"
#define PORT 80
#define TiempoDeReconexion 3000 // 15 min -> 900000 miliSegundos

#define Nodo "/"
#define TiempoDeLectura 2500

/* Datos del dispensador */
#define nombreDelDispensador "Dispensador 124"
#define cantidadDeProductos 16

/*** Configuracion de interrupcion del monedero ***/
#define PinMonedero 4

/* Terminal para la interrupcionde botones*/
#define PinBotones 5

/* El boton de encoder se usara tambien como pin para iniciar calibracion */
#define PinCalibracion 15

/* Terminales para el encoder rotativo*/
#define encoderA 16
#define encoderB 17

/** Constantes de tiempo **/
#define tiempoDeRebote 65 // en mS
#define tiempoDeReboteBoton 200 // en mS

/** Nombre de la base de datos **/
#define DataBase "DispDataV3"

/** Cantidad minima de producto dispoble para despahcar, en litros**/
#define CantidadMinimaDeProducto 1  

/* Etiqueta para los leds indicadores */
#define SI_HAY LOW
#define NO_HAY HIGH



