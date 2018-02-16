import pyaudio
import wave
import sys
import zmq
import socket


def main():
    #entradas: ipServer, portServer, ipPropia, puertoPropio, alias
    if len(sys.argv) != 4:
        print("Error en los atributos.")
        exit()

    ip = sys.argv[1]#Ip del servidor
    port = sys.argv[2]#Puerto del servidor
    ipPropia = sys.argv[3]#Ip propia
    puertoPropio = sys.argv[4]#Puerto propio
    alias = sys.argv[5]#Alias del cliente
    context = zmq.Context()
    s = connect("tcp://{}:{}".format(ip, port))
    #Conexion primera vez al servidor
    s.send_json({"ip" : ipPropia, "puerto" : puertoPropio, "alias" : alias})
    msg = s.recv_json()
    identificador = msg["identificador"]
    print(identificador)





if __name__ == '__main__':
    main()
