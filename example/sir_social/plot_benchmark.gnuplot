set terminal pngcairo size 560,420 enhanced
set output 'img/benchmark_1.png'
set datafile separator ","
unset key

set title "Benchmark: Single Group"
set rmargin 5.0
set xlabel "Agent Count"
set ylabel "Duration (ms)"
set xrange [0:200000]
set yrange [0:200]

COLOR = "#3C93C2"

plot "data/benchmark.dat" index 0 using 2:4 with lines notitle linecolor rgb COLOR lw 2

set terminal pngcairo size 560,420 enhanced
set output 'img/benchmark_2.png'
set datafile separator ","
unset key

set title "Benchmark: Network (Group Size 5000)"
set rmargin 5.0
set xlabel "Group Count"
set ylabel "Duration (ms)"
set xrange [0:10]
set yrange [0:200]

COLOR = "#3C93C2"

plot "data/benchmark.dat" index 1 using 1:4 with lines notitle linecolor rgb COLOR lw 2
