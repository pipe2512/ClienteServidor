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
        if msg["operacion"] == "registro":#Registra los clientes nuevos en el diccionario
            context2 = zmq.Context()
            p = context2.socket(zmq.REQ)
            p.connect("tcp://{}:{}".format(msg["ip"], msg["puerto"]))
            s.send_json({"okey" : "okey"})
            clientes[msg["alias"]] = [p, "0"]#Socket y bandera de estado(Ocupado 1/Libre 0)#Registra un cliente nuevo en el diccionario
        elif msg["operacion"] == "clientes":#Envia la lista de clientes conectados
            s.send_json({"clientes" : list(clientes.keys())})
        elif msg["operacion"] == "conexion":
            alias = msg["conexion"]
            aliasCliente = msg["alias"]
            clientes[alias][0].send_json({"operacion" : "ocupado", "estado" : clientes[alias][1], "alias" : msg["alias"], "lista" : list(msg["lista"])})
            msg = clientes[msg["conexion"]][0].recv_json()
            if msg["resultado"] == "conectado":
                clientes[alias][1] = "1"
            elif msg["resultado"] == "ocupado":
                for i in msg["lista"]:
                    clientes[i][0].send_json({"operacion" : "agregar", "aliasCliente" : msg["aliasCliente"]})
                    clientes[i][0].recv_json()
            clientes[aliasCliente][1] = "1"
            s.send_json(msg)
        elif msg["operacion"] == "desconexion":
            s.send_json({"okey" : "okey"})
            clientes[msg["alias"]][0].send_json({"operacion" : "desconexion"})
            clientes[msg["alias"]][0].recv_json()
        elif msg["operacion"] == "audio":
            s.send_json({"okey" : "okey"}) #tener encuenta el delay
            print(msg["cliente"])
            for i in msg["cliente"]:
                clientes[i][0].send_json({"operacion" : "audio" , "frames" : msg["frames"]})
                clientes[i][0].recv_json()






if __name__ == "__main__":
    main()
