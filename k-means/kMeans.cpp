#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <vector>
#include <cstdlib>
#include <random>
#include <math.h>

using namespace std;

typedef vector<tuple<size_t, size_t>> vectorTupla;
typedef unordered_map<size_t, vectorTupla> tabla;
typedef unordered_map<size_t, double> tablaNorma;
typedef unordered_map<size_t, tuple<double, double>> tablaDistacias;

void calculoNormas(tabla &norma, tablaNorma &normas){
  double calculo = 0;
  for(const auto& n : norma)
  {
    for(int i = 0; i < n.second.size(); i++)
    {
      calculo += pow(get<1>(n.second[i]), 2);
    }
    calculo = sqrt(calculo);
    normas.insert({n.first, calculo});
  }
}

double normaCentroide(vectorTupla centroide){
  double calculo = 0;
  for(int i = 0; i < centroide.size(); i++)
  {
    calculo += pow(get<1>(centroide[i]), 2);
  }
  calculo = sqrt(calculo);
  return calculo;
}

void means(tabla &datos, tabla &centroides, tablaNorma &normas, tablaDistacias &distancias){
  size_t j, productoPunto, usuario = 0;
  tuple<double, double> mejorDistancia;
  double distancia, normaCentro = 0;
  for(const auto tCentroides : centroides){
    normaCentro = normaCentroide(tCentroides.second);
    for(const auto tDatos : datos){
      for(size_t i = 0; i < tDatos.second.size(); i++){
          j = get<0>(tDatos.second[i]);
          productoPunto += get<1>(tDatos.second[i]) * get<1>(tCentroides.second[j-1]);
      }
      usuario = tDatos.first;
      distancia = (productoPunto/(normas[usuario] * normaCentro));//Creo que falta el arcocoseno
      mejorDistancia = distancias[usuario];
      if(get<1>(mejorDistancia) == -1){
        distancias[usuario] = make_tuple(tCentroides.first, distancia);
      }
      else{
        cout << get<1>(distancias[usuario]) << " > " << distancia << endl;
        if(get<1>(distancias[usuario]) > distancia){
          cout << "entre" << endl;
          distancias[usuario] = make_tuple(tCentroides.first, distancia);
        }
      }
      distancia = 0;
      productoPunto = 0;
    }
  }
}

void generarCentroide(tabla &centroides, size_t numCentroides){
  random_device rd;
  uniform_int_distribution<int> dist(0, 5);
  vectorTupla calificaciones;
  for(size_t j = 0; j < numCentroides; j++){
    for(size_t i = 1; i <= 17770; i++){
      calificaciones.push_back(make_tuple(i, dist(rd)));
    }
    centroides.insert({j, calificaciones});
    calificaciones.clear();
  }
}

void lecturaArchivo(tabla &datos, tablaDistacias &distancias){
  ifstream archivo("netflix/combined_data_1.txt");
  char linea[256];
  string line;
  vectorTupla argumentos;
  size_t idPelicula = 0;
  size_t key = 0;
  size_t calificacion = 0;
  while(getline(archivo,line))
  //for(int i = 0; i < 100; i++)
  {
    //archivo.getline(linea, 256);
    //line = linea;
    int posicionCaracter = line.find(":");
    if(posicionCaracter != string::npos){
      idPelicula = atoi(line.substr(0, posicionCaracter).c_str());
      //cout << "id de la pelicula:  " << idPelicula << endl;
    }
    else{
        for(int j = 0; j < 2; j++){
          posicionCaracter = line.find(",");
          if(j == 0){
            key = atoi(line.substr(0, posicionCaracter).c_str());
            //cout << "id del usuario:  " << key << endl;
            line.erase(0, posicionCaracter + 1);
            //cout << "linea: " << line << endl;
          }
          else{
            calificacion = atoi(line.substr(0, posicionCaracter).c_str());
            //cout << "calificacion:  " << calificacion << endl;
            auto search = datos.find(key);
            if(search != datos.end())
            {
              //cout << "entro:" << calificacion <<"\n";
              search->second.push_back(make_tuple(idPelicula, calificacion));
            }
            else
            {
              //cout << "entro:" << calificacion <<"\n";
              argumentos.push_back(make_tuple(idPelicula, calificacion));
              datos.insert({key, argumentos});
              distancias.insert({key, make_tuple(-1, -1)});
              //keys.push_back(key);
              argumentos.clear();
            }
            //argumentos.push_back(make_tuple(idPelicula, calificacion));
          }
          //datos.insert(make_pair(key, argumentos));
        }
    }

  }
  archivo.close();
}

int main(){
  tabla datos;
  tabla centroides;
  tablaDistacias distancias;
  size_t numCentroides = 0;
  tablaNorma normas;
  lecturaArchivo(datos, distancias);
  cout << "Datos cargados..." << endl;
  calculoNormas(datos, normas);
  cout << "Normas calculadas..." << endl;
  cout << "ingrese el número de centroides: ";
  cin >> numCentroides;
  generarCentroide(centroides, numCentroides);
  means(datos, centroides, normas, distancias);
  for(const auto& n : distancias ) {
        cout << "usuario:[" << n.first << "] centroide:[" << get<0>(n.second) << "] distancia:[" << get<1>(n.second) << "]\n";
   }

  /*for(const auto& m : datos)
  {
    cout << "Key:[" << m.first << "]\n";
    for(int i = 0; i < m.second.size(); i++)
    {
      cout << "Pelicula:" << get<0>(m.second[i])<< " Calificacion:" << get<1>(m.second[i]) << '\n';
    }
  }*/
}
