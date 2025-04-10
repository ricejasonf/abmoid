c.out: peak_infections_mc.cpp peak_times.hpp sir_social.hpp
	$(CXX) -O3 -std=c++23 -I../../include/ peak_infections_mc.cpp -o c.out

output_peak_times_mc.dat: c.out
	./c.out > output_peak_times_mc.dat
