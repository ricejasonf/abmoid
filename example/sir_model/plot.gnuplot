set terminal pngcairo size 560,420 enhanced
set output 'img/agent_based_sir.png'
set datafile separator ","

set title "SIR Model: ODE vs Agent Based"
set key box opaque 
set key box width 1.5 
set key title center

# Define darker variants for Set 2
original_S = "#3C93C2"
original_I = "#22BB3B"
original_R = "#FC4E2A"

darker_S = "#C5E1EF"
darker_I = "#CDE5D2"
darker_R = "#FED976"

plot "output.dat" index 1:100 using ($0 * 100):1 with lines notitle linecolor rgb darker_S lw 2, \
     "output.dat" index 1:100 using ($0 * 100):3 with lines notitle linecolor rgb darker_R lw 2, \
     "output.dat" index 1:100 using ($0 * 100):2 with lines notitle linecolor rgb darker_I lw 2, \
     "output.dat" index 0 using 0:1 with lines title 'S' linecolor rgb original_S lw 2, \
     "output.dat" index 0 using 0:3 with lines title 'R' linecolor rgb original_R lw 2, \
     "output.dat" index 0 using 0:2 with lines title 'I' linecolor rgb original_I lw 2, \
