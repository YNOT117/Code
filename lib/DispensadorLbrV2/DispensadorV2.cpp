#include "DispensadorV2.h"

LiquidCrystal_I2C Display(0x27, 16, 2); // Display por fuera
PCF8574 Botones(0x20);
PCF8574 Botones2(0x21);
PCF8574 Bombas(0x24);
PCF8574 Bombas2(0x3D);
Preferences preferences;

unsigned long tiempoAnteriorRebote;
unsigned long tiempoAnteriorReboteBoton;
unsigned long tiempoAnterior;
float _Credito = 0;
volatile bool _CreditoAdded = false;
bool _BotonInterrupcion = false;

/** Caracteres especiales **/
byte customChar[8] = {
	0b00000,
	0b01010,
	0b01010,
	0b01010,
	0b00000,
	0b10001,
	0b01110,
	0b00000
};
byte customCharTriste[8] = {
  B00000,
  B01010,
  B01010,
  B01010,
  B00000,
  B01110,
  B10001,
  B00000
};


/** Declaracion de funciones de interrupcion **/
void IRAM_ATTR agregarCredito();
void IRAM_ATTR seleccionProducto();

DispensadorV2::DispensadorV2(String usuario, String nombre, byte cantidad){
    _Usuario = usuario;
    _Nombre = nombre;
    _CantidadDeProductos = cantidad;
}

void DispensadorV2::begin(){

    Wire.begin();
    //if(Wire.available()){ Wire.read(); }
    Serial.println();
    Serial.println("~ Configuraciones ~");
    /** Inicializar expansores **/
    /** Las salidas estan negadas, por eso se inicializa con un HIGH**/
    Botones.begin( )==true?Serial.println("Expansor Botones - CONECTADO"):Serial.println("Expansor Botones - NO CONECTADO");
    Botones2.begin( )==true?Serial.println("Expansor Botones - CONECTADO"):Serial.println("Expansor Botones - NO CONECTADO");
    Bombas.begin(HIGH)==true?Serial.println("Expansor Bombas - CONECTADO"):Serial.println("Expansor Bombas - NO CONECTADO");
    Bombas2.begin(HIGH)==true?Serial.println("Expansor Bombas - CONECTADO"):Serial.println("Expansor Bombas - NO CONECTADO");
    
    Serial.println("Apagar bombas");
    for(uint8_t bomb=0; bomb<8; bomb++){
        Bombas.write(bomb, HIGH);
        Bombas2.write(bomb, HIGH);
    }

    Serial.println("Inicializar display");
    /** Inicializar display LCD **/
    Display.init();
    Display.backlight();
    Display.createChar(0, customChar);          // Crear caracter 
    Display.createChar(1, customCharTriste);    // Crear caracter 
    Display.clear();
    Display.home();
    Display.write(0);
    Display.print(" ");
    Display.print("Iniciando!!!");   
    Display.print(" ");
    Display.write(0);
    
    Serial.println("Configurar terminales");
    pinMode(PinCalibracion, INPUT_PULLUP);
    pinMode(PinMonedero, INPUT_PULLUP);
    pinMode(PinBotones, INPUT_PULLUP);
    activarInterrupciones();

    /* Conexion al WiFi*/
    /* Sacar del bucle cuando no encuentra wifi */
    conecToWiFi();
}

