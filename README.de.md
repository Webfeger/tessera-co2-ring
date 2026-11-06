# CO₂ Ring

A Tessera plugin for a Home Assistant CO₂ sensor. The tile shows a large ppm value and a colour ring on a black background.

In den Tile-Optionen gibst du **Lower; Upper Threshold** als `untere;obere Schwelle` (Standard `800;1200`) und den **Ringbereich** als `Minimum;Maximum` (Standard `400;2000`) ein. Für jeden Text gibt es ein eigenes Feld `X;Y;Textgröße`: **PPM-Wert** (`0;15;180`), **CO2** (`0;0;100`) und **ppm** (`0;0;100`). X/Y verschieben relativ zur Kachelgröße; CO2 steht standardmäßig oben links außerhalb des Rings. Der Kachelhintergrund ist schwarz.

Tile sizes depend on the screen grid and Tessera's layout rules. Plugin tiles cannot currently occupy an entire page.
