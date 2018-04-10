#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <vector>
#include <cstdlib>
#include <random>
#include <math.h>
#include "timer.hh"

using namespace std;

typedef vector<tuple<size_t, double>> vectorTupla;
typedef unordered_map<size_t, vectorTupla> tabla;
typedef unordered_map<size_t, double> tablaNorma;
typedef unordered_map<size_t, tuple<double, double>> tablaDistacias;

void calculoNormas(vector<vectorTupla> &vectorVectores, tablaNorma &normas){
  double calculo = 0;
  for(int j = 0; j < vectorVectores.size(); j++)
  {
    for(int i = 0; i < vectorVectores[j].size(); i++)
    {
      calculo += pow(get<1>(vectorVectores[j][i]), 2);
    }
    //calculo = sqrt(calculo);
    normas.insert({j, calculo});
    calculo = 0;
  }
}

double normaCentroide(vectorTupla centroide){
  double calculo = 0;
  for(int i = 0; i < centroide.size(); i++)
  {
    calculo += pow(get<1>(centroide[i]), 2);
  }
  //calculo = sqrt(calculo);
  return calculo;
}

void generarDatosCero(vectorTupla &calculoCentroide){
	for(size_t i = 1; i <= 17770; i++)
	{
		calculoCentroide.push_back(make_tuple(i,0));
	}
}

double calculoError(vectorTupla &calculoCentroide, vectorTupla &centroide, double normaCentorides, tablaDistacias &distancias, vector<vectorTupla> &vectorVectores, double actual){
  double error = 0.0;
  double productoPunto  = 0.0;
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
    normaCalculoCentroide = normaCentroide(calculoCentroide);
  }
  //double norma = normaCentroide(centroide);
  for(int i = 0; i < centroide.size(); i++){
    productoPunto += get<1>(centroide[i]) * get<1>(calculoCentroide[i]);
  }
  //cout << "calculo: " << productoPunto << "/" << normaCalculoCentroide << "*" << normaCentorides;
  double calculo = productoPunto/sqrt(normaCalculoCentroide * normaCentorides);
  error = acos(calculo);
  //cout << "Print" << error <<endl;
  /*if(error <= 0.087){
    error = 0;
  }*/
  //cout << " = " << error << endl;
  return error;
}


double calculaPromedio(vector<vectorTupla> &vectorVectores, vectorTupla &Centroide, tablaDistacias &distancias, double idCentroide, double normaCentorides){
	vectorTupla calculoCentroide;
	vector<size_t> cont(17770,0);
	generarDatosCero(calculoCentroide);
	double error = 0.0;
	//este for recorre las mejores distancias que tenga asignado el centroide
	for(const auto recorreDistancia: distancias){
		if(get<0>(recorreDistancia.second) == idCentroide){
			vectorTupla datos_usuario = vectorVectores[recorreDistancia.first];
			for(size_t i = 0; i < datos_usuario.size(); i++){
				get<1>(calculoCentroide[(get<0>(datos_usuario[i]))-1]) += get<1>(datos_usuario[i]);
        cont[(get<0>(datos_usuario[i]))-1]++;
			}
		}
	}
	for(int i = 0; i < calculoCentroide.size(); i++){
      if(cont[i] != 0){
        //cout << get<1>(calculoCentroide[i]) << "/" << cont[i] << endl;
        get<1>(calculoCentroide[i]) = ceil(get<1>(calculoCentroide[i]) / cont[i]);
      }
    }

    error = calculoError(calculoCentroide, Centroide, normaCentorides, distancias, vectorVectores, idCentroide);
    Centroide = calculoCentroide;
    return error;

}

//usuario por centroide
/*tuple<double, double> distanciasMeans(vectorTupla &usuario, tabla &centroides, double &normaUsuario, vector<double> &normaCentorides){
	//for  que recorre los centroides para realizar  el producto punto
	size_t j = 0, productoPunto = 0;
	double distancia = 0.0;
	tuple<double, double> mejorDistancia = make_tuple(-1,-1);
	for(const auto tCentroides : centroides){
		//recorre las peliculas del usuario para realizar el producto punto
		for(int i = 0; i < usuario.size(); i++){
			j = get<0>(usuario[i]);
			productoPunto += get<1>(usuario[i]) * get<1>(tCentroides.second[j-1]);
		}
		distancia = acos(productoPunto/(normaUsuario * normaCentorides[tCentroides.first]));
		if(get<1>(mejorDistancia) == -1){
			mejorDistancia = make_tuple(tCentroides.first,distancia);
		}
		else{
			if(get<1>(mejorDistancia) > distancia){
				mejorDistancia = make_tuple(tCentroides.first,distancia);
			}
		}
	}
	return mejorDistancia;

}*/