void DispensadorV2::conecToWiFi(){
    Serial.println();
    Serial.println("~ Conexion a internet ~");
    Serial.println("Habilitar STA");
    WiFi.enableSTA(true);
    Serial.print("Conectando a " SSID);
    Display.clear();
    Display.home();
    Display.print("~  Intentando  ~");
    Display.setCursor(0,1);
    Display.print("~   conexion   ~");
    WiFi.begin(SSID, PASS);
    i = 0;
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        i++;
        if(i>8){
            break;
        }
        _TiempoDeEspera = millis();
        while(millis() < (500 + _TiempoDeEspera));
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.println();
        Serial.println("Conexion a WiFi exitosa");
        Serial.println();
        Serial.print("Tu IP es: ");
        Serial.println(WiFi.localIP());
        Serial.println();
        Display.clear();
        Display.home();
        Display.print("  Conectado a");
        Display.setCursor(0,1);
        Display.print("    internet");

        /* Habilitamos para hacer ping */
        _PingConexionInternet = true;

         /* Tiempo de espera */
        _TiempoDeEspera = millis();
        while(millis() < (1200 + _TiempoDeEspera));

    }else{
        Serial.println();
        Serial.println("No se logro a conexion al WiFi :(");
        Serial.print("Error: ");
        Serial.println(WiFi.status());
        Serial.println();
        Display.clear();
        Display.home();
        Display.print("  No se logro");
        Display.setCursor(0,1);
        Display.print(" la conexion :(");
        /* Tiempo de espera */
        _TiempoDeEspera = millis();
        while(millis() < (1200 + _TiempoDeEspera));
    }

    /* Conexion a la base de datos */
    conectToFirebase();

}

void DispensadorV2::conectToFirebase(){
    if(statusWiFi()){
    
        Serial.println("Conectando con la base de datos...");
       
        Serial.println("Inicializando firebase...");
        Firebase.begin(DATABASE_URL, SECRET_DB); // Crear objeto configuracion y autentificarse
        
        Serial.println("Configurando la reconeccion...");
        Firebase.reconnectWiFi(true);

        if(!Firebase.beginStream(firebaseData, String(Nodo + _Usuario))){
            Serial.println("No se establecio conexion con Firebase");
        }else{
            Serial.println("Se logro la conexion a Firebase!!!");
        }
    }
}

/* Utiliza la funcion Ping para verificar que la red este conectado a internet */
bool DispensadorV2::statusWiFi(){
   
    _TiempoDeEspera = millis();
    _PingConexionInternet = _PingConexionInternet?Ping.ping(HOST, 1):false;

    /* Hacemos un timeout para el ping si se tarda mas de 50ms da por hecho que no hay internet */
    Serial.print("Tiempo: ");
    Serial.println((millis()-_TiempoDeEspera)/1000.0000);
    if(_PingConexionInternet){
        if (millis() - _TiempoDeEspera >= 200){
            Serial.println("Estado de ESP32: No conectado a internet");
            _StatusWiFi = false;
        }else{
            Serial.println("Estado de ESP32: Conectado a internet");
            _StatusWiFi = true;
        }
    }else{
        Serial.println("Estado de ESP32: No conectado a internet");
        _StatusWiFi = false;
    }
    return _StatusWiFi;
}

bool DispensadorV2::isConexionInternet(){
    return _PingConexionInternet;
}

bool DispensadorV2::reconectToInternet(){
    if(millis() > _TiempoDeReconexion + TiempoDeReconexion){
        if(WiFi.status() == WL_CONNECTED){
            Serial.println("Verificando conexion");
            _PingConexionInternet = Ping.ping(HOST, 1)?true:false;

            if(_PingConexionInternet){
                Serial.println("Reconexion a internet exitosa");
            }else{
                Serial.println("No hay respuesta del servidor");
            }
            
        }else{
            Serial.println("~ Red desconectada ~");
            Serial.println("Intentando reconexion...");
            if(WiFi.reconnect()){
                Serial.println("Se establecion conexion con la red");
            }else{
                Serial.println("No fue posible conectar con la red");
            }
        }
        _TiempoDeReconexion = millis();
    }
    return _PingConexionInternet;
}


void DispensadorV2::isDataBaseChange(){
    if(millis() > _TiempoParaLecturaDB + TiempoDeLectura){
        if(statusWiFi()){
            if(Firebase.readStream(firebaseData)){
                //Serial.println("Lectura de DB exitosa");
                if(firebaseData.dataAvailable()){
                    //Serial.println(firebaseData.dataType());
                    //Serial.println(firebaseData.dataPath());
                    if(firebaseData.dataType().equals("int") || firebaseData.dataType().equals("float") || firebaseData.dataType().equals("string") || firebaseData.dataType().equals("boolean")){
                    
                        Firebase.getJSON(firebaseData, String(Nodo + _Usuario));
                        firebaseJsonData = firebaseData.jsonObject();
                        firebaseJsonData.toString(_DispensadorJsonStr, false);
                        //Serial.println("Json en firebase:");
                        //Serial.println(_DispensadorJsonStr);
                        setDispensadorData(_DispensadorJsonStr);
                        
                    }
                    
                }
            }else{
                Serial.println("No se pudo leer la DB :(");
            }
        }
        _TiempoParaLecturaDB = millis();
    }
}

