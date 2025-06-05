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

set xrange [30:784+100]
set yrange [699+25:241-25]

# Country indices for world.dat
$MAP_IDS << EOD
14 # BD
22 # BR
65 # CO
73 # DE
86 # EG
93 # ET
97 # FR
105 # GB
125 # ID
139 # IN
143 # IT
148 # JP
154 # KE
159 # KR
178 # MM
187 # MX
194 # NG
210 # PH
217 # PK
283 # TH
287 # TR
290 # TZ
293 # US
305 # VN
311 # ZA
EOD

set colorbox user
set cbrange[0:17000]
set palette defined(0 "white", 17000 "red")

color_max = 17001
color_norm(val) = int(real(val) / color_max * 256)
color(val) = floor(65536 * color_norm(val) + 256 * 0 + 0)
function $plot_country(map_index, val) << EOF
  plot 'data/world2.dat' index map_index using 1:2:(val) with filledcurves \
    lc palette
  plot 'data/world2.dat' index map_index with lines ls 2
EOF

function $plot_infecteds(co, t) << EOF
  set table $I_ROW
  plot "data/pandemic.dat" \
    index 1 \
    using 1:(int($MAP_IDS[co])):(column(co + 1)) \
    every ::t::t with table
  unset table
  void = $plot_country(int(word($I_ROW[1], 2)), int(word($I_ROW[1], 3)))
EOF


set output 'img/pandemic.png'
set multiplot
do for [i = 0:313] {
  plot 'data/world2.dat' index i with filledcurves ls 1
  plot 'data/world2.dat' index i with lines ls 2
}
# Draw China with I = 0 as a joke since they block facebook.
void = $plot_country(63, 0)

# Iterate the nations.
do for [i = 1:25] {
  void = $plot_infecteds(i, 125)
}
unset multiplot

set terminal gif animate size 640,480 enhanced loop 0
set output 'img/pandemic.gif'
do for [t = 0:363:8] {
set multiplot
set title sprintf("Global Pandemic (25 Largest Nations) t = %i", t)
do for [i = 0:313] {
  plot 'data/world2.dat' index i with filledcurves ls 1
  plot 'data/world2.dat' index i with lines ls 2
}
# Draw China with I = 0 as a joke since they block facebook.
void = $plot_country(63, 0)

# Iterate the nations.
do for [i = 1:25] {
  void = $plot_infecteds(i, t)
}
unset multiplot
}
