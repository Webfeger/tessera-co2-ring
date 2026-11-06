# CO₂ Ring

A Tessera tile for a Home Assistant CO₂ value. It shows a colour gauge and a large ppm value, sized to the tile.

## Set up

1. Add this plugin to the screen in Tessera and install the build.
2. In Layout, place **CO₂ Ring** from the Plugins group.
3. Choose the entity for this tile. The picker supports `sensor`, `air_quality` and `number` entities.
4. Set the two colour thresholds, the ring's minimum and maximum ppm values, and a colour for each range. Adjust the ring, value, CO2 and ppm text sizes in the tile options. **Text sizes (CO2, ppm)** takes two percentages, such as `100,120`. **Text positions (value; CO2; ppm)** takes three X,Y pairs, such as `0,15;0,0;0,15`. Positive X moves right and positive Y moves down. The tile background is black.

The gauge defaults to 400–2000 ppm; values outside the selected range stay visible while the gauge stops at its end. Tile sizes are subject to the screen grid and Tessera's layout rules; plugin tiles cannot currently occupy an entire page.


