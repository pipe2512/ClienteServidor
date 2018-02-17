#import pyaudio
import wave
import sys
import zmq

def main():
    #Entradas puertoServidor.
    if len(sys.argv) != 2:
        print("Error en los atributos")
        exit()

    clientes = []
    port = sys.argv[1]#Puerto del servidor
    context = zmq.Context()
    s = context.socket(zmq.REP)
    s.bind("tcp://*:{}".format(port))
    while True:
        msg = s.recv_json()
        if msg["operacion"] == "registro":
            context2 = zmq.Context()
            p = context2.socket(zmq.REQ)
            p.connect("tcp://{}:{}".format(msg["ip"], msg["puerto"]))
            clientes.append(p)
            print("Registrado el cliente {}".format(msg["alias"]))

if __name__ == "__main__":
    main()
