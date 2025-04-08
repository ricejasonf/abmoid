set terminal pngcairo size 560,420 enhanced
set output 'img/peak_times.png'
set datafile separator ","

set title "Peak Time \"Bifurcation Diagram\" on Bridge Count"
set key box opaque 
set key box width 1.5 
set key title center

set xlabel "Bridge Count"
set ylabel "Peak Time"

# Define darker variants for Set 2
original_S = "#3C93C2"
original_I = "#FC4E2A"
original_R = "#22BB3B"

darker_S = "#C5E1EF"
darker_I = "#FED976"
darker_R = "#CDE5D2"

plot "output_peak_times.dat" using 1:2 with lines title 'A' linecolor rgb original_S lw 2, \
     "output_peak_times.dat" using 1:3 with lines title 'B' linecolor rgb original_I lw 2, \
