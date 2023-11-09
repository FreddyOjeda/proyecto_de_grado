import subprocess
import sys
import time
import subprocess

def get_devices():
    # Ejecuta el comando `netsh wlan show networks`
    process = subprocess.run(["netsh", "wlan", "show", "networks"], capture_output=True)

    # Obtén la salida del comando como una secuencia de bytes
    output = process.stdout

    # Decodifica la secuencia de bytes a una cadena Unicode usando la codificación ISO-8859-1
    output = output.decode("ISO-8859-1")

    # Divide la cadena Unicode en líneas
    lines = output.splitlines()

    # Filtra las líneas que contienen "SSID"
    devices = [line for line in lines if "SSID" in line]

    return devices



def get_rssi(device):
    # Separa la información del dispositivo en una lista
    info = device.split(" ")

    # Verifica si la lista contiene la información RSSI
    if len(info) >= 9:
        # Obtén la señal RSSI
        rssi = info[8]
        return rssi
    else:
        return None


while True:
    # Obtén la lista de dispositivos
    devices = get_devices()

    # Procesa la lista de dispositivos
    for device in devices:
        # Obtén la señal RSSI
        rssi = get_rssi(device)

        # Imprime la señal RSSI
        print(rssi)

        # Espera un segundo
        time.sleep(1)
