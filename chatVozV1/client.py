#import pyaudio
import wave
import sys
import zmq
import socket


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

    #Conexion con el servidor.
    context = zmq.Context()
    s = context.socket(zmq.REQ)
    s.connect("tcp://{}:{}".format(ip, port))

    #Conexion primera vez al servidor
    s.send_json({"ip" : ipPropia, "puerto" : puertoPropio, "alias" : alias, "operacion" : "registro"})

    #Creacion del socket con el cual se conectara el servidor.
    context2 = zmq.Context()
    p = context2.socket(zmq.REP)
    p.bind("tcp://*:{}".format(puertoPropio))


    while True:
        msg = p.recv_json()




if __name__ == '__main__':
    main()
