#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <vector>
#include <cstdlib>
#include <random>
#include <math.h>
#include <limits>
#include <queue>
#include <zmqpp/zmqpp.hpp>
#include "timer.hh"

using namespace std;
using namespace zmqpp;

typedef vector<tuple<size_t, double>> vectorTupla;


//Funcion que se encarga de buscar y organizar los k's con su respectivo error, se realiza busqueda binaria
void vectorK(vectorTupla &indiceK,size_t numCentroides, bool &parada ,double sumaDistancias, double &kAnterior, double &kSiguiente, double &kActual){
  //si el vector inicialmente es vacio añade el primer elemento
  if(indiceK.size() < 1){
    indiceK.push_back(make_tuple(numCentroides,sumaDistancias));
    kAnterior = -1;
    kSiguiente = -1;
    kActual = 0;
  }
  else{
    int Iarriba = indiceK.size();
    int Iabajo = 0;
    int Icentro,  indice = 0;
    while(Iabajo <= Iarriba){
      //se halla el centro entre el limite superior e inferior
      Icentro = ceil((Iarriba + Iabajo)/2);
      //se compara que el kActual que se esta examinando no se encuentre otra vez condishion de parada
      if(get<0>(indiceK[Icentro]) == numCentroides){
        parada = true;
        break;
      }
      if(abs(Iarriba - Iabajo) == 1 or abs(Iarriba - Iabajo) == 0){
        //si es 0 es porque esta posicionado bien sino es en el anterior a su posicion
        if(abs(Iarriba - Iabajo) == 0){
          indice = Iarriba;
        }
        else{
          indice = Iarriba-1;
        }
        //cuando no encuentra agrega al inicio sino agrega en el indice correspondiente
        if(get<0>(indiceK[indice]) > numCentroides){
          if(indice == 0){
            indiceK.insert(indiceK.begin(), make_tuple(numCentroides,sumaDistancias));
            kAnterior = -1;
            kSiguiente = 1;
            kActual = 0;
          }
          else{
            indiceK.insert(indiceK.begin()+(indice), make_tuple(numCentroides,sumaDistancias));
            kAnterior = indice - 1;
            kSiguiente = indice + 1;
            kActual = indice;
          }
          break;
        }
        else{
          if(abs(Iarriba - Iabajo) == 1){
            //si el indice es menor al tamaño agrega dos posiciones enseguida 
            if((indice+1) < indiceK.size()){
                if(get<0>(indiceK[indice+1]) <= numCentroides){
                  indice = indice+2;
                }else{
                  indice = indice+1;
                }
            }
          }else{
            indice = Iarriba+1;
          }
            if((Iarriba+1) < indiceK.size()){
              indiceK.insert(indiceK.begin()+(indice), make_tuple(numCentroides,sumaDistancias));
              kAnterior = indice - 1;
              kSiguiente = indice + 1;
              kActual = indice;
            }else{
              indiceK.push_back(make_tuple(numCentroides,sumaDistancias));
              kAnterior = indiceK.size() - 2;
              kSiguiente = -1;
              kActual = indiceK.size()-1;
            }
          break;
        }
      }
      else{
        if(get<0>(indiceK[Icentro]) > numCentroides){
          Iarriba = Icentro-1;
        }
        else{
          Iabajo = Icentro+1;
        }
      }
    }
  }
}


