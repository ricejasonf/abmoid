set terminal gif animate delay 50 size 560,420 enhanced loop 0
set output 'img/peak_times_2.gif'
set datafile separator ","

set title "Peak Time \"Bifurcation Diagram\" on Bridge Count"
set key box opaque 
set key box width 1.5 
set key title center

set xlabel "B.N"
set ylabel "Peak Time"
set xrange [0:9500]
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

do for [i = 1:9] {
  set multiplot
  plot "output_peak_times.dat" index i \
               using 3:4:(Br = strcol(1)) with lines title 'A.I' \
               linecolor rgb original_S lw 2, \
       "output_peak_times.dat" index i \
               using 3:5 with lines title 'B.I' \
               linecolor rgb original_I lw 2
      set label 1 "Br = " . Br left at screen 0.75, 0.55
  unset multiplot
}
