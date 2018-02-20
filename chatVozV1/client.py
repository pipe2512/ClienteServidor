import pyaudio
import wave
import sys
import zmq
import socket
import threading

def pruebaReproducir(frames):
    CHUNK = 1024

    #wf = wave.open("paso.wav", 'rb')

    p = pyaudio.PyAudio()

    stream = p.open(format = pyaudio.paInt16,
                    channels = 2,
                    rate = 44100,
                    output=True)

    #data = wf.readframes(CHUNK)

    #for i in range(220160):
    #    print("estoy entrando")
    stream.write(frames)
        #data = wf.readframes(CHUNK)

    stream.stop_stream()
    stream.close()

    p.terminate()

def grabarAudio():
    #Parametros para el manejo del audio
    CHUNK = 1024
    RECORD_SECONDS = 5

    #Instanciamos pyaudio
    p = pyaudio.PyAudio()

    #Creamos el stream para grabar el audio
    stream = p.open(format = pyaudio.paInt16,
                    channels = 2,
                    rate = 44100,
                    input=True,
                    frames_per_buffer=CHUNK)

    frames = []
    numeroFrames = int(RATE / CHUNK * RECORD_SECONDS)

    print("Grabando")
    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        data = stream.read(CHUNK)
        frames.append(data)
    print("******")

    stream.stop_stream()
    stream.close()
    p.terminate()
    retorno = (b''.join(frames))
    return retorno

def envioMensajes(cliente, s):
    print("inicio el hilo de envios de mensajes con: " + cliente)

def recepcionMensaje(banderaOcupado, p, s):
    while True:
        msg = p.recv_json()
        if msg["operacion"] == "ocupado":
            if banderaOcupado == "0" and msg["estado"] == "0":
                p.send_json({"resultado" : "conectado"})
                banderaOcupado = "1"
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(msg["alias"], s))
                hiloEnvioMensajes.start()
                ###################AQUI SE PONDRIA EL HILO DE ENVIOMENSAJES##################
            elif banderaOcupado == "1" and msg["estado"] == "0":
                p.send_json({"resultado" : "ocupado"})
            elif banderaOcupado == "1" and msg["estado"] == "1":
                p.send_json({"resultado" : "desconectado"})
                banderaOcupado = "0"

def menu(banderaOcupado, alias, s):
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
                hiloEnvioMensajes = threading.Thread(target = envioMensajes, args=(opcion2, s))
                hiloEnvioMensajes.start()
                ###################AQUI SE PONDRIA EL HILO DE ENVIOMENSAJES##################
        elif opcion == "1" and banderaOcupado == "1":
            print("Usted se encuentra conectado con otro cliente")
        elif opcion == "2" and banderaOcupado == "1":
            print("pendiente.......")
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
    hiloMenu = threading.Thread(target = menu, args=(banderaOcupado, alias, s))####Preguntar acerca de el cambio de variables en varios procesos
    hiloRecepcionMensaje = threading.Thread(target = recepcionMensaje, args=(banderaOcupado, p, s))
    hiloMenu.start()
    hiloRecepcionMensaje.start()


if __name__ == '__main__':
    main()
