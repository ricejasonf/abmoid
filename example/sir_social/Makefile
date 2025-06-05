d.out: pandemic.cpp sir_social.hpp country_connections.hpp national_pops.hpp
	$(CXX) -O3 -std=c++23 -I../../include/ pandemic.cpp -o d.out

c.out: peak_infections_mc.cpp peak_times.hpp sir_social.hpp
	$(CXX) -O3 -std=c++23 -I../../include/ peak_infections_mc.cpp -o c.out

output_peak_times_mc.dat: c.out
	./c.out > output_peak_times_mc.dat

plot_peak_times_mc_1.png: output_peak_times_mc.dat
	gnuplot plot_peak_times_mc_1.gnuplot

sir_network.out : sir_network.cpp sir_social.hpp
	$(CXX) -O3 -std=c++23 -I../../include/ sir_network.cpp -o sir_network.out

data/sir_network_infected.dat: sir_network.out
	./sir_network.out

plot_sir_network.png: data/sir_network_infected.dat
	gnuplot plot_sir_network.gnuplot

plot_sir_network_graph.png: data/sir_network_infected.dat
	gnuplot plot_sir_network_graph.gnuplot

plot_pandemic.png: data/pandemic.dat data/world2.dat
	gnuplot plot_pandemic.gnuplot
