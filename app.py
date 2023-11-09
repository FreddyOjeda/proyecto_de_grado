import time
#from scapy.all import *
from scapy.layers.dot11 import Dot11
from scapy.sendrecv import sniff
from pywifi import PyWiFi

# Función para obtener la señal RSSI de una dirección MAC
def get_rssi(mac_address, packets):
    rssi_list = [packet.dBm_AntSignal for packet in packets if packet.haslayer(Dot11) and packet.addr2 == mac_address]
    if rssi_list:
        return max(rssi_list)
    return None

# Función para escanear y mostrar la señal RSSI de los dispositivos en la red WiFi
def scan_wifi_network(interface):
    wifi = PyWiFi()
    iface = wifi.interfaces()[interface]

    print(f"Escaneando dispositivos en la red WiFi de {iface.name()}...")
    packets = sniff(iface=iface, count=100, timeout=10)
    
    devices = set(packet.addr2 for packet in packets if packet.haslayer(Dot11))
    
    for device in devices:
        rssi = get_rssi(device, packets)
        if rssi is not None:
            print(f"Dispositivo: {device}, RSSI: {rssi} dBm")

def getInterface(interface_name):
    wifi = PyWiFi()
    iface = None
    
    # Encuentra la interfaz por su nombre
    for interface in wifi.interfaces():
        if interface.name() == interface_name:
            iface = interface
            break

    if iface is not None:
        # Procede con la captura de señales RSSI en 'iface'
        print(f"Interfaz encontrada: {iface.name()}")
        # Continúa con tu lógica para la captura RSSI
    else:
        print(f"No se encontró una interfaz con el nombre '{interface_name}'.")

if __name__ == "__main__":
    interface_name = "LAN inalámbrica Wi-Fi"
    getInterface(interface_name)
