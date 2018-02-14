import zmq
import sys
import time

def main():
    if len(sys.argv) != 4:
        print("Error!!!!!")
        exit()
    ip = sys.argv[1] #Server's ip
    port = sys.argv[2] #Server's import
    operation = sys.argv[3] #Operation to perfom

    context = zmq.Context()
    s = context.socket(zmq.REQ)
    s.connect("tcp://{}:{}".format(ip, port))
    print("Connecting to server {} at {}".format(ip,port))

    if operation == "list":
        s.send_json({"op":"list"})
        files = s.recv_json()
        print(files)
    elif operation == "download":
        name = input("File to download? ")
        part = int(input("Parte a descargar "))
        s.send_json({"op": "download", "file": name, "part": part})
        file = s.recv()
        with open("descarga.mp3", "wb") as output:
            output.write(file)
    elif operation == "parts":
        name = input("Nombre del archivo ")
        s.send_json({"op": "parts", "file": name})
        file = s.recv_json()
        print ("El numero de partes es " + file["parts"])
    elif operation == "all":
        name = input("File to download? ")
        s.send_json({"op": "parts", "file": name})
        parts = s.recv_json()
        i = 0;
        parts = int(parts["parts"])
        print ("Numero de partes de la cancion: {}".format(parts))
        start = time.time()
        while i < parts:
            s.send_json({"op": "download", "file": name, "part": i})
            file = s.recv()
            with open("{}".format(name), "ab+") as output:
                print ("Recibiendo parte {}".format(i))
                #output.seek(i*1048576, 1)
                output.write(file)
            i = i+1
        end = time.time()
        print (end - start)
    else:
        print("Error!!! unsupported operation")

if __name__ == '__main__':
    main()
