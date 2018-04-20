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
#include "timer.hh"

using namespace std;

typedef vector<tuple<size_t, double>> vectorTupla;
typedef unordered_map<size_t, double> tablaNorma;
typedef unordered_map<size_t, tuple<double, double>> tablaDistacias;

//Funcion que calcula las normas de los usuarios guardandolos en la tabla de normas
//NOTA: no se calcula la raiz ya que genera error en las aproximaciones
void calculoNormas(vector<vectorTupla> &vectorVectores, tablaNorma &normas){
  double calculo = 0;
  for(int j = 0; j < vectorVectores.size(); j++)
  {
    for(int i = 0; i < vectorVectores[j].size(); i++)
    {
      calculo += pow(get<1>(vectorVectores[j][i]), 2);
    }
    normas.insert({j, calculo});
    calculo = 0;
  }
}

//Funcion que calcula las normas de cada uno de los centroides retornando asi su norma
//NOTA: no se calcula la raiz ya que genera error en las aproximaciones
double normaCentroide(vectorTupla centroide){
  double calculo = 0;
  for(int i = 0; i < centroide.size(); i++)
  {
    calculo += pow(get<1>(centroide[i]), 2);
  }
  return calculo;
}

//Funcion que permite llenar un centroide en todas sus posiciones con ceros y la pelicula
void generarDatosCero(vectorTupla &calculoCentroide){
	for(size_t i = 1; i <= 17770; i++)
	{
		calculoCentroide.push_back(make_tuple(i,0));
	}
}

//Funcion que verifica que un centroide calculado a partir de los promedios no sea 0
//NOTA: en caso de que sea 0 asigna un punto aleatorio para cambiar su posicion
void verifica(vectorTupla &calculoCentroide, tablaDistacias &distancias, vector<vectorTupla> &vectorVectores, double actual){
  double normaCalculoCentroide = normaCentroide(calculoCentroide);
  if(normaCalculoCentroide == 0)
  {
    random_device rd;
    uniform_int_distribution<int> dist(0, distancias.size());
    int indice = dist(rd);
    for(int i = 0; i < vectorVectores[indice].size(); i++)
    {
      int indicePelicula = get<0>(vectorVectores[indice][i]);
      get<1>(calculoCentroide[indicePelicula-1]) = get<1>(vectorVectores[indice][i]);
    }
    get<0>(distancias[indice]) = actual;
  }
}

//Funcion que calcula la suma de todos los ponderados que dieron por cada centroide
double calculoError(tablaDistacias &distancias, size_t numCentroides, double &sumaDistancias){
  double error = 0.0;
  vector<double> grupos(numCentroides,0);
  vector<double> sumas(numCentroides,0);
  for(const auto recorreDistancia: distancias){
    grupos[get<0>(recorreDistancia.second)] += 1;
    sumas[get<0>(recorreDistancia.second)] += get<1>(recorreDistancia.second);
  }

  for(int i = 0; i < grupos.size(); i++)
  {
    sumaDistancias += sumas[i];
    error += (sumas[i]/grupos[i]);
  }
  return error;
}

//Funcion que genera el nuevo centroide apartir del promedio de las distancias y los usuarios
double calculaPromedio(vector<vectorTupla> &vectorVectores, vectorTupla &Centroide, tablaDistacias &distancias, double idCentroide, double normaCentorides){
	vectorTupla calculoCentroide;
	//vector<size_t> cont(17770,0);
	generarDatosCero(calculoCentroide);
	double error = 0.0, cont = 0;
	//este for recorre las mejores distancias que tenga asignado el centroide
	for(const auto recorreDistancia: distancias){
		if(get<0>(recorreDistancia.second) == idCentroide){
			vectorTupla datos_usuario = vectorVectores[recorreDistancia.first];
      //se recorren las peliculas y se suman para sacar el promedio
      //NOTA:recordar que son vectores dispersos
			for(size_t i = 0; i < datos_usuario.size(); i++){
				get<1>(calculoCentroide[(get<0>(datos_usuario[i]))-1]) += get<1>(datos_usuario[i]);
        //cont[(get<0>(datos_usuario[i]))-1]++;
			}
      cont += 1;
		}
	}
	for(int i = 0; i < calculoCentroide.size(); i++){
      if(cont != 0){
        get<1>(calculoCentroide[i]) = ceil(get<1>(calculoCentroide[i]) / cont);
      }
    }
    verifica(calculoCentroide,distancias,vectorVectores,idCentroide);
    Centroide = calculoCentroide;
}

