import zmq
import sys
import os
import math

def loadFiles(path):
    files = {}
    dataDir = os.fsencode(path)
    for file in os.listdir(dataDir):
        filename = os.fsdecode(file)
        print("Loading {}".format(filename))
        files[filename] = files
    return files

def main():
    if len(sys.argv) != 3:
        print("Error!!!!!!!")
        exit()

    directory = sys.argv[2]
    port = sys.argv[1]

    context = zmq.Context()
    s = context.socket(zmq.REP)
    s.bind("tcp://*:{}".format(port))

    files = loadFiles(sys.argv[2])
    #print(files)

    while True:
        msg = s.recv_json()
        if msg["op"] == "list":
            s.send_json({"files": list(files.keys())})
        elif msg["op"] == "download":
            filename = msg["file"]
            part = msg["part"]
            if filename in files:
                with open(directory + filename, "rb") as input:
                    input.seek(part * 1048576)
                    data = input.read(1048576)
                    s.send(data)
            else:
                print("JAJAJA!!")
                s.send_string("No existe")
        elif msg["op"] == "parts":
            filename = msg["file"]
            if filename in files:
                with open(directory + filename, "rb") as input:
                    tamano = int(os.path.getsize(directory + filename))
                    s.send_json({"parts": str(math.ceil(tamano/1048576))})
            else:
                print("JAJAJA!!")
                s.send_string("No existe")
        else:
            print("unsupported action!")

if __name__ == "__main__":
    main()
