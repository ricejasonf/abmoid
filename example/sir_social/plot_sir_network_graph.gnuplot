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
color_A = 0xFF1F5B;
color_B = 0x00CD6C;
color_C = 0x009ADE;
color_D = 0xAF58BA;
color_E = 0xF28522;

$COLORS << EOD
color_A
color_B
color_C
color_D
color_E
EOD
print $COLORS

set table $POPS
  plot "data/sir_network_infected.dat" index 0 with table
unset table

# Social group node position and edge data.
$POINTS << EOD
A, 0.8, 2.3
B, 1.6, 2.2
C, 2.4, 1.7
D, 3.2, 0.8
E, 1.2, 0.6
EB, 1.6, 2.2 # E -> B
EOD
print $POINTS

set table $POPS_POINTS separator comma
  plot $POINTS every ::0::4 \
    using (stringcolumn(1)):2:3:(word($POPS[1], int($0 + 2))*.00006):(value($COLORS[$0 + 1])) \
    with table
unset table
print $POPS_POINTS

set multiplot
# Plot the edges.
plot $POINTS using 2:3 with lines linecolor rgb "black" lw 2

set style fill solid
plot $POPS_POINTS using 2:3:4:5 with circles fillcolor rgb variable, \
     $POPS_POINTS using 2:3:(stringcolumn(1)) with labels
unset multiplot