void DispensadorV2::actualizarFirebase(){
    if(statusWiFi()){
        // _DispensadorJsonStr
        firebaseJsonData.setJsonData(_DispensadorJsonStr);
        //Firebase.setJSON(firebaseData, String(Nodo + _Usuario), firebaseJsonData);
        Serial.println("Set Json Async Firebase");
        Firebase.setJSONAsync(firebaseData, String(Nodo + _Usuario), firebaseJsonData);
    }
}

void DispensadorV2::despacharProducto(){
    if((_ProductoSeleccionado != 255) && (_ProductoSeleccionado != 72)){
        if(_Credito > 0){
            if(verificarProductoDisponible()){
                /** Calculo de tiempo y producto a despachar**/
                _TiempoADespachar = _Credito*_Productos[_ProductoSeleccionado].getTiempo()/_Productos[_ProductoSeleccionado].getCosto();
                Serial.print("Tiempo a despachar: "); Serial.print(_TiempoADespachar/1000.00); Serial.println(" Seg");
                _ProductoADespachar = ((_Credito*1)/_Productos[_ProductoSeleccionado].getCosto());
                Serial.print("Cantidad a despachar: "); Serial.print(_ProductoADespachar); Serial.println(" Litros");
                Display.clear();
                Display.home();
                Display.print("Despachando...");
                Display.setCursor(9,1);
                Display.print("%");
                
                /* Botones del 1 al 8 */
                if(_ProductoSeleccionado < 8){
                    i = 0;
                    Serial.print("Despachando ");
                    Serial.print(_Productos[_ProductoSeleccionado].getNombre());
                    Serial.print(" -> [");
                    // Las salidas estan negadas LOW significa prender la bomba
                    Bombas.write(_ProductoSeleccionado, LOW);
                    tiempoAnterior = millis();
                    while(millis() < (_TiempoADespachar + tiempoAnterior)){
                        // Modificar LCD
                        Display.setCursor(6, 1);
                        Display.print(map((millis()-tiempoAnterior), 0, _TiempoADespachar, 0, 100));
                        if(map((millis()-tiempoAnterior), 0, _TiempoADespachar, 0, 100) == i){
                            Serial.print("*");
                            i+=10;
                        }
                    }
                    Bombas.write(_ProductoSeleccionado, HIGH);
                    Serial.println("]"); Serial.println();
                }
                
                /* Botones del 9 al 16*/
                if(_ProductoSeleccionado >7){
                    i = 0;
                    Serial.print("Despachando ");
                    Serial.print(_Productos[_ProductoSeleccionado].getNombre());
                    Serial.print(" -> [");
                    _ProductoSeleccionado-=8;
                    Bombas2.write(_ProductoSeleccionado, LOW);
                    tiempoAnterior = millis();
                    while(_TiempoADespachar > (millis() - tiempoAnterior)){
                        // Modificar LCD
                        Display.setCursor(6, 1);
                        Display.print(map((millis()-tiempoAnterior), 0, _TiempoADespachar, 0, 100));
                        if(map((millis()-tiempoAnterior), 0, _TiempoADespachar, 0, 100) == i){
                            Serial.print("*");
                            i+=10;
                        }
                    }
                    Bombas2.write(_ProductoSeleccionado, HIGH);
                    Serial.println("]"); Serial.println();
                    _ProductoSeleccionado+=8;
                }

                /** Actualizacion del producto**/

                // Cantidad
                _Productos[_ProductoSeleccionado].setCantidad(_Productos[_ProductoSeleccionado].getCantidad() - _ProductoADespachar);
                // Balance del producto
                _Productos[_ProductoSeleccionado].setBalance(_Productos[_ProductoSeleccionado].getBalance()+_Credito);
                // Balance del dispensador
                _Balance += _Credito;

                /* Guardamos el valor de la ultima compra */
                _UltimoCredito = _Credito;

                /** Borramos el credito de esa compra **/
                _Credito = 0;

                /* Guardamos los nuevos valores */
                saveData();

                /* Mensaje de gracias */
                Display.clear();
                Display.home();
                Display.print("Gracias por");
                Display.setCursor(0,1);
                Display.print("tu compra !!!");

                /* Actualizar base de datos de firebase */
                actualizarFirebase();

                /* Tiempo de espera */
                if(isConexionInternet()){
                    _TiempoDeEspera = millis();
                    while(millis() < (200 + _TiempoDeEspera));
                }else{
                    _TiempoDeEspera = millis();
                    while(millis() < (1500 + _TiempoDeEspera));
                }

                /* Mensaje de Bienvenido */
                mensajeDeBienvenida(); 

                /* Mostrar credito disponible */
                mostrarCredito();


            }else{ 
                /** No hay suficiente producto **/
                Display.clear();
                Display.home();
                Display.print("No hay ");
                Display.print(_Productos[_ProductoSeleccionado].getNombre());
                Display.setCursor(7,1);
                Display.write(1);
                Display.write(1);
                
                /* Tiempo de espera */
                tiempoAnterior = millis();
                while(millis() < (1500 + tiempoAnterior));

                /** Mostrar mensaje inicial **/
                mensajeDeBienvenida();
                mostrarCredito();
            }
        }else{
            //Serial.print(_Productos[_ProductoSeleccionado].getNombre());
            Serial.print(" Disponible: ");
            Serial.print(_Productos[_ProductoSeleccionado].getCantidad());
            Serial.print(" L");
            Serial.println("");
            /** Mostrar liquido disponible **/
            Display.clear();
            Display.home();
            Display.print(_Productos[_ProductoSeleccionado].getNombre());
            Display.setCursor(0,1);
            Display.print("Disp: ");
            Display.print(_Productos[_ProductoSeleccionado].getCantidad());
            Display.print(" L");
            
            /* Tiempo de espera */
            tiempoAnterior = millis();
            while(millis() < (1500 + tiempoAnterior));

            /** Mostrar mensaje inicial **/
            mensajeDeBienvenida();
            mostrarCredito();
        }
    }
}

