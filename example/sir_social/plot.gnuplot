set terminal pngcairo size 560,420 enhanced
set output 'img/sir_social_1.png'
set datafile separator ","

set title "SIR Social Groups (1 vs. 2)"
set key box opaque 
set key box width 1.5 
set key title center

# Define darker variants for Set 2
original_S = "#3C93C2"
original_I = "#FC4E2A"
original_R = "#22BB3B"

darker_S = "#C5E1EF"
darker_I = "#FED976"
darker_R = "#CDE5D2"

plot "output.dat" index 0 using 0:1 with lines title 'S' linecolor rgb darker_S lw 2, \
     "output.dat" index 0 using 0:3 with lines title 'R' linecolor rgb darker_R lw 2, \
     "output.dat" index 0 using 0:2 with lines title 'I' linecolor rgb darker_I lw 2, \
     "output.dat" index 1 using 0:1 with lines title 'S_2' linecolor rgb original_S lw 2, \
     "output.dat" index 1 using 0:3 with lines title 'R_2' linecolor rgb original_R lw 2, \
     "output.dat" index 1 using 0:2 with lines title 'I_2' linecolor rgb original_I lw 2
