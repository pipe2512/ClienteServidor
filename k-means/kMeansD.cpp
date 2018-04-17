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

void vectorK(vectorTupla &indiceK,size_t numCentroides,double sumaDistancias){
  if(indiceK.size() < 1){
    indiceK.push_back(make_tuple(numCentroides,sumaDistancias));
  }
  else{
    int Iarriba = indiceK.size();
    int Iabajo = 0;
    int Icentro;
    while(Iabajo <= Iarriba){
      //cout << "comparacion del while: " << Iabajo << " <= " << Iarriba << endl;
      Icentro = (Iarriba + Iabajo)/2;
      cout << "comparacion salida: " << Iabajo << " - " << Iarriba << endl;
      if(abs(Iarriba - Iabajo) == 1 or abs(Iarriba - Iabajo) == 0){
        if(get<0>(indiceK[Iarriba]) > numCentroides){
          if(Iarriba == 0){
            cout << "guardandose en la posicion: " << Iarriba << endl;
            indiceK.insert(indiceK.begin(), make_tuple(numCentroides,sumaDistancias));
          }
          else{
            cout << "guardandose en la posicion: " << Iarriba-1 << endl;
            indiceK.insert(indiceK.begin()+(Iarriba-1), make_tuple(numCentroides,sumaDistancias));
          }
          break;  
        }
        else{
          if((Iarriba+1) < indiceK.size()){
            cout << "guardandose en la posicion: " << Iarriba-1 << endl;
            indiceK.insert(indiceK.begin()+(Iarriba+1), make_tuple(numCentroides,sumaDistancias));   
          }else{
            cout << "guardandose en la posicion: " << "en la ultima" << endl;
            indiceK.push_back(make_tuple(numCentroides,sumaDistancias));
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
    colaPrioridad.push(make_tuple(numeric_limits<int>::max(),ceil(limiteK/2)));
    size_t numCentroides = 0;
    vectorTupla indiceK;
    while(colaPrioridad.size() > 0)
    {
      numCentroides = get<1>(colaPrioridad.top());
      colaPrioridad.pop();
      generarCentroidePuntos(centroides, numCentroides, vectorVectores);
      double sumaDistancias = 0;
      //Timer t;
      sumaDistancias = means(vectorVectores, centroides, normas, distancias, numCentroides);
      /*cout << sumaDistancias << endl;
      cout << "tiempo: " << t.elapsed() << endl;*/
      centroides.clear();
    }
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
  /*unordered_map<size_t, size_t> datos;
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
  cin >> limiteK;*/
  random_device rd;
  uniform_int_distribution<int> dist(0, 100);
  vectorTupla indiceK;
  for(int i = 0; i < 10; i++){
    size_t k = dist(rd);
    cout << "la K que se envia: " << k << endl;
    vectorK(indiceK,k,11111);
    cout << "[ ";
    for(int j = 0; j < indiceK.size(); j++){
      cout << get<0>(indiceK[j]) << ", ";
    }
    cout << " ]" << endl;
  }

  //calculaK(limiteK,vectorVectores,distancias,normas);
}