/**
 * @brief Para reiniciar la terminal de interrupcion del PCF8574 se tiene que leer o escribir
 *        con la funcion selectAll() pone en alto a todas las terminales del PCF8574 asi 
 *        evita que se quede enclavado por el posible ruido del boton y que no detecte las 
 *        demas interrpciones.
 */
void DispensadorV2::resetPCF(){
    Botones.selectAll();
    Botones2.selectAll();
}

void DispensadorV2::readBotones(){
    if(Botones.read8() != 255){
        _ProductoSeleccionado = Botones.value();
        Botones.selectAll();
        switch (_ProductoSeleccionado){
        case 254:
            _ProductoSeleccionado = 0;
            break;
        case 253:
            _ProductoSeleccionado = 1;
            break;
        case 251:
            _ProductoSeleccionado = 2;
            break;
        case 247:
            _ProductoSeleccionado = 3;
            break;
        case 239:
            _ProductoSeleccionado = 4;
            break;
        case 223:
            _ProductoSeleccionado = 5;
            break;
        case 191:
            _ProductoSeleccionado = 6;
            break;
        case 127:
            _ProductoSeleccionado = 7;
            break;
        default:
            break;
        }
        Serial.println();
        Serial.print("Producto: ");
        Serial.println(_ProductoSeleccionado+1);
        //Serial.println(_Productos[_ProductoSeleccionado].getNombre());
    }
    if(Botones2.read8() != 255){
        _ProductoSeleccionado = Botones2.value();
        Botones2.selectAll();
        switch (_ProductoSeleccionado){
        case 254:
            _ProductoSeleccionado = 8;
            break;
        case 253:
            _ProductoSeleccionado = 9;
            break;
        case 251:
            _ProductoSeleccionado = 10;
            break;
        case 247:
            _ProductoSeleccionado = 11;
            break;
        case 239:
            _ProductoSeleccionado = 12;
            break;
        case 223:
            _ProductoSeleccionado = 13;
            break;
        case 191:
            _ProductoSeleccionado = 14;
            break;
        case 127:
            _ProductoSeleccionado = 15;
            break;
        default:
            _ProductoSeleccionado = 72;
            break;
        }
        Serial.println();
        Serial.print("Producto: ");
        Serial.println(_ProductoSeleccionado);
        //Serial.println(_Productos[_ProductoSeleccionado].getNombre());
    }
    /*
    0 -> 254    1 -> 253    2 -> 251    3 -> 247
    4 -> 239    5 -> 223    6 -> 191    7 -> 127 */
    //Serial.println(Botones.read8(), BIN);
}