//centroide por usuario
void distanciasMeans(tablaDistacias &distancias, vector<vectorTupla> &usuario, vectorTupla &centroide_iterado, tablaNorma &normaUsuario, double &normaCentorides, int &centroide_actual){
  size_t m = 0, productoPunto = 0;
  double distancia = 0.0;
  //tuple<double, double> mejorDistancia = make_tuple(-1,-1);
  for(int i = 0; i < usuario.size(); i++)
  {
    for(int j = 0; j < usuario[i].size(); j++)
    {
      m = get<0>(usuario[i][j]);
      productoPunto += get<1>(usuario[i][j]) * get<1>(centroide_iterado[m-1]);
    }
    distancia = acos(productoPunto/sqrt(normaUsuario[i] * normaCentorides));
    if(get<1>(distancias[i]) == -1){
      distancias[i] = make_tuple(centroide_actual,distancia);
    }
    else{
      if(get<1>(distancias[i]) > distancia){
        distancias[i] = make_tuple(centroide_actual,distancia);
      }
    }
    productoPunto = 0;
  }
}

void reseteaDistancia(tablaDistacias &distancias){
  for(int i = 0; i < distancias.size(); i++)
  {
    distancias[i] = make_tuple(-1,-1);
  }
}

void means(vector<vectorTupla> &vectorVectores, vector<vectorTupla> &centroides, tablaNorma &normas, tablaDistacias &distancias, size_t numCentroides){
  double error = 1;
  vector<double>  normaCentorides(numCentroides, 0);
  tuple<double,double> distancia;
  double errorAnterior = -1;
  double errorDiferencia = 0.0;
  int cont = 0;
  while(cont < 2){
    Timer t;
  	error = 0;
    reseteaDistancia(distancias);
  	//for que calcula la norma de centroides
  	/*for(int i = 0; i < centroides.size(); i++){
  		normaCentorides[i] = normaCentroide(centroides[i]);
  	}*/
  	//for que calcula las distancias iterando sobre cada usuario
  	/*for(int i = 0;  i < vectorVectores.size(); i++){
  		distancia = distanciasMeans(vectorVectores[i],centroides,normas[i],normaCentorides);
  		distancias[i] = distancia;
  	}*/

    //for que calcula las distancias iterando sobre cada centroide
    #pragma omp parallel for
    for(int i = 0; i < centroides.size(); i++)
    {
      normaCentorides[i] = normaCentroide(centroides[i]);
      distanciasMeans(distancias,vectorVectores,centroides[i],normas,normaCentorides[i],i);
    }
    #pragma omp parallel for
  	for(int i = 0; i < centroides.size(); i++){
  		//como es una suma no hay problemas con la condicion de carrera
  		error += calculaPromedio(vectorVectores,centroides[i],distancias,i,normaCentorides[i]);
  	}
    if(errorAnterior == -1){
      errorAnterior = error;
    }
    else{
      errorDiferencia = abs(error - errorAnterior);
      cout << "diferencia: " << errorDiferencia << endl;
      if(errorDiferencia < 0.087){
        cont++;
      }
      else{
        cont = 0;
      }
      errorAnterior = error;
    }
  	cout << "Error Total: "<<error<<endl;
    cout << "tiempo: " << t.elapsed() << endl;
  }
}

void generarCentroide(vector<vectorTupla> &centroides, size_t numCentroides){
  random_device rd;
  uniform_int_distribution<int> dist(0, 5);
  vectorTupla calificaciones;
  for(size_t j = 0; j < numCentroides; j++){
    for(size_t i = 1; i <= 17770; i++){
      calificaciones.push_back(make_tuple(i, dist(rd)));
    }
    centroides.push_back(calificaciones);
    calificaciones.clear();
  }
}

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
              //search->second.push_back(make_tuple(idPelicula, calificacion));
              vectorVectores[search->second].push_back(make_tuple(idPelicula, calificacion));

            }
            else
            {
              //cout << "entro:" << calificacion <<"\n";
              argumentos.push_back(make_tuple(idPelicula, calificacion));
              vectorVectores.push_back(argumentos);
              datos.insert({key, (vectorVectores.size() - 1)});
              distancias.insert({(vectorVectores.size() - 1), make_tuple(-1, -1)});
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
  unordered_map<size_t, size_t> datos;
  vector<vectorTupla> centroides;
  tablaDistacias distancias;
  size_t numCentroides = 0;
  vector<vectorTupla> vectorVectores;
  tablaNorma normas;
  lecturaArchivo(datos, distancias, vectorVectores);
  cout << "Datos cargados..." << endl;
  calculoNormas(vectorVectores, normas);
  cout << "Normas calculadas..." << endl;
  cout << "ingrese el número de centroides: ";
  cin >> numCentroides;
  generarCentroide(centroides, numCentroides);
  /*for(int i = 0; i < vectorVectores.size(); i++)
  {
  	for(int j = 0; j < vectorVectores[i].size(); j++)
  	{
  		cout<<"Vector["<<i<<"] = "<<get<1>(vectorVectores[i][j])<<endl;
  	}
  }*/
  //Timer t;
  means(vectorVectores, centroides, normas, distancias, numCentroides);
  //cout << "tiempo: " << t.elapsed() << endl;
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