void calculaK(size_t limiteK){
  	context context;
    socket server_send(context,socket_type::push);
    socket server_recv(context,socket_type::pull);
    server_send.bind("tcp://*:5557");
    server_recv.bind("tcp://*:5558");
    vector<vectorTupla> centroides;
    priority_queue<tuple<double,size_t>> colaPrioridad;
    colaPrioridad.push(make_tuple(numeric_limits<int>::max(),1));
    colaPrioridad.push(make_tuple(numeric_limits<int>::max(),limiteK));
    colaPrioridad.push(make_tuple(numeric_limits<int>::max()-1,ceil(limiteK/2)));
    size_t numCentroides = 0;
    double  kSiguiente, kAnterior, kActual, kActualAnterior, kAnteriorAnterior, kSiguienteAnterior, pPendiente, sPendiente;
    double cont = 0.0, anguloAnterior = 0.0, anguloSiguiente = 0.0, anguloPrioridad = 0.0;
    bool entrada = false, parada = false;
    vectorTupla indiceK;
    while(parada == false)
    {
      numCentroides = get<1>(colaPrioridad.top());
      colaPrioridad.pop();
      string calculo_aleatorio = to_string(numCentroides);
      zmqpp::message msg;
      msg << calculo_aleatorio;
      server_send.send(msg);
      double sumaDistancias = 0;
      zmqpp::message resultado;
      server_recv.receive(resultado);
      string x;
      resultado >> x;
      cout << "Respuesta = " << x << endl;
      sumaDistancias = atoi(x.c_str());
      //Timer t;
      vectorK(indiceK,numCentroides,parada,sumaDistancias, kAnterior, kSiguiente, kActual);

      /*for(int j = 0; j < indiceK.size(); j++){
        cout << "[ " << get<0>(indiceK[j]) << ", " << get<1>(indiceK[j]) << " ]";
      }*/
      cout  << endl;
      if(kAnterior > -1){
          //diferenciaPendiente = 0.0;
          //cout << "pPendiente "<<((double)get<1>(indiceK[kAnterior]) - (double)get<1>(indiceK[kActual]))/((double)get<0>(indiceK[kAnterior]) - (double)get<0>(indiceK[kActual]))<<endl;
          pPendiente = ((double)get<1>(indiceK[kAnterior]) - (double)get<1>(indiceK[kActual]))/((double)get<0>(indiceK[kAnterior]) - (double)get<0>(indiceK[kActual]));
      }
      if(kSiguiente < indiceK.size() and kSiguiente > -1){
          //diferenciaPendiente = 0.0;
          //cout << "sPendiente "<<((double)get<1>(indiceK[kActual]) - (double)get<1>(indiceK[kSiguiente]))/((double)get<0>(indiceK[kActual]) - (double)get<0>(indiceK[kSiguiente]))<<endl;
          sPendiente = ((double)get<1>(indiceK[kActual]) - (double)get<1>(indiceK[kSiguiente]))/((double)get<0>(indiceK[kActual]) - (double)get<0>(indiceK[kSiguiente]));
      }

      if(cont == 2){
        anguloSiguiente = abs(atan(pPendiente) - atan(sPendiente));
        if(get<0>(indiceK[kActual]) == get<0>(indiceK[kActualAnterior])){
          parada = true;
        }
        if(anguloSiguiente < anguloAnterior){
          kActual = kActualAnterior;
          kAnterior = kAnteriorAnterior;
          kSiguiente = kSiguienteAnterior;
          anguloPrioridad = anguloAnterior;
        }
        else{
          anguloPrioridad = anguloSiguiente;
        }
        //cout << "El k que se maneja: " << get<0>(indiceK[kActual]) << endl;
        //cout << "mejor angulo: "<< anguloPrioridad <<  " angulo inferior: " << anguloAnterior << " anguloSiguiente: " << anguloSiguiente << endl;
        cont = 0;
        entrada = false;
      }

      if(cont == 1){
        anguloAnterior = abs(atan(pPendiente) - atan(sPendiente));
        kActualAnterior = kActual;
        kAnteriorAnterior = kAnterior;
        kSiguienteAnterior = kSiguiente;
        cont++;
      }

      if(kSiguiente < indiceK.size() and kSiguiente > -1 and kAnterior > -1 and entrada == false){
        int mitad = ceil(abs(((double)get<0>(indiceK[kActual]) - (double)get<0>(indiceK[kAnterior]))/2));
        //size_t mitadSiguiente = ceil(abs(kSiguiente-kActual)/2);
        colaPrioridad.push(make_tuple(anguloPrioridad, (get<0>(indiceK[kActual]) - mitad)));
        colaPrioridad.push(make_tuple(anguloPrioridad, (get<0>(indiceK[kActual]) + mitad)));
        entrada = true;
        cont++;
      }
    }
    //server.close();
    cout << "El mejor k es: " << get<0>(indiceK[kActual]) << endl;
}

int main(){
	calculaK(10);
}