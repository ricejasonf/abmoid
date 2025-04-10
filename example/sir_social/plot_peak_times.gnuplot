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

set size ratio 1
set size 0.70, 1.0

do for [i = 0:8] {
  set multiplot

  plot "output_peak_times.dat" index i \
               using 1:4:(A = strcol(2), B = strcol(3)) with lines title 'A' \
               linecolor rgb original_S lw 2, \
       "output_peak_times.dat" index i \
               using 1:5 with lines title 'B' \
               linecolor rgb original_I lw 2
      set label 1 "A = " . A left at screen 0.75, 0.55
      set label 2 "B = " . B left at screen 0.75, 0.50
  unset multiplot
}
