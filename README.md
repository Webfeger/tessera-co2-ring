# CO₂ Ring

A Tessera tile for a Home Assistant CO₂ value. It shows a colour gauge and a large ppm value, sized to the tile.

## Set up

1. Add this plugin to the screen in Tessera and install the build.
2. In Layout, place **CO₂ Ring** from the Plugins group.
3. Choose the entity for this tile. The picker supports `sensor`, `air_quality` and `number` entities.
4. Set **Thresholds** as `lower;upper` (default `800;1200`) and **Ring range** as `minimum;maximum` (default `400;2000`). Each text has its own `X;Y;size` setting: **PPM value** (`0;15;180`), **CO2** (`0;0;100`) and **ppm** (`0;0;100`). X/Y shift as a percentage of tile size; CO2 starts at the upper left, outside the ring. The tile background is black.

The gauge defaults to 400–2000 ppm; values outside the selected range stay visible while the gauge stops at its end. Tile sizes are subject to the screen grid and Tessera's layout rules; plugin tiles cannot currently occupy an entire page.


