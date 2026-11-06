## 0.2.15

- Show the three colour ranges as a thin segmented frame along the gauge ring.
- Expand the PPM value's usable area so settings above 200% can render larger.

## 0.2.14

- Allow the PPM value size to scale up to 300%.
- Add coloured markers at the configured lower and upper colour thresholds on the ring.

## 0.2.13

- Give the PPM value, CO2 label and ppm unit separate X;Y;size settings; place CO2 above the ring.
- Group the colour thresholds and ring range into semicolon-separated pairs.

## 0.2.12

- Move the ppm value, CO2 label and ppm unit independently.
- Set separate text sizes for the CO2 label and ppm unit.

## 0.2.11

- Set the ring's start and full values, defaulting to 400–2000 ppm.
- Move the text group horizontally and vertically from the tile settings.
- Use a black tile background and black inactive ring track.

## 0.2.10

- Remove the duplicate generic Entity selector; keep the CO2 source selector.

## 0.2.9

- Keep one CO2 source selector in the tile settings instead of duplicating it with Tessera's generic Entity selector.
- Add sample sensor data and tile sizes for the firmware preview bench.

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








