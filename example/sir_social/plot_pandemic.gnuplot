#!/usr/bin/gnuplot
reset

# png
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'

# color definitions
set border lw 2
set style line 1 lc rgb 'gray' lt 1 lw 1
set style line 2 lc rgb 'black' lt 1 lw 1

unset key
unset tics

set xrange [0:950]
set yrange [620:0]

# Country indices for world.dat
BD = 14
BR = 22
CO = 65
DE = 73
EG = 86
ET = 93
FR = 97
GB = 105
ID = 125
IN = 139
IT = 143
JP = 148
KE = 154
KR = 159
MM = 178
MX = 187
NG = 194
PH = 210
PK = 217
TH = 283
TR = 287
TZ = 290
US = 293
VN = 305
ZA = 311

set colorbox user
set cbrange[0:5000]
set palette defined(0 "white", 5000 "red")

color_max = 5001
color_norm(val) = int(real(val) / color_max * 256)
color(val) = floor(65536 * color_norm(val) + 256 * 0 + 0)
function $plot_country(i, val) << EOF
  print color(val)
  plot 'data/world.dat' index i with filledcurves ls 2
  plot 'data/world.dat' index i using 1:2:(val) with filledcurves \
    lc palette
    #lc rgb variable
EOF

set output 'img/pandemic.png'
set multiplot
do for [i = 0:334] {
  plot 'data/world.dat' index i with filledcurves ls 2
  plot 'data/world.dat' index i with filledcurves ls 1
}

void = $plot_country(britain, 2500)
void = $plot_country(canada, 2500)
void = $plot_country(china, 500)
void = $plot_country(france, 2500)
void = $plot_country(germany, 2500)
void = $plot_country(india, 2500)
void = $plot_country(mexico, 2500)
void = $plot_country(south_korea, 2500)
void = $plot_country(usa, 5000)
unset multiplot

