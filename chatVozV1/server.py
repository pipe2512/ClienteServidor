import pyaudio
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

    #Creacion del socket para escuchar los clientes
    context = zmq.Context()
    s = context.socket(zmq.REP)
    s.bind("tcp://*:{}".format(port))

    #Diccionario de clientes conectados
    clientes = {}

    while True:
        msg = s.recv_json()
        if msg["operacion"] == "registro":
            context2 = zmq.Context()
            p = context2.socket(zmq.REQ)
            p.connect("tcp://{}:{}".format(msg["ip"], msg["puerto"]))
            s.send_json({"okey" : "okey"})
            clientes = {msg["alias"] : [p, "0"]}#Socket y bandera de estado(Ocupado 1/Libre 0)
        if msg["operacion"] == "clientes":
            s.send_json({"clientes" : list(cliente.keys())})
        if msg["operacion"] == "conexion":
            clientes[msg["conexion"]].send_json({"operacion" : "ocupado", "estado" : "0"})
            msg = clientes[msg["conexion"]].recv_json()
            s.send_json(msg)####Tener en cuenta###### Puede ser un error




if __name__ == "__main__":
    main()
