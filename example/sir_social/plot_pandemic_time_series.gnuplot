#!/usr/bin/gnuplot
reset

# png
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'img/pandemic_time_series.png'

# color definitions
set border lw 2

set xlabel "Time"
set ylabel "Infected Pop. (Indexed) (Log scale)"
set xrange [1:364]
set yrange [0:log10(100000)]

# Country indices for pandemic.dat
$MAP_IDS << EOD
2 # BD
3 # BR
4 # CO
5 # DE
6 # EG
7 # ET
8 # FR
9 # GB
10 # ID
11 # IN
12 # IT
13 # JP
14 # KE
15 # KR
16 # MM
17 # MX
18 # NG
19 # PH
20 # PK
21 # TH
22 # TR
23 # TZ
24 # US
25 # VN
26 # ZA
EOD

set multiplot

plot for [i = 1:25] \
  "data/pandemic.dat" index 1 \
  using 1:(log10(column(i + 1))) notitle with lines \
  linecolor 'gray' lw 1 \

color_A = "#FF1F5B";
color_B = "#00CD6C";
color_C = "#009ADE";
color_D = "#AF58BA";

plot \
  "data/pandemic.dat" index 1 \
  using 1:(log10(column(24))) title "US" with lines \
  linecolor rgb color_A lw 2, \
  "data/pandemic.dat" index 1 \
  using 1:(log10(column(17))) title "MX" with lines \
  linecolor rgb color_B lw 2, \
  "data/pandemic.dat" index 1 \
  using 1:(log10(column(15))) title "KR" with lines \
  linecolor rgb color_C lw 2, \
  "data/pandemic.dat" index 1 \
  using 1:(log10(column(11))) title "IN" with lines \
  linecolor rgb color_D lw 2 \

unset multiplot
