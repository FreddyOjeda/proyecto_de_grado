from pyLoRa import LoRa, ModemConfig

# Configurar el módem LoRa
lora = LoRa(0, 17, 8, 1, 1, 0, 17, 8, 1, 0, 1)
lora.set_mode_sleep()
lora.set_mode_rx()

# Esperar y imprimir mensajes recibidos
while True:
    if lora.packet_available():
        packet = lora.receive_packet()
        print(f"Mensaje recibido: {packet}")
