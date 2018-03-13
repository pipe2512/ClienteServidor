#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <tuple> //Preguntar que es mas optimo tupla o vector
#include <vector>
#include <cstdlib>

using namespace std;

typedef vector<tuple<size_t, size_t>> vectorTupla;
typedef unordered_map<double, vectorTupla> tabla;


void lecturaArchivo(tabla &datos){
  ifstream archivo("netflix/combined_data_1.txt");
  char linea[256];
  string line;
  //vectorTupla argumentos;
  size_t idPelicula = 0;
  size_t key = 0;
  size_t calificacion = 0;
  for(int i = 0; i < 6; i++){
    archivo.getline(linea, 256);
    line = linea;
    int posicionCaracter = line.find(":");
    if(posicionCaracter != string::npos){
      idPelicula = atoi(line.substr(0, posicionCaracter).c_str());
      cout << "id de la pelicula:  " << idPelicula << endl;
    }
    else{
        for(int j = 0; j < 2; j++){
          posicionCaracter = line.find(",");
          if(j == 0){
            key = atoi(line.substr(0, posicionCaracter).c_str());
            cout << "id del usuario:  " << key << endl;
            line.erase(0, posicionCaracter + 1);
            //cout << "linea: " << line << endl;
          }
          else{
            calificacion = atoi(line.substr(0, posicionCaracter).c_str());
            cout << "calificacion:  " << calificacion << endl;
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
  lecturaArchivo(datos);
}
