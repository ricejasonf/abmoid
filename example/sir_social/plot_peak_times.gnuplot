set terminal gif animate size 560,420 enhanced loop 0
set output 'img/peak_times.gif'
set datafile separator ","

set title "Peak Time \"Bifurcation Diagram\" on Bridge Count"
set key box opaque 
set key box width 1.5 
set key title center

set xlabel "Bridge Count"
set ylabel "Peak Time"
set xrange [1:100]
set yrange [0:364]

# Define darker variants for Set 2
original_S = "#3C93C2"
original_I = "#FC4E2A"
original_R = "#22BB3B"

darker_S = "#C5E1EF"
darker_I = "#FED976"
darker_R = "#CDE5D2"

do for [i = 0:37] {
  set multiplot
  plot "output_peak_times.dat" index i \
               using 1:4 with lines title 'A' \
               linecolor rgb original_S lw 2, \
       "output_peak_times.dat" index i \
               using 1:5 with lines title 'B' \
               linecolor rgb original_I lw 2
  unset multiplot
}
