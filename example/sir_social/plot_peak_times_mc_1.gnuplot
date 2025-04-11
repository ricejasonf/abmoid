set terminal pngcairo size 560,420 enhanced
set output 'img/plot_peak_times_mc_1.png'
set datafile separator ","

#set title "Peak Time \"Bifurcation Diagram\" on Bridge Count"
#set key box opaque 
#set key box width 1.5 
#set key title center

#set xlabel "Bridge Count"
#set ylabel "Peak Time"
set xrange [5:1005]
set yrange [30:100]
#set ytics autofreq 20
set tics font ",10"

# Define darker variants for Set 2
original_S = "#3C93C2"
original_I = "#FC4E2A"
original_R = "#22BB3B"

darker_S = "#C5E1EF"
darker_I = "#FED976"
darker_R = "#CDE5D2"

alpha_S = "#dd3C93C2"
alpha_I = "#ddFC4E2A"
alpha_R = "#dd22BB3B"

#set size ratio 1
#set size 0.70, 0.5

set multiplot 
plot for [i = 900:999] \
      "output_peak_times_mc.dat" index i \
               using 1:4:(A = strcol(2), B = strcol(3)) with lines notitle \
               linecolor rgb alpha_S lw 2

set label 1 "A = " . A left at screen 0.75, 0.55 front
set label 2 "B = " . B left at screen 0.75, 0.50 front

plot for [i = 900:999] \
       "output_peak_times_mc.dat" index i \
               using 1:5 with lines notitle \
               linecolor rgb alpha_I lw 2
unset multiplot