void DispensadorV2::mensajeCalibracion(){
    Display.clear();
    Display.home();
    Display.print("   ~ Inicia ~");
    Display.setCursor(0,1);
    Display.print("~ Calibracion ~");
}

void DispensadorV2::startCalibracion(){
    if(_ProductoSeleccionado != 255){
        Display.clear();
        Display.home();
        Display.print("Contando...");
        Serial.println();
        Serial.println("~ Inicia Calibracion ~");
        Serial.print("Midiendo tiempo: [");
        i = 1000;
        if(_ProductoSeleccionado<8){
            /* Encender bomba del 0-7*/
            Bombas.write(_ProductoSeleccionado, HIGH);
            _TiempoDeCalibracion = millis();
            while(!Botones.read(_ProductoSeleccionado)){
                Display.setCursor(6, 1);
                Display.print((millis() - _TiempoDeCalibracion)/1000.00);
                if((millis() - _TiempoDeCalibracion) >= i){
                    Serial.print("*");
                    i+=1000;
                }
            }
            /* Apagar bombga del 0-7*/
            Bombas.write(_ProductoSeleccionado, LOW);
        }
        /* Productos del 8 - 15*/
        if(_ProductoSeleccionado>7){ 
            /* Restamos 8 para empezar desde cero*/
            _ProductoSeleccionado-=8;
            /* Encender bomba del 8 - 15 */
            Bombas2.write(_ProductoSeleccionado, HIGH);
            _TiempoDeCalibracion = millis();
            while(!Botones2.read((_ProductoSeleccionado))){
                Display.setCursor(6, 1);
                Display.print((millis() - _TiempoDeCalibracion)/1000.00);
                if((millis() - _TiempoDeCalibracion) >= i){
                    Serial.print("*");
                    i+=1000;
                }
            }
            /* Apagar bomba del 8 - 15 */
            Bombas2.write(_ProductoSeleccionado, LOW);
            _ProductoSeleccionado+=8;
        }
        Serial.println("]");
        //Apagar bomba

        Serial.println();
        _Productos[_ProductoSeleccionado].setTiempo((float)(millis() - _TiempoDeCalibracion));
        Serial.print("Tiempo medido: "); Serial.println(_Productos[_ProductoSeleccionado].getTiempo());
        Serial.print("Asignado al producto: "); Serial.println(_Productos[_ProductoSeleccionado].getNombre());
        Serial.println();

        /** Mostrar en LCD **/
        Display.clear();
        Display.home();
        Display.print(_Productos[_ProductoSeleccionado].getNombre());
        Display.setCursor(0,1);
        Display.print("Tiempo: ");
        Display.print((_Productos[_ProductoSeleccionado].getTiempo())/1000);
        Display.print(" S");
        
        /* Esperamos 1.5s*/
        _TiempoDeEspera = millis();
        while(millis() < _TiempoDeEspera + 1117);


        /* Guardamos base de datos local */
        saveData();

        /* Actualizamos firebase */
        actualizarFirebase();

        /* Mostramos mensaje de calibracion */
        mensajeCalibracion();

    }
}

