set terminal pngcairo size 560,420 enhanced
set output 'img/sir_network_graph.png'
set datafile separator ","

set title "SIR Network Diagram"
set key box opaque
set key box width 1.5
set key title center

#set xlabel "Time"
#set ylabel "Infected Pop."
set xrange [0:4]
set yrange [0:3]

# Colors from https://www.molecularecologist.com/2020/04/23/simple-tools-for-mastering-color-in-scientific-figures/
color_A = "#FF1F5B";
color_B = "#00CD6C";
color_C = "#009ADE";
color_D = "#AF58BA";
color_E = "black";

set table $POPS
  plot "data/sir_network_infected.dat" index 0 with table
unset table

# Social group node position and edge data.
$POINTS << EOD
0.8, 2.3 # A
1.6, 2.2 # B
2.4, 1.7 # C
3.2, 0.8 # D
1.2, 0.6 # E
1.6, 2.2 # B
EOD

set table $POPS_POINTS separator comma
  plot $POINTS every ::0::4 using 1:2:(word($POPS[1], int($0 + 2))*.00006) with table
unset table
print $POPS_POINTS

set multiplot
# Plot the edges.
plot $POINTS with lines linecolor rgb "black" lw 2

set style fill solid
plot $POPS_POINTS using 1:2:3 with circles fillcolor rgb color_A
unset multiplot
