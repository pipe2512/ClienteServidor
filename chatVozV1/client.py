import pyaudio
import wave
import sys
import zmq
import socket
import threading

def grabarAudio(stream):
    frames = []
    for i in range(0, int(44100 / 1024 * 0.3)): #RATE/CHUNK*TIEMPODEGRABADO
        data = stream.read(1024)#CHUNK
        frames.append(data)
    #retorno = (b''.join(frames))
    return frames

def envioMensajes(cliente, s, parada):
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
        s.send_json({"operacion" : "audio", "frames" : converso, "cliente" : cliente})
        s.recv_json()

    stream.stop_stream()
    stream.close()
    play.terminate()

def recepcionMensaje(banderaOcupado, p, s, parada):
    #Instanciamos el pyaudio
    play = pyaudio.PyAudio()

    #Creamos el stream para reproducir el audio
    stream = play.open(format = pyaudio.paInt16,
                    channels = 2,
                    rate = 44100,
                    output=True)
    while True:
        msg = p.recv_json()
        if msg["operacion"] == "ocupado":
            if banderaOcupado == "0" and msg["estado"] == "0":
                p.send_json({"resultado" : "conectado"})
                banderaOcupado = "1"
                parada = True
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(msg["alias"], s, parada))
                hiloEnvioMensajes.start()
                ###################AQUI SE PONDRIA EL HILO DE ENVIOMENSAJES##################
            elif banderaOcupado == "1" and msg["estado"] == "0":
                p.send_json({"resultado" : "ocupado"})
            elif banderaOcupado == "1" and msg["estado"] == "1":
                p.send_json({"resultado" : "desconectado"})
                banderaOcupado = "0"
                parada = False
        elif msg["operacion"] == "desconexion":
            if banderaOcupado == "1":
                banderaOcupado = "0"
                parada = False
                p.send_json({"resultado" : "ocupado"})
        if msg["operacion"] == "audio":
            p.send_json({"okey" : "okey"})
            print("voy a reproducir")
            frames = msg["frames"]
            converso = frames.encode('utf-16')
            stream.write(converso)

    stream.stop_stream()
    stream.close()
    play.terminate()


def menu(banderaOcupado, alias, s, parada):
    while True:
        print("Seleccione una opcion: ")
        print ("1.Conectarme a otro cliente")
        print("2.Salir del chat")
        opcion = input()
        if opcion == "1" and banderaOcupado == "0":
            print("Ingrese el alias del cliente")
            s.send_json({"operacion" : "clientes"})
            msg = s.recv_json()
            print(msg["clientes"])
            opcion2 = input()
            s.send_json({"conexion" : opcion2, "operacion" : "conexion", "alias" : alias})
            msg = s.recv_json()#Mensaje de okey
            if msg["resultado"] == "conectado":
                banderaOcupado = "1"
                parada = True
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(opcion2, s, parada))
                hiloEnvioMensajes.start()
                ###################AQUI SE PONDRIA EL HILO DE ENVIOMENSAJES##################
        elif opcion == "1" and banderaOcupado == "1":
            print("Usted se encuentra conectado con otro cliente")
        elif opcion == "2" and banderaOcupado == "1":
            banderaOcupado = "0"
            parada = False
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
    banderaOcupado = "0"
    parada = False

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
    hiloMenu = threading.Thread(target = menu, args=(banderaOcupado, alias, s, parada))####Preguntar acerca de el cambio de variables en varios procesos
    hiloRecepcionMensaje = threading.Thread(target = recepcionMensaje, args=(banderaOcupado, p, s, parada))
    hiloMenu.start()
    hiloRecepcionMensaje.start()


if __name__ == '__main__':
    main()
