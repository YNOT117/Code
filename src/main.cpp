/**
 * @file Dispensador de productos de limpieza
 * @author Edgar Dominguez (Electronica117.mx)
 * @brief Dispensador
 * @version V3.0
 * @date 2021-12-17
 * @version V4.0
 * @date 2022-10-13 
 * @copyright Copyright (c) 2021
 * 
 */

#include <Arduino.h>
#include <DispensadorV2.h>

DispensadorV2 Dispensador("Edgar", nombreDelDispensador, cantidadDeProductos);
TaskHandle_t Task1;
void Task1code( void * pvParameters );

void setup(){
    Serial.begin(115200); 
    
    Dispensador.begin();
    Dispensador.readData();
    
    if(digitalRead(PinCalibracion)){
        /* Modo normal */
        Dispensador.setCalibracion(false);
        Serial.println("~~ MODO DESPACHADOR ~~");
        Dispensador.mensajeDeBienvenida();
        Dispensador.mostrarCredito();
    }else{
        /* Modo calibracion */
        //Mensaje de calibracion 
        Serial.println("~~ MODO CALIBRACION ~~");
        Dispensador.setCalibracion(true);
        Dispensador.mensajeCalibracion();
        Serial.println("Esperando a elegir un producto...");
    }
    
}

void loop(){

    /** Mostrar credito **/
    if(Dispensador.isCreditoAdded()){
        Dispensador.addCredito();
        Dispensador.mostrarCredito(); // Mostrar credito en display
        Dispensador.setCreditoAdded(false);
    }

    /** Dispachar producto **/
    if(Dispensador.isBotonPressed() && !Dispensador.isCalibracionActive()){
        Dispensador.readBotones();
        Dispensador.desactivarInterrupciones();
        Dispensador.despacharProducto();
        Dispensador.activarInterrupciones();
        Dispensador.setBotonInterrupcion(false);
    }
    Dispensador.resetPCF();

    /** Calibracion activa **/
    while(Dispensador.isCalibracionActive()){
        if(Dispensador.isBotonInterrupcion()){
            Dispensador.desactivarInterrupciones();
            Dispensador.readBotones();
            Dispensador.startCalibracion();
            Dispensador.setBotonInterrupcion(false);
            Dispensador.activarInterrupciones();
        }
        Dispensador.resetPCF();
        /** Lectura de base de datos **/
        Dispensador.isDataBaseChange();
    }

    if(!Dispensador.isConexionInternet()){
        if(Task1 == NULL){ /* Verificamos si ya existe la tarea a ejecutar en segundo plano */
            Serial.println("No existe la tarea, creando...");
            xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */     
        }
    }else{
        /** Lectura de base de datos **/
        Dispensador.isDataBaseChange();
    }

    /* Funcion para hacer pruebas de cualquier cosa */
    //Dispensador.testAnyway();

}


void Task1code( void * pvParameters ){
    unsigned long _Tiempo = 0;
    while(true){
        if(!Dispensador.isConexionInternet()){
            if(Dispensador.reconectToInternet()){
                Serial.println("Reconectado :D");
            }
        }else{
            if(millis() > _Tiempo+2000){
                Serial.println("Nada en segundo plano ;)");
                _Tiempo = millis();
            }
        }
    }
}