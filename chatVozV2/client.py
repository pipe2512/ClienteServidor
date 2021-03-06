import pyaudio
import wave
import sys
import zmq
import socket
import threading
import time

def grabarAudio(stream):
    frames = []
    for i in range(0, int(44100 / 1024 * 0.3)): #RATE/CHUNK*TIEMPODEGRABADO
        data = stream.read(1024)#CHUNK
        frames.append(data)
    #retorno = (b''.join(frames))
    return frames

def envioMensajes(cliente, s, parada, lista):
    #Instanciamos pyaudio
    play = pyaudio.PyAudio()

    #Creamos el stream para grabar el audio
    stream = play.open(format = pyaudio.paInt16,
                    channels = 2,
                    rate = 44100,
                    input=True,
                    frames_per_buffer=1024)#CHUNK

    while parada == True:
        retorno = grabarAudio(stream)
        audio = (b''.join(retorno))
        converso = audio.decode('utf-16', "ignore")
        s.send_json({"operacion" : "audio", "frames" : converso, "cliente" : list(lista)})
        s.recv_json()

    stream.stop_stream()
    stream.close()
    play.terminate()


def reproducir_lista(lock, lista_reproduccion, aux):
    #Instanciamos el pyaudio
    play = pyaudio.PyAudio()

    #Creamos el stream para reproducir el audio
    stream = play.open(format = pyaudio.paInt16,
                    channels = 2,
                    rate = 44100,
                    output=True)
    i = 0
    while True:
        if(len(lista_reproduccion) > i):
            stream.write(lista_reproduccion[i])
            i = i + 1
    stream.stop_stream()
    stream.close()
    play.terminate()


def recepcionMensaje(banderaOcupad, p, s, parada, lista):
    lista_reproduccion = []
    lock = threading.Lock()
    aux = 1
    hiloreproducir_lista = threading.Thread(target = reproducir_lista, args=(lock, lista_reproduccion, aux))
    hiloreproducir_lista.start()
    while True:
        msg = p.recv_json()
        if msg["operacion"] == "ocupado":
            if msg["estado"] == "0":
                p.send_json({"resultado" : "conectado"})
                parada = True
                lista = list(msg["lista"])
                lista.append(msg["alias"])
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(msg["alias"], s, parada, lista))
                hiloEnvioMensajes.start()
            elif msg["estado"] == "1" :
                p.send_json({"resultado" : "ocupado", "lista" : list(lista), "aliasCliente" : msg["alias"]})
                lista.append(msg["alias"])
        elif msg["operacion"] == "agregar":
            lista.append(msg["aliasCliente"])
            p.send_json({"okey" : "okey"})
        elif msg["operacion"] == "desconexion":
            if  banderaOcupado == "1":
                banderaOcupado = "0"
                parada = False
                p.send_json({"resultado" : "ocupado"})
        elif msg["operacion"] == "audio":
            p.send_json({"okey" : "okey"})
            frames = msg["frames"]
            converso = frames.encode('utf-16')
            lista_reproduccion.append(converso)
            aux = aux + 1;


def menu(banderaOcupad, alias, s, parada, lista_usuarios):
    while True:
        print("Seleccione una opcion: ")
        print ("1.Conectarme a otro cliente")
        print("2.Salir del chat")
        opcion = input()
        if opcion == "1":
            print("Ingrese el alias del cliente")
            s.send_json({"operacion" : "clientes"})
            msg = s.recv_json()
            print(msg["clientes"])
            opcion2 = input()
            s.send_json({"conexion" : opcion2, "operacion" : "conexion", "alias" : alias,"lista" : list(lista_usuarios)})
            msg = s.recv_json()#Aca colocamos la lista de alias de los usuarios de la persona con quien me conecte
            if msg["resultado"] == "conectado":
                lista_usuarios.append(opcion2)
                #banderaOcupado = "1"
                parada = True
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(opcion2, s, parada, lista_usuarios))
                hiloEnvioMensajes.start()
            elif msg["resultado"] == "ocupado":
                lista_usuarios = list(msg["lista"])
                lista_usuarios.append(opcion2)
                #banderaOcupado = "1"
                parada = True
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(opcion2, s, parada, lista_usuarios))
                hiloEnvioMensajes.start()
                ###################AQUI SE PONDRIA EL HILO DE ENVIOMENSAJES##################
        #elif opcion == "1" and  banderaOcupado == "1":
        #    print("Usted se encuentra conectado con otro cliente")
        elif opcion == "2" and  banderaOcupado == "1":
            banderaOcupado = "0"
            parada = False
            print("oye te has desconectado")
            s.send_json({"operacion" : "desconexion", "alias" : alias })
            s.recv_json()
        else:
            print("No se encuentra conectado con nadie")


def main():
    #entradas: ipServer, portServer, ipPropia, puertoPropio, alias
    if len(sys.argv) != 6:
        print("Error en los atributos.")
        exit()

    ip = sys.argv[1]#Ip del servidor
    port = sys.argv[2]#Puerto del servidor
    ipPropia = sys.argv[3]#Ip propia
    puertoPropio = sys.argv[4]#Puerto propio
    alias = sys.argv[5]#Alias del cliente
    banderaOcupad = "0"
    parada = False
    lista_usuarios = []

    #Conexion con el servidor.
    context = zmq.Context()
    s = context.socket(zmq.REQ)
    s.connect("tcp://{}:{}".format(ip, port))

    #Creacion del socket con el cual se conectara el servidor.
    context2 = zmq.Context()
    p = context2.socket(zmq.REP)
    p.bind("tcp://*:{}".format(puertoPropio))

    #Conexion primera vez al servidor
    s.send_json({"ip" : ipPropia, "puerto" : puertoPropio, "alias" : alias, "operacion" : "registro"})
    msg = s.recv_json()#Mensaje de okey
    hiloMenu = threading.Thread(target = menu, args=(banderaOcupad, alias, s, parada,lista_usuarios))
    hiloRecepcionMensaje = threading.Thread(target = recepcionMensaje, args=(banderaOcupad, p, s, parada,lista_usuarios))
    hiloMenu.start()
    hiloRecepcionMensaje.start()


if __name__ == '__main__':
    main()
