from scapy.all import ARP, Ether, srp
import requests

def scan_network(ip):
    arp = ARP(pdst=ip)
    ether = Ether(dst="ff:ff:ff:ff:ff:ff")
    packet = ether/arp
    result = srp(packet, timeout=3, verbose=0)[0]
    devices = []
    for sent, received in result:
        vendor = get_vendor(received.hwsrc)
        devices.append({'ip': received.psrc, 'mac': received.hwsrc, 'vendor': vendor})
    return devices

def get_vendor(mac_address):
    url = f"https://api.macvendors.com/{mac_address}"
    response = requests.get(url)
    if response.status_code != 200:
        return "N/A"
    return response.content.decode()

devices = scan_network('192.168.1.0/24')
for device in devices:
    print(f"IP: {device['ip']} MAC: {device['mac']} Vendor: {device['vendor']}")
