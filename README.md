# CO₂ Ring

A Tessera tile for a Home Assistant CO₂ value. It shows a colour gauge and a large ppm value, sized to the tile.

## Set up

1. Add this plugin to the screen in Tessera and install the build.
2. In Layout, place **CO₂ Ring** from the Plugins group.
3. Choose the entity for this tile. The picker supports `sensor`, `air_quality` and `number` entities.
4. Set the two ppm thresholds and choose a colour for each of the three ranges.

The tile supports sizes from 1×1 through 2×3. A 2×3 tile fills a display grid with two columns and three rows. A 3×3 tile cannot fit that grid. The gauge fills over 400–2000 ppm; values outside that display range stay visible while the gauge stops at its end.
