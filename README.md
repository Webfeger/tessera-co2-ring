# CO₂ Ring

A Tessera tile for a Home Assistant CO₂ sensor. It shows a colour gauge and a large ppm value, sized to the tile. The gauge uses three bands: green below 800 ppm, yellow from 800 to below 1200 ppm, and red from 1200 ppm.

## Setup

1. Add this plugin to your screen in Tessera and install the build.
2. In Layout, place **CO₂ Ring** from the Plugins group.
3. Choose a Home Assistant CO₂ sensor in the tile inspector.

The gauge fills over 400–2000 ppm. Values below or above that range remain visible; the gauge stays at its lower or upper end. The value is rounded to whole ppm for readability.

## Display note

The tile adapts its gauge and number to the dimensions Tessera gives it. A 1×1 tile uses the compact arrangement; a 2×2 tile gives the scale and value more room. Tessera does not expose a user-controlled font-size setting to plugin tiles, so the plugin selects the largest built-in value font that fits.