//Funcion que permite calcular la similaridad de angulosque existe entre los centroides y usuarios almacenando el menor en la tabla de distancias
void distanciasMeans(tablaDistacias &distancias, vector<vectorTupla> &usuario, vectorTupla &centroide_iterado, tablaNorma &normaUsuario, double &normaCentorides, int &centroide_actual){
  size_t m = 0, productoPunto = 0;
  double distancia = 0.0;
  for(int i = 0; i < usuario.size(); i++)
  {
    for(int j = 0; j < usuario[i].size(); j++)
    {
      m = get<0>(usuario[i][j]);
      productoPunto += get<1>(usuario[i][j]) * get<1>(centroide_iterado[m-1]);
    }
    distancia = acos(productoPunto/sqrt(normaUsuario[i] * normaCentorides));
    #pragma omp critical
    {
      if(get<1>(distancias[i]) > distancia){
        distancias[i] = make_tuple(centroide_actual,distancia);
      }
    }
    productoPunto = 0;
  }
}

//Funcion que permite reiniciar la tabla de distancias para realizar nuevos calculos
void reseteaDistancia(tablaDistacias &distancias){
  double maximo = numeric_limits<int>::max();
  for(int i = 0; i < distancias.size(); i++)
  {
    distancias[i] = make_tuple(-1,maximo);
  }
}

//Funcion donde se realiza la magia
double means(vector<vectorTupla> &vectorVectores, vector<vectorTupla> &centroides, tablaNorma &normas, tablaDistacias &distancias, size_t numCentroides){
  double error = 0.0;
  vector<double>  normaCentorides(numCentroides, 0);
  tuple<double,double> distancia;
  double errorAnterior = 0.0, sumaDistancias = 0;
  double salida = 1.0;
  int cont = 0;
  while(salida > 0.087){
    errorAnterior = error;
    error = 0.0;
    reseteaDistancia(distancias);

    #pragma omp parallel for
    //for que calcula las distancias iterando sobre cada centroide
    for(int i = 0; i < centroides.size(); i++)
    {
      normaCentorides[i] = normaCentroide(centroides[i]);
      distanciasMeans(distancias,vectorVectores,centroides[i],normas,normaCentorides[i],i);
    }
    #pragma omp parallel for
    //for  que calcula los promedios iterando sobre cada centroide
  	for(int i = 0; i < centroides.size(); i++){
  		//como es una suma no hay problemas con la condicion de carrera
  		calculaPromedio(vectorVectores,centroides[i],distancias,i,normaCentorides[i]);
  	}
    sumaDistancias = 0;
    error = calculoError(distancias,numCentroides, sumaDistancias);
    salida = abs(error - errorAnterior);
  }
  return sumaDistancias;
}

//Funcion que genera los centroides tomando de manera aleatoria los puntos existentes
void generarCentroidePuntos(vector<vectorTupla> &centroides, size_t numCentroides, vector<vectorTupla> &vectorVectores){
  random_device rd;
  uniform_int_distribution<int> dist(0, vectorVectores.size() - 1);
  for(size_t i = 0; i < numCentroides; i++){
    size_t indice = dist(rd);
    vectorTupla calificaciones;
    generarDatosCero(calificaciones);
    for(size_t j = 0; j < vectorVectores[indice].size(); j++){
      int indicePelicula = get<0>(vectorVectores[indice][j]);
      get<1>(calificaciones[indicePelicula-1]) = get<1>(vectorVectores[indice][i]);
    }
    centroides.push_back(calificaciones);
  }

}

