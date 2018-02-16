import pyaudio
import wave
import sys
import zmq

def main():
    #Entradas puertoServidor.
    if len(sys.argv) != 2:
        print("Error en los atributos")
        exit()

    direcciones = []
    port = sys.argv[1]#Puerto del servidor
    context = zmq.Context()
    s = context.socket(zmq.REP)
    s.bind("tcp://*:{}".format(port))
    while True:
        msg = s.recv_json()
        if msg["ip"] != None:
            parametro = [msg["ip"], msg["puerto"], msg["alias"]]
            direcciones.append(parametro)
            identificador = len(direcciones) - 1
            s.send_json({"identificador" : identificador})


if __name__ == "__main__":
    main()