/** Se guarda el json en preferences **/
void DispensadorV2::saveData(){
     /** Comienzo a serializar**/
        _DispensadorJson.clear();
        _DispensadorJson["Nombre"] = _Nombre;
        _DispensadorJson["Balance"] = _Balance;
        _DispensadorJson["Credito"] = _Credito;
        _DispensadorJson["UltimoCredito"] = _UltimoCredito;
        _DispensadorJson["Usuario"] = _Usuario;
        _DispensadorJson["Calibracion"] = _Calibracion;
        
        JsonArray Productos = _DispensadorJson.createNestedArray("Productos");
        for(i=0; i<_CantidadDeProductos; i++){
            JsonObject producto = Productos.createNestedObject();

            producto["Nombre"] = _Productos[i].getNombre();
            producto["Cantidad"] = _Productos[i].getCantidad();
            producto["Costo"] = _Productos[i].getCosto();
            producto["Tiempo"] = _Productos[i].getTiempo();
            producto["Balance"] = _Productos[i].getBalance();
        }
        
        //Serial.println("~ Serializando... ~");
        _DispensadorJsonStr = "";
        serializeJson(_DispensadorJson, _DispensadorJsonStr);
        
        //Serial.println(_DispensadorJsonStr);


        if(preferences.begin(DataBase, false)){
            preferences.putString(DataBase, _DispensadorJsonStr);
            preferences.end();
        }
        Serial.println("~ Guardando base de datos ~");
}

void DispensadorV2::readData(){
    if(preferences.begin(DataBase, false)){
        //preferences.clear();
        setDispensadorData(preferences.getString(DataBase, "No hay JSON"));
        preferences.end();
    }
    Serial.println("Actualizando firebase...");
    actualizarFirebase();
}


/** Calculamos si hay suficnete producto para despachar **/
bool DispensadorV2::verificarProductoDisponible(){
    if(_Productos[_ProductoSeleccionado].getCantidad()>=((_Credito*1)/_Productos[_ProductoSeleccionado].getCosto())){//CantidadMinimaDeProducto){
        return true;
    }else{
        return false;
    }
}

void DispensadorV2::setDispensadorData(String DispensadorDataJsonStr){
    _DispensadorJsonStr = DispensadorDataJsonStr;
    Serial.println("Json a deserializar:");
    Serial.println(_DispensadorJsonStr); Serial.println();Serial.println();
    if(!_DispensadorJsonStr.equals("No hay JSON")){
        /** Deserializar el Json para convertirlo en objeto **/
        DeserializationError error = deserializeJson(_DispensadorJson, _DispensadorJsonStr);
        if(error == DeserializationError::Ok){
            Serial.println("~ Todo bien al deserializar :D ~"); Serial.println();
            Serial.println("~ Informacion del dispensador ~");
           
            _Nombre = _DispensadorJson["Nombre"].as<String>();
            _Balance = _DispensadorJson["Balance"].as<int>();
            _Credito = _DispensadorJson["Credito"].as<int>();
            _Usuario = _DispensadorJson["Usuario"].as<String>();
            _Calibracion = _DispensadorJson["Calibracion"].as<bool>();
            
            i = 0;
            for(JsonObject producto : _DispensadorJson["Productos"].as<JsonArray>()) {
                _Productos[i] = Producto(
                    producto["Nombre"].as<String>(),
                    producto["Costo"].as<int>(),
                    producto["Tiempo"].as<float>(),
                    producto["Cantidad"].as<float>(),
                    producto["Balance"].as<int>()
                );
                i++;
            }

            Serial.print("Usuario: "); Serial.println(_Usuario);
            Serial.print("Nombre: "); Serial.println(_Nombre);
            Serial.print("Credito: "); Serial.println(_Credito);
            Serial.println();

            /* Guardamos nuevos valores */
            saveData();

            /** Credito agregado **/
            /* Restamos 1 credito ya que cuando _CreditoAdded es true se suma un credito */
            if(_Credito>0){
                _Credito--;
                _CreditoAdded = true;
            }else{
                _CreditoAdded = false;
            }
            
            if(_Calibracion){
                Serial.println("~~ MODO CALIBRACION ~~");
                setCalibracion(true);
                mensajeCalibracion();
                Serial.println("Esperando a elegir un producto...");
            }else{
                Serial.println("~~ MODO DESPACHADOR ~~");
                mensajeDeBienvenida();
                mostrarCredito();
            }


        }else{
            Serial.print("~ Hubo un error: ");
            Serial.print(error.f_str());
            Serial.println(" ~");
        }
    }else{
        /** Crear Base de datos para almacenarlo en local **/
        Serial.println("~ No existe base de datos ~");
        Serial.println("~ Creando una... ~");
        /** Creando objetos de productos **/
        for(i=0; i<_CantidadDeProductos; i++){
            _Productos[i].setNombre("Producto");
            _Productos[i].setCantidad(20);
            _Productos[i].setCosto(10);
            _Productos[i].setTiempo(1000); // Tiempo en mili segundos
            _Productos[i].setBalance(i);
        }

        /** Guardamos un json con los nuevos valores **/
        saveData();
        
    }
    
}

