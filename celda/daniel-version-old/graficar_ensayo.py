#!/usr/bin/env python3
"""
graficar_ensayo.py — Grafica los CSV del banco de empuje.

Por defecto dibuja exactamente lo que dice el archivo, sin corregir nada.

USO
---
  Instalar dependencias (una sola vez):
      python -m pip install pandas matplotlib

  Graficar un archivo:
      python graficar_ensayo.py ensayos/ensayo_20260812_143052.csv

  Sin argumentos toma el CSV mas reciente de la carpeta 'ensayos/':
      python graficar_ensayo.py

OPCIONES
--------
  --kg              grafica en kg-f en vez de Newtons
  --conteos         grafica los conteos crudos del ADC
  --base-ms 2000    resta la linea base (promedio de los primeros N ms).
                    Por defecto NO se resta nada.
  --suavizado 51    media movil de N muestras (por defecto 0 = crudo)
  --analisis        agrega pico, tiempo de quemado e impulso total
  --no-guardar      solo muestra la ventana, no escribe el PNG
"""

import argparse
import glob
import os
import sys

try:
    import pandas as pd
    import matplotlib.pyplot as plt
except ImportError as e:
    sys.exit(f"Falta una dependencia ({e.name}). Instala con:\n"
             f"    python -m pip install pandas matplotlib")


def elegir_archivo():
    candidatos = sorted(glob.glob("ensayos/*.csv"), key=os.path.getmtime)
    if not candidatos:
        sys.exit("No se encontro ningun CSV en 'ensayos/'.\n"
                 "Pasa la ruta a mano:  python graficar_ensayo.py archivo.csv")
    print(f"Usando el mas reciente: {candidatos[-1]}\n")
    return candidatos[-1]


def main():
    ap = argparse.ArgumentParser(description="Grafica ensayos del banco de empuje")
    ap.add_argument("archivo", nargs="?", help="Ruta del CSV")
    ap.add_argument("--kg", action="store_true", help="Graficar en kg-f")
    ap.add_argument("--conteos", action="store_true", help="Graficar conteos crudos")
    ap.add_argument("--base-ms", type=float, default=0,
                    help="Restar linea base de los primeros N ms (0 = no restar)")
    ap.add_argument("--suavizado", type=int, default=0,
                    help="Media movil de N muestras (0 = sin suavizar)")
    ap.add_argument("--analisis", action="store_true",
                    help="Calcular pico, tiempo de quemado e impulso")
    ap.add_argument("--no-guardar", action="store_true")
    args = ap.parse_args()

    ruta = args.archivo or elegir_archivo()
    if not os.path.exists(ruta):
        sys.exit(f"No existe el archivo: {ruta}")

    df = pd.read_csv(ruta, comment="#")
    df.columns = [c.strip() for c in df.columns]
    if "t_ms" not in df.columns:
        sys.exit(f"El CSV no tiene columna 't_ms'. Columnas: {list(df.columns)}")

    # --- Elegir columna ---
    if args.conteos:
        col, unidad, etiqueta = ("neto" if "neto" in df else "conteo"), "conteos", "Senal (conteos)"
    elif args.kg:
        col, unidad, etiqueta = "kg", "kg-f", "Empuje (kg-f)"
    else:
        col, unidad, etiqueta = "N", "N", "Empuje (N)"

    if col not in df.columns:
        sys.exit(f"El CSV no tiene la columna '{col}'. Columnas: {list(df.columns)}")

    t = df["t_ms"].astype(float)
    y = df[col].astype(float)

    if y.abs().max() == 0:
        print(f"AVISO: la columna '{col}' esta toda en cero (factor_kg = 0 al medir).")
        print("       Usa --conteos para ver la senal cruda.\n")

    # Lo que hay en el archivo, antes de tocar nada
    print(f"Columna '{col}' tal como esta en el CSV:")
    print(f"   minimo: {y.min():.4f} {unidad}")
    print(f"   maximo: {y.max():.4f} {unidad}")
    print(f"   muestras: {len(y)}\n")

    # --- Correccion de linea base (opcional, apagada por defecto) ---
    if args.base_ms > 0:
        base = float(y[t <= args.base_ms].median())
        y = y - base
        print(f"Linea base restada: {base:.4f} {unidad} "
              f"(primeros {args.base_ms:.0f} ms)\n")

    if args.suavizado > 1:
        # center=True + min_periods=1 evita que los bordes se hundan
        y_plot = y.rolling(args.suavizado, center=True, min_periods=1).mean()
    else:
        y_plot = y

    # --- Grafico ---
    fig, ax = plt.subplots(figsize=(11, 5.5))
    ax.plot(t, y_plot, lw=0.9, color="#0b3d5c")
    ax.axhline(0, color="#999", lw=0.7)

    ax.set_xlabel("Tiempo (ms)")
    ax.set_ylabel(etiqueta)
    ax.set_title(f"Ensayo de empuje — {os.path.basename(ruta)}", fontsize=12, pad=10)
    ax.grid(alpha=0.25, lw=0.5)
    ax.margins(x=0.01)

    # --- Analisis opcional ---
    if args.analisis:
        pico = float(y_plot.max())
        base_ruido = float(y_plot[t <= 2000].std()) if (t <= 2000).sum() > 5 else 0.0
        # El umbral nunca debe quedar por debajo del ruido de fondo
        umbral = max(pico * 0.05, base_ruido * 5)
        sobre = (y_plot > umbral).to_numpy()

        # Busca el tramo continuo mas largo sobre el umbral
        mejor = (0, -1)
        i = 0
        while i < len(sobre):
            if sobre[i]:
                j = i
                while j + 1 < len(sobre) and sobre[j + 1]:
                    j += 1
                if j - i > mejor[1] - mejor[0]:
                    mejor = (i, j)
                i = j + 1
            else:
                i += 1

        i0, i1 = mejor
        if i1 > i0 and pico > umbral:
            t0, t1 = float(t.iloc[i0]), float(t.iloc[i1])
            seg_t, seg_y = t.iloc[i0:i1 + 1] / 1000.0, y_plot.iloc[i0:i1 + 1]
            impulso = float((seg_y * seg_t.diff().fillna(0)).sum())
            lineas = [
                f"Pico    : {pico:.2f} {unidad}  (a los {t.iloc[y_plot.idxmax()]:.0f} ms)",
                f"Medio   : {seg_y.mean():.2f} {unidad}",
                f"Duracion: {(t1 - t0) / 1000:.3f} s",
            ]
            if col == "N":
                lineas.append(f"Impulso : {impulso:.1f} N·s")
            ax.axvspan(t0, t1, color="#b45309", alpha=0.07, zorder=0)
            ax.text(0.012, 0.97, "\n".join(lineas), transform=ax.transAxes,
                    va="top", ha="left", fontsize=8.5, family="monospace",
                    bbox=dict(boxstyle="round,pad=0.45", fc="#f7f7f7", ec="#ccc", lw=0.6))
            print("\n".join(" " + l for l in lineas))
        else:
            print("Sin evento de empuje detectable sobre el ruido de fondo.")
            print("(La senal no supera 5 sigma de la linea base.)")

    fig.tight_layout()

    if not args.no_guardar:
        salida = os.path.splitext(ruta)[0] + ".png"
        fig.savefig(salida, dpi=150)
        print(f"\nGrafico guardado: {salida}")

    plt.show()


if __name__ == "__main__":
    main()