
//  Edgar Antonio Domínguez Ramírez 
#include <Arduino.h>

class Producto {
  private:
  String Nombre;
  int Costo;
  float Tiempo;  // Tiempo en mili segundos
  float Cantidad;
  int Balance;
  
  public:   
  void operator =(Producto);
  Producto();
  /**
   * @brief Construct a new Producto object
   * Nombre, Costo, Tiempo, Cantidad, Balance
   */
  Producto(String, int, float, float, int);
  void setNombre(String);
  void setCosto(int);
  void setTiempo(float);
  void setCantidad(float);
  void setBalance(int);

  String getNombre();
  int getCosto();
  float getTiempo();
  float getCantidad();
  int getBalance();
};
