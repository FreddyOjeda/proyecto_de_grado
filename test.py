import scapy.all as scapy

def obtener_dispositivos_en_red(ip):
    arp_request = scapy.ARP(pdst=ip)
    broadcast = scapy.Ether(dst="ff:ff:ff:ff:ff:ff")
    arp_request_broadcast = broadcast/arp_request
    answered_list = scapy.srp(arp_request_broadcast, timeout=1, verbose=False)[0]

    dispositivos = []
    for elemento in answered_list:
        dispositivo = {"ip": elemento[1].psrc, "mac": elemento[1].hwsrc}
        dispositivos.append(dispositivo)
    return dispositivos

def mostrar_resultados(resultados):
    print("IP Address\t\tMAC Address")
    print("-----------------------------------------")
    for dispositivo in resultados:
        print(dispositivo["ip"] + "\t\t" + dispositivo["mac"])

ip_destino = "192.168.1.1/24"  # Reemplaza con la dirección de tu red
dispositivos_en_red = obtener_dispositivos_en_red(ip_destino)
mostrar_resultados(dispositivos_en_red)
