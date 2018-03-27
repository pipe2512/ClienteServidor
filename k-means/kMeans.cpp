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

typedef vector<tuple<size_t, double>> vectorTupla;
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
    calculo = 0;
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

void generarDatosCero(vectorTupla &calculoCentroide, vector<size_t> &cont){
	for(size_t i = 1; i <= 17770; i++)
	{
		calculoCentroide.push_back(make_tuple(i,0));
    cont[i-1] = 0;
	}

}

void calculoError(vectorTupla &calculoCentroide, tabla &centroides){
  
}

void calcula_promedio(tabla &datos, tabla &centroides, tablaDistacias &distancias, vector<size_t> &numIteracion){
	vectorTupla calculoCentroide;
  vector<size_t> cont(17770);
	double x = 0.0;
	for(const auto recorreCentroide: centroides)
	{
		generarDatosCero(calculoCentroide, cont);
		for(const auto recorreDistancia: distancias)
		{
			if(get<0>(recorreDistancia.second) == recorreCentroide.first)
			{
				vectorTupla datos_usuario = datos[recorreDistancia.first];
				for(size_t i = 0; i < datos_usuario.size(); i++)
				{
					get<1>(calculoCentroide[(get<0>(datos_usuario[i]))-1]) += get<1>(datos_usuario[i]);
          cont[(get<0>(datos_usuario[i]))-1]++;
				}
			}
		}
    cout << "centroide: " << recorreCentroide.first << endl;
    for(int i = 0; i < calculoCentroide.size(); i++){
      if(cont[i] != 0){
        cout << get<1>(calculoCentroide[i]) << "/" << cont[i] << endl;
        get<1>(calculoCentroide[i]) = get<1>(calculoCentroide[i]) / cont[i];
      }
    }
    calculoCentroide.clear();
	//aca va el error esa es la idea
	}
}

void means(tabla &datos, tabla &centroides, tablaNorma &normas, tablaDistacias &distancias, size_t numCentroides){
  size_t j, productoPunto = 0, usuario = 0;
  tuple<double, double> mejorDistancia;
  vector<size_t> numIteracion(numCentroides, 0);
  double distancia, normaCentro = 0;
  for(const auto tCentroides : centroides){
    normaCentro = normaCentroide(tCentroides.second);
    for(const auto& tDatos : datos){
      for(size_t i = 0; i < tDatos.second.size(); i++){
          j = get<0>(tDatos.second[i]);
          productoPunto += get<1>(tDatos.second[i]) * get<1>(tCentroides.second[j-1]);
      }
      usuario = tDatos.first;
      distancia = acos(productoPunto/(normas[usuario] * normaCentro));//PREGUNTAR SI ES EN RAD O EN DEG
      mejorDistancia = distancias[usuario];
      if(get<1>(mejorDistancia) == -1){
        distancias[usuario] = make_tuple(tCentroides.first, distancia);
        numIteracion[tCentroides.first]++;
      }
      else{//REVISAR BIEN EL CALCULO DE LAS DISTANCIAS.........................
        if(get<1>(distancias[usuario]) > distancia){
          numIteracion[(get<0>(distancias[usuario]))]--;
          numIteracion[tCentroides.first]++;
          distancias[usuario] = make_tuple(tCentroides.first, distancia);
        }
      }
      distancia = 0;
      productoPunto = 0;
    }
  }
  calcula_promedio(datos, centroides, distancias, numIteracion);
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
  //ifstream archivo("netflix/combined_data_1.txt");
  ifstream archivo("netflix/prueba.txt");
  char linea[256];
  string line;
  vectorTupla argumentos;
  size_t idPelicula = 0;
  size_t key = 0;
  double calificacion = 0;
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
  means(datos, centroides, normas, distancias, numCentroides);
  /*for(const auto& n : distancias ) {
        cout << "usuario:[" << n.first << "] centroide:[" << get<0>(n.second) << "] distancia:[" << get<1>(n.second) << "]\n";
   }*/

  /*for(const auto& m : datos)
  {
    cout << "usuario:[" << m.first << "]\n";
    for(int i = 0; i < m.second.size(); i++)
    {
      cout << "Pelicula:" << get<0>(m.second[i])<< " Calificacion:" << get<1>(m.second[i]) << '\n';
    }
  }*/
}
