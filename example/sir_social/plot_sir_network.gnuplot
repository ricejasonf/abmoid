set terminal pngcairo size 560,420 enhanced
set output 'img/sir_network.png'
set datafile separator ","

set title "SIR Network"
set key box opaque
set key box width 1.5
set key title center

set xlabel "Time"
set ylabel "Infected Pop."
set xrange [1:364]
set yrange [0:6000]

# Colors from https://www.molecularecologist.com/2020/04/23/simple-tools-for-mastering-color-in-scientific-figures/
color_A = "#FF1F5B";
color_B = "#00CD6C";
color_C = "#009ADE";
color_D = "#AF58BA";

plot "data/sir_network_infected.dat" index 1 \
             using 1:2 with lines title 'A' \
             linecolor rgb color_A lw 2, \
     "data/sir_network_infected.dat" index 1 \
             using 1:3 with lines title 'B' \
             linecolor rgb color_B lw 2, \
     "data/sir_network_infected.dat" index 1 \
             using 1:4 with lines title 'C' \
             linecolor rgb color_C lw 2, \
     "data/sir_network_infected.dat" index 1 \
             using 1:5 with lines title 'D' \
             linecolor rgb color_D lw 2, \
     "data/sir_network_infected.dat" index 1 \
             using 1:6 with lines title 'E' \
             linecolor rgb "black" lw 2, \
