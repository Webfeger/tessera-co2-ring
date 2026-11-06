## 0.2.9

- Add entity domains and sample preview data so Tessera can draw the tile from a CO2 sensor.
- Include 2x3 and other tile sizes in the firmware preview bench.

## 0.2.8

- Keep the PPM value centred at every size by centring its LVGL transform pivot.

## 0.2.7

- Use the screen font's supported CO2 spelling in plugin labels.

## 0.2.6

- Keep the ring centred as text scales; place CO2 at the upper left of the ring and ppm below the value.

## 0.2.5

- Put the ppm reading in the gauge on larger tiles, with CO2 and ppm labels beside it.
- Keep the stacked layout on compact tiles.

## 0.2.4

- Raise the declared maximum tile size to 4x4 so Tessera can offer the full page on smaller grids.

## 0.2.3

- Quote percent signs in the size option units so the plugin manifest parses as YAML.

## 0.2.2

- Add adjustable ring, ppm number and label sizes; enlarge the default ppm number.
- Advertise sizes through 3x3 so Tessera can offer the largest shape supported by each screen.

## 0.2.0

- Add per-tile lower and upper ppm thresholds and selectable colours for all three ranges.
- Expand the entity picker to sensor, air_quality and number domains.
- Keep tile sizes from 1x1 through the display's maximum 2x3 grid.