void vectorK(vectorTupla &indiceK,size_t numCentroides, bool &parada ,double sumaDistancias, double &kAnterior, double &kSiguiente, double &kActual){
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
      Icentro = ceil((Iarriba + Iabajo)/2);
      if(get<0>(indiceK[Icentro]) == numCentroides){
        cout << "la k que se repetia es: " << get<0>(indiceK[Icentro]) << endl; 
        parada = true;
        break;
      }
      if(abs(Iarriba - Iabajo) == 1 or abs(Iarriba - Iabajo) == 0){
        if(abs(Iarriba - Iabajo) == 0){
          indice = Iarriba;
        }
        else{
          indice = Iarriba-1;
        }
        if(get<0>(indiceK[indice]) == numCentroides){
          parada = true;
          break;
        }
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
          if(get<0>(indiceK[indice]) == numCentroides){
            parada = true;
            break;
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


void calculaK(size_t limiteK, vector<vectorTupla> &vectorVectores,tablaDistacias &distancias,tablaNorma &normas){
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
    while(colaPrioridad.size() > 0 && parada == false)
    {
      numCentroides = get<1>(colaPrioridad.top());
      colaPrioridad.pop();
      generarCentroidePuntos(centroides, numCentroides, vectorVectores);
      double sumaDistancias = 0;
      //Timer t;
      sumaDistancias = means(vectorVectores, centroides, normas, distancias, numCentroides);
      vectorK(indiceK,numCentroides,parada,sumaDistancias, kAnterior, kSiguiente, kActual);

      for(int j = 0; j < indiceK.size(); j++){
        cout << "[ " << get<0>(indiceK[j]) << ", " << get<1>(indiceK[j]) << " ]";
      }
      cout  << endl;
      if(kAnterior > -1){
          //diferenciaPendiente = 0.0;
          pPendiente = ((double)get<1>(indiceK[kAnterior]) - (double)get<1>(indiceK[kActual]))/((double)get<0>(indiceK[kAnterior]) - (double)get<0>(indiceK[kActual]));
      }
      if(kSiguiente < indiceK.size() and kSiguiente > -1){
          //diferenciaPendiente = 0.0;
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
        cout << "El k que se maneja: " << get<0>(indiceK[kActual]) << endl;
        cout << "mejor angulo: "<< anguloPrioridad <<  " angulo inferior: " << anguloAnterior << " anguloSiguiente: " << anguloSiguiente << endl;
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

      /*cout << sumaDistancias << endl;
      cout << "tiempo: " << t.elapsed() << endl;*/
      centroides.clear();
    }
    cout << "El mejor k es: " << get<0>(indiceK[kActual]) << endl;
}


//Funcion donde se lee el archivo y se crean de manera inicial los datos para trabajar
void lecturaArchivo(unordered_map<size_t, size_t> &datos, tablaDistacias &distancias, vector<vectorTupla> &vectorVectores){
  ifstream archivo("netflix/combined_data_1.txt");
  //ifstream archivo("netflix/prueba.txt");
  char linea[256];
  string line;
  vectorTupla argumentos;
  size_t idPelicula = 0;
  size_t key = 0;
  double calificacion = 0;
  while(getline(archivo,line))
  {
    int posicionCaracter = line.find(":");
    if(posicionCaracter != string::npos){
      idPelicula = atoi(line.substr(0, posicionCaracter).c_str());
    }
    else{
        for(int j = 0; j < 2; j++){
          posicionCaracter = line.find(",");
          if(j == 0){
            key = atoi(line.substr(0, posicionCaracter).c_str());
            line.erase(0, posicionCaracter + 1);
          }
          else{
            calificacion = atoi(line.substr(0, posicionCaracter).c_str());
            auto search = datos.find(key);
            if(search != datos.end())
            {
              vectorVectores[search->second].push_back(make_tuple(idPelicula, calificacion));

            }
            else
            {
              argumentos.push_back(make_tuple(idPelicula, calificacion));
              vectorVectores.push_back(argumentos);
              datos.insert({key, (vectorVectores.size() - 1)});
              double maximo = numeric_limits<int>::max();
              distancias.insert({(vectorVectores.size() - 1), make_tuple(-1, maximo)});
              argumentos.clear();
            }
          }
        }
    }

  }
  archivo.close();
}

//Funcion principal encargada de ejecutar cada coshita
int main(){
  unordered_map<size_t, size_t> datos;
  tablaDistacias distancias;
  size_t numCentroides = 0;
  vector<vectorTupla> vectorVectores;
  tablaNorma normas;
  lecturaArchivo(datos, distancias, vectorVectores);
  cout << "Datos cargados..." << endl;
  calculoNormas(vectorVectores, normas);
  cout << "Normas calculadas..." << endl;
  cout << "limite de k: ";
  size_t limiteK = 0;
  cin >> limiteK;
  calculaK(limiteK,vectorVectores,distancias,normas);
}
