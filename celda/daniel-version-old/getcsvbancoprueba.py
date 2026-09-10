#!/usr/bin/env python3
"""
registrar_ensayo.py — Guarda automaticamente los ensayos del banco de empuje.

Escucha el puerto serie del ESP32 (MODO 4), dispara la captura y guarda el
CSV con nombre y fecha, sin copiar y pegar nada.

USO
---
  1) Instalar la dependencia (una sola vez):
         pip install pyserial

  2) Cerrar el Monitor Serie del Arduino IDE (no pueden usar el puerto los dos).

  3) Ejecutar:
         python registrar_ensayo.py

     Para elegir el puerto a mano:
         python registrar_ensayo.py --puerto COM5
         python registrar_ensayo.py --puerto /dev/cu.usbserial-0001

  4) Seguir las instrucciones en pantalla. El archivo queda en la carpeta
     'ensayos/' con nombre tipo  ensayo_20260812_143052.csv
"""

import argparse
import os
import sys
from datetime import datetime

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("Falta pyserial. Instalalo con:  pip install pyserial")


BAUDIOS = 115200
CARPETA = "ensayos"

# Marcas que imprime el ESP32 y que el script reconoce
MARCA_ESPERA = "Envia cualquier tecla"
MARCA_FIN_CAPTURA = "--- FIN DE CAPTURA ---"
MARCA_CABECERA = "t_ms,conteo"
MARCA_FIN_DATOS = "--- FIN DE DATOS ---"


def elegir_puerto():
    """Detecta el puerto del ESP32, o pide elegir si hay varios."""
    puertos = list(list_ports.comports())
    if not puertos:
        sys.exit("No se detecto ningun puerto serie. Revisa el cable USB.")

    # Filtra los que suelen ser adaptadores USB-serie de placas ESP32
    probables = [
        p for p in puertos
        if any(x in (p.description or "").lower() or x in (p.device or "").lower()
               for x in ("usb", "cp210", "ch340", "wch", "silicon", "slab"))
    ]
    candidatos = probables or puertos

    if len(candidatos) == 1:
        print(f"Puerto detectado: {candidatos[0].device}  ({candidatos[0].description})")
        return candidatos[0].device

    print("\nPuertos disponibles:")
    for i, p in enumerate(candidatos):
        print(f"  [{i}] {p.device}  —  {p.description}")
    while True:
        try:
            n = int(input("\nElige el numero del puerto: ").strip())
            if 0 <= n < len(candidatos):
                return candidatos[n].device
        except ValueError:
            pass
        print("Numero invalido.")


def main():
    ap = argparse.ArgumentParser(description="Registrador de ensayos del banco de empuje")
    ap.add_argument("--puerto", help="Puerto serie (ej. COM5 o /dev/cu.usbserial-0001)")
    ap.add_argument("--baudios", type=int, default=BAUDIOS, help=f"Por defecto {BAUDIOS}")
    ap.add_argument("--nombre", help="Nombre base del archivo (opcional)")
    args = ap.parse_args()

    puerto = args.puerto or elegir_puerto()

    print(f"\nAbriendo {puerto} a {args.baudios} baudios...")
    print("(Recuerda cerrar el Monitor Serie del Arduino IDE)\n")

    try:
        ser = serial.Serial(puerto, args.baudios, timeout=1)
    except serial.SerialException as e:
        sys.exit(f"No se pudo abrir el puerto: {e}")

    # El ESP32 se reinicia al abrir el puerto: se captura desde el arranque.
    ser.setDTR(False)
    ser.setRTS(False)

    print("=" * 62)
    print(" Esperando al ESP32. Si no aparece nada, pulsa RESET en la placa.")
    print("=" * 62 + "\n")

    metadatos = []
    filas = []
    estado = "arranque"      # arranque -> metadatos -> datos -> listo

    try:
        while estado != "listo":
            linea = ser.readline().decode("utf-8", errors="replace").rstrip("\r\n")
            if not linea:
                continue

            if estado == "datos":
                # Dentro del bloque CSV
                if MARCA_FIN_DATOS in linea:
                    estado = "listo"
                    continue
                filas.append(linea)
                if len(filas) % 1000 == 0:
                    print(f"  ... {len(filas)} muestras recibidas")
                continue

            # Fuera del bloque CSV: se muestra todo lo que dice el ESP32
            print(linea)

            if MARCA_FIN_CAPTURA in linea:
                estado = "metadatos"
                continue

            if estado == "metadatos" and MARCA_CABECERA not in linea:
                if linea.strip() and not linea.startswith("Copia desde"):
                    metadatos.append(linea.strip())

            if MARCA_CABECERA in linea:
                filas.append(linea)          # cabecera del CSV
                estado = "datos"
                print("\nRecibiendo datos...")
                continue

            if MARCA_ESPERA in linea:
                input("\n>>> Pulsa ENTER para DISPARAR la captura "
                      "(enciende el motor 2-3 s despues) <<<\n")
                ser.write(b"g")
                ser.flush()

    except KeyboardInterrupt:
        print("\n\nInterrumpido por el usuario.")
    finally:
        ser.close()

    if len(filas) <= 1:
        print("\nNo se recibieron datos. No se guardo ningun archivo.")
        return

    # --- Guardado ---
    os.makedirs(CARPETA, exist_ok=True)
    marca = datetime.now().strftime("%Y%m%d_%H%M%S")
    base = args.nombre or "ensayo"
    ruta = os.path.join(CARPETA, f"{base}_{marca}.csv")

    with open(ruta, "w", encoding="utf-8", newline="") as f:
        f.write(f"# Ensayo del banco de empuje\n")
        f.write(f"# Fecha: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"# Puerto: {puerto}\n")
        for m in metadatos:
            f.write(f"# {m}\n")
        f.write("\n".join(filas) + "\n")

    n = len(filas) - 1
    print("\n" + "=" * 62)
    print(f" Guardado: {ruta}")
    print(f" Muestras: {n}")
    print("=" * 62)

    for m in metadatos:
        if "Tasa efectiva" in m:
            print(f"\n {m}   <- si esta muy por debajo de 1000, se perdieron muestras")


if __name__ == "__main__":
    main()