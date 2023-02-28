/**
 * @file DispensadorV2.h
 * @author Edgar Dominguez (ragdedominguez8669@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-10-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <PCF8574.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Variables.h>
#include <Producto.h>
#include <FirebaseESP32.h>
#include <ESP32Ping.h>

class DispensadorV2{

  private:
  Producto _Productos[cantidadDeProductos];
  byte _CantidadDeProductos;
  bool _Calibracion = false;
  int _UltimoCredito = 0;
  String _DispensadorJsonStr;
  StaticJsonDocument<5000> _DispensadorJson;
  String getDispensadorData();
  int _ProductoSeleccionado;
  bool verificarProductoDisponible();
  unsigned long _TiempoADespachar;
  unsigned long _TiempoDeCalibracion;
  unsigned long _TiempoParaLecturaDB = 0;
  unsigned long _TiempoDeEspera = 0;
  unsigned long _TiempoDeReconexion = 0;
  float _ProductoADespachar;
  void saveData();
  int i;
  void conecToWiFi();
  String _Usuario;
  FirebaseData firebaseData;
  FirebaseJson firebaseJsonData;
  FirebaseAuth auth;
  FirebaseConfig config;
  bool _PingConexionInternet = true;
  bool _StatusWiFi = false;


  public:
  /* Cantidad de productos del dispensador */
  DispensadorV2(String, String, byte);
  void begin();
  String _Nombre;
  int _Balance = 0;

  void readData();
  void setDispensadorData(String);
  bool isCalibracionActive();
  void setCalibracion(bool);
  void addCredito();
  bool isCreditoAdded();
  void setCreditoAdded(bool);
  bool isBotonPressed();
  void setBotonInterrupcion(bool);
  bool isBotonInterrupcion();
  void activarInterrupciones();
  void desactivarInterrupciones();
  void mensajeDeBienvenida();
  void mostrarCredito();
  void mensajeCalibracion();
  void readBotones();
  void despacharProducto();
  void resetPCF();
  void startCalibracion();
  bool statusWiFi();
  void isDataBaseChange();
  void actualizarFirebase();
  bool isConexionInternet();
  bool reconectToInternet();
  void conectToFirebase();

  void testAnyway();
};

