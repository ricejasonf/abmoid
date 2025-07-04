set datafile separator ","
#set key box opaque
#set key box width 1.5
#set key title center

#set xlabel "Time"
#set ylabel "Infected Pop."
set xrange [0:4]
set yrange [0:3]
unset xtics
unset ytics

# Colors from https://www.molecularecologist.com/2020/04/23/simple-tools-for-mastering-color-in-scientific-figures/
color_A = 0xFF1F5B;
color_B = 0x00CD6C;
color_C = 0x009ADE;
color_D = 0xAF58BA;
color_E = 0xF28522;
color_Sick = 0xFFC61E;

$COLORS << EOD
color_A
color_B
color_C
color_D
color_E
EOD

set table $POPS
  plot "data/sir_network_infected.dat" index 0 with table
unset table

# Social group node position and edge data.
$POINTS << EOD
A, 0.8, 2.3
B, 1.6, 2.2
C, 2.4, 1.7
D, 3.2, 1.0
E, 1.2, 0.8
EB, 1.6, 2.2 # E -> B
EOD

function $scale_radius(value) << EOF
  if (value <= 0) {
    return 0;
  } else {
    return log10(value) * 0.1;
  }
EOF

set table $POPS_POINTS separator comma
  plot $POINTS every ::0::4 \
    using (stringcolumn(1)):2:3:($scale_radius(word($POPS[1], int($0 + 2)))):(value($COLORS[$0 + 1])) \
    with table
    #using (stringcolumn(1)):2:3:(word($POPS[1], int($0 + 2))*.00006):(value($COLORS[$0 + 1])) \
    with table
unset table

function $plot_base() << EOF
  # Plot the edges.
  plot $POINTS using 2:3 notitle with lines linecolor rgb "black" lw 2

  # Plot social group nodes with labels.
  set style fill solid
  plot $POPS_POINTS using 2:3:4:5 notitle with circles fillcolor rgb variable, \
       $POPS_POINTS using 2:3:(stringcolumn(1)) notitle with labels
EOF

function $plot_infecteds(t) << EOF
  # The word function doesn't like commas I guess.
  set table $I_ROW separator tab
  plot "data/sir_network_infected.dat" \
        index 1 every ::t::t with table
  unset table
  set table $I_POINTS separator comma
    plot $POINTS every ::0::4 \
      using (stringcolumn(1)):2:3:($scale_radius(word($I_ROW[1], int($0 + 2)))) \
      with table
  unset table
  set style fill solid
  plot $I_POINTS using 2:3:4 notitle with circles fillcolor rgb color_Sick, \
       $I_POINTS using 2:3:(stringcolumn(1)) notitle with labels
EOF

function $full_plot(t) << EOF
  set multiplot
    set title sprintf("SIR Network Diagram (t = %i)", t)
    result = $plot_base()
    result = $plot_infecteds(t)
  unset multiplot
EOF


# Cannot seem to do both without losing data.
set terminal gif animate size 560,420 enhanced loop 0
set output 'img/sir_network_graph.gif'
do for [t = 0:225] {
  anim = $full_plot(t)
}

set terminal pngcairo size 560,420 enhanced
set output 'img/sir_network_graph_1.png'
result = $full_plot(0)
set output 'img/sir_network_graph_2.png'
result = $full_plot(33)
set output 'img/sir_network_graph_3.png'
result = $full_plot(66)
set output 'img/sir_network_graph_4.png'
result = $full_plot(99)