String DispensadorV2::getDispensadorData(){
    return "";
}

void DispensadorV2::mostrarCredito(){
    _Credito = _Credito<0?0:_Credito;
    Display.setCursor(6,1);
    Display.print(_Credito);     // Escribe el mensaje
    Display.print("$");
    Serial.print("Credito: ");
    Serial.println(_Credito);
    saveData();
}

void DispensadorV2::mensajeDeBienvenida(){
    Display.clear();
    Display.home();
    Display.write(0);
    Display.print(" ");
    Display.print("Bienvenido!! ");     // Escribe el mensaje
    Display.write(0);
    Display.print(" ");
}

void DispensadorV2::activarInterrupciones(){
    attachInterrupt(digitalPinToInterrupt(PinBotones), seleccionProducto, FALLING); // Cualquier flanco 
    attachInterrupt(digitalPinToInterrupt(PinMonedero), agregarCredito, FALLING); // Flanco de bajada
    
}

void DispensadorV2::desactivarInterrupciones(){
    detachInterrupt(digitalPinToInterrupt(PinMonedero));
    detachInterrupt(digitalPinToInterrupt(PinBotones));
}

void DispensadorV2::setCalibracion(bool calibracion){
    _Calibracion = calibracion;
}

bool DispensadorV2::isCalibracionActive(){
    return _Calibracion;
}
bool DispensadorV2::isCreditoAdded(){
    return _CreditoAdded;
}

void DispensadorV2::setCreditoAdded(bool creditoAdded){
    _CreditoAdded = creditoAdded;
}

bool DispensadorV2::isBotonPressed(){
    return _BotonInterrupcion;
}

void DispensadorV2::setBotonInterrupcion(bool btnInterrupcion){
    _BotonInterrupcion = btnInterrupcion;
    _ProductoSeleccionado = 255;
}

bool DispensadorV2::isBotonInterrupcion(){
    return _BotonInterrupcion;
}

void DispensadorV2::addCredito(){
    _Credito++;
}

void IRAM_ATTR agregarCredito() {
    if(millis() > (tiempoDeRebote + tiempoAnteriorRebote)){
        _CreditoAdded = true;
        tiempoAnteriorRebote = millis();
    }                                                            
}

void IRAM_ATTR seleccionProducto(){
    if(millis() > tiempoDeReboteBoton + tiempoAnteriorReboteBoton){
        _BotonInterrupcion = true;
        tiempoAnteriorReboteBoton = millis();
    }
}

void DispensadorV2::testAnyway(){
    Serial.println("SI HAY P1");
    delay(2000);

    Serial.println("No HAY P2");
    delay(1250);
    Serial.println("SI HAY P2");
    delay(1250);
    Serial.println("No HAY P2");
    delay(1250);
    Serial.println("SI HAY P2");
    delay(1250);

    Serial.println("NO HAY P1");
    delay(2000);
}