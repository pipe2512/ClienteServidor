#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void lecturaArchivo(){
  ifstream archivo("netflix/combined_data_1.txt");
  char linea[256];
  string line;
  for(int i = 0; i < 6; i++){
    archivo.getline(linea, 256);
    line = linea;
    int posicionCaracter = line.find(":");
    if(posicionCaracter != string::npos){
      cout << "id de la pelicula:  " << line.substr(0, posicionCaracter) << endl;
    }
    else{
        for(int j = 0; j < 2; j++){
          posicionCaracter = line.find(",");
          if(j == 0){
            cout << "id del usuario:  " << line.substr(0, posicionCaracter) << endl;
            line.erase(0, posicionCaracter + 1);
            //cout << "linea: " << line << endl;
          }
          else{
            cout << "calificacion:  " << line.substr(0, posicionCaracter) << endl;
          }
        }
    }

  }
  archivo.close();
}

int main(){
  lecturaArchivo();
}
