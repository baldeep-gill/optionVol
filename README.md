Application that fetches and aggregates historical and realtime (depending on subscription level with Polygon/Massive) SPX option data to visualise where the majority of positioning is with 0DTE contracts. The average strike for each contract weighted by volume is calculated for each 5-minute timestamp and plotted on a graph using Matplot++. Simple linear regression is used to visualise and extrapolate the trend of the day.
<p align="center"><img src="vwas_animation.gif" alt="animated" /></p>

Requirements:
libcurl, gnuplot, cmake, ffmpeg (for gif creation)

To run the program (in build directory):
```bash
cmake ..
```
```bash
cmake --build . --parallel
```
```bash
./optionVol
```

To generate a gif (in build directory):
```bash
ffmpeg -i frames/frame%03d.png -vf "palettegen" palette.png
```
```bash
ffmpeg -framerate 5 -i frames/frame%03d.png -i palette.png -filter_complex "paletteuse" -loop 0 animation.gif
```
