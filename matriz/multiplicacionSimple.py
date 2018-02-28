import time


def multiplicacion(matriz):
    matrizResultado = []
    i = 0
    while i < len(matriz):
        j = 0
        lista = []
        while j < len(matriz[i]):
            k = 0
            resultado = 0
            while k < len(matriz):
                resultado = resultado + (matriz[i][k] * matriz[k][j])
                k = k + 1
            lista.append(resultado)
            j = j + 1
        matrizResultado.append(lista)
        i = i + 1
    #print(matrizResultado)



def main():
    numeroColumnasA = 300
    numeroFilaA = 300
    matriz = [range(numeroColumnasA) for i in range(numeroFilaA)]
    start = time.time()
    multiplicacion(matriz)
    end = time.time()
    print(end-start)

if __name__ == '__main__':
    main()
