#include "Producto.h"

void Producto::operator=(Producto producto){
  Nombre = producto.getNombre();
  Costo = producto.getCosto();
  Tiempo = producto.getTiempo();
  Cantidad = producto.getCantidad();
  Balance = producto.getBalance();
}

Producto::Producto(){}

Producto::Producto(String nombre, int costo, float tiempo, float cantidad, int balance){
  Nombre = nombre;
  Costo = costo;
  Tiempo = tiempo;
  Cantidad = cantidad;
  Balance = balance;
}

void Producto::setNombre(String nombre){
  Nombre = nombre;
}

void Producto::setCosto(int costo){
  Costo = costo;
}

void Producto::setTiempo(float tiempo){
  Tiempo = tiempo;
}

void Producto::setCantidad(float cantidad){
  Cantidad = cantidad;
}

void Producto::setBalance(int balance){
  Balance = balance;
}

String Producto::getNombre(){
  return Nombre;
}

int Producto::getCosto(){
  return Costo;
}

float Producto::getTiempo(){
  return Tiempo;
}

float Producto::getCantidad(){
  return Cantidad;
}

int Producto::getBalance(){
  return Balance;
}