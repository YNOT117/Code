#include <Arduino.h>
#include <Dispensador.h>
#include <pathFirebase.h>

static Dispensador myDispensador;
String mensaje = "";
StaticJsonDocument<250> cambiarProducto;


void firebaseCallback(StreamData);
void timeoutCallback(bool);
float valor;
String valorStr;
bool valorBool;
String namePath;

Preferences preferencess;

void setup() {  
  
  Serial.begin(115200); 
  while(!Serial);
   
  myDispensador.init();
  Serial.println("Inicio de programa");

  myDispensador.configuracionDeTerminales();
  myDispensador.activarInterrupciones();
  myDispensador.actualizarIndicadores();

  if(!digitalRead(ActivarCalibracion)){
    myDispensador.setCalibracionActiva(true);
  }else{
    myDispensador.setCalibracionActiva(false);
  }

  /** Firebase **/
    myDispensador.initWiFi();
    if(myDispensador.isConnected()){
      Firebase.begin(DB_URL, SECRET_KEY);       // Inicializamos Firebase, dandole como argumento el URL y secreto de la base de datos.
      Firebase.reconnectWiFi(true); 

      if(Firebase.beginStream(myDispensador.myFirebaseDataCommands, ComandosBasePath)){   
        Firebase.setStreamCallback(myDispensador.myFirebaseDataCommands, firebaseCallback, timeoutCallback);  
      }else{
        Serial.println("No se puede establecer conexión con la base de datos.");
        Serial.println("Error: " + myDispensador.myFirebaseDataCommands.errorReason());
      }
    }

  // Lectura de pesosDepositados 
  if(preferencess.begin(Archivo, false)){
    myDispensador.setCantidadDePesos(preferencess.getInt(pesosDepositados, 0));
    Serial.print("Depositado: ");
    Serial.println(preferencess.getInt(pesosDepositados, 0));
    preferencess.end();
  }
}

void loop() { 
  delay(7);
  if(myDispensador.isCalibracionActiva()){
    myDispensador.calibracionLoop();
  }else{
    myDispensador.dispensadorLoop();
  }  

}


void firebaseCallback(StreamData data){
  Serial.println("~ Cambios en la base de datos ~");
  //Serial.print("Path: "); Serial.println(myFirebaseDataCommands.dataPath());
  //Serial.print("Tipo: "); Serial.println(data.dataType());            // Que tipo de dato esta llegando
  // Tipos: int, float, string, boolean, null

  /* CorteDeCaja, Despachar */
  if(data.dataType().equals("boolean")){
    valorBool = myDispensador.myFirebaseDataCommands.boolData();
    namePath = myDispensador.myFirebaseDataCommands.dataPath();
    Serial.println(namePath + ": " + valorStr);

    if(namePath.equals(CorteDeCaja)){
      Serial.print("Corte: "); Serial.println(valorBool);
      myDispensador.corteDeCaja();
    }

    if(namePath.equals(Despachar05L)){
      Serial.print("Despachar: "); Serial.println(valorBool);
    }

    if(namePath.equals(Calibracion)){
      myDispensador.setCalibracionActiva(valorBool);
    }

    if(namePath.equals(Suministrar)){
      // Aqui va el el metodo suministrar
      Serial.print("Modo suministro: ");
      Serial.println(valorBool);
    }
  }

  /* Nombres */
  if(data.dataType().equals("string")){
    valorStr = myDispensador.myFirebaseDataCommands.stringData();
    namePath = myDispensador.myFirebaseDataCommands.dataPath();
    Serial.println(namePath + ": " + valorStr);

    /* Nombre del dispensador */ 
    if(namePath.equals(Nombre)){
      myDispensador.setNombreDispensador(valorStr);
    }
      // /Productos/0/Nombre
    /* Nombre de los productos */
    if(namePath.equals(P0Nombre) || namePath.equals(P1Nombre)
      || namePath.equals(P2Nombre) || namePath.equals(P3Nombre)
      || namePath.equals(P4Nombre) || namePath.equals(P5Nombre)
      || namePath.equals(P6Nombre) || namePath.equals(P7Nombre)){
      
      // Los subString empiezan desde cero (0)
      myDispensador.setNombreProducto(valorStr, namePath.substring(11,12).toInt());

    }
    

  }

  /* Precio, Tiempo, Cantidad, AllPromo, Promocion */
  if(data.dataType().equals("int")){
    valor = myDispensador.myFirebaseDataCommands.intData();
    namePath = myDispensador.myFirebaseDataCommands.dataPath();
    Serial.println(namePath + ": " + valor);
    
    /* Precio */
    if(namePath.equals(P0Precio) || namePath.equals(P1Precio)
      || namePath.equals(P2Precio) || namePath.equals(P3Precio)
      || namePath.equals(P4Precio) || namePath.equals(P5Precio)
      || namePath.equals(P6Precio) || namePath.equals(P7Precio)){
      
      myDispensador.setCostoProducto(valor, namePath.substring(11,12).toInt());
    }

    /* Tiempo */
    if(namePath.equals(P0Tiempo) || namePath.equals(P1Tiempo)
      || namePath.equals(P2Tiempo) || namePath.equals(P3Tiempo)
      || namePath.equals(P4Tiempo) || namePath.equals(P5Tiempo)
      || namePath.equals(P6Tiempo) || namePath.equals(P7Tiempo)){

      myDispensador.setTiempoProducto(valor, namePath.substring(11,12).toInt());
    }

    /* Cantidad */
    if(namePath.equals(P0Cantidad) || namePath.equals(P1Cantidad)
      || namePath.equals(P2Cantidad) || namePath.equals(P3Cantidad)
      || namePath.equals(P4Cantidad) || namePath.equals(P5Cantidad)
      || namePath.equals(P6Cantidad) || namePath.equals(P7Cantidad)){

      myDispensador.setCantidadProducto(valor, namePath.substring(11,12).toInt());
    }


    // Las promociones seran en descuentos
    // 1% de descuento
    // 2% de descuento, etc.
    // AllPromo
    if(namePath.equals(AllPromocion)){
      Serial.print("Promocion del dispensador: "); Serial.println(valor);
      myDispensador.allPromocion(valor);
    }

    /* Promocion */
    if(namePath.equals(P0Promocion) || namePath.equals(P1Promocion)
      || namePath.equals(P2Promocion) || namePath.equals(P3Promocion)
      || namePath.equals(P4Promocion) || namePath.equals(P5Promocion)
      || namePath.equals(P6Promocion) || namePath.equals(P7Promocion)){

      myDispensador.setPromocion(valor, namePath.substring(11,12).toInt());
    }
   
  }

  

}

void timeoutCallback(bool timeCallback){
  if(timeCallback){
    Serial.println("Tiempo de espera excedido");
  }
}