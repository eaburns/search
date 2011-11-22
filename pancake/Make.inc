PCAKESRC=\
	pancake/pancake.cc\

PCAKEHDR=\
	pancake/pancake.hpp\

pancake/*.o: $(PCAKEHDR)

BINS+=\
	pancake/50pancake_solver\
	pancake/70pancake_solver\

pancake/50pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(PCAKESRC)\
		-DNCAKES=50  pancake/main.cc -o pancake/50pancake_solver

pancake/70pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(PCAKESRC)\
		-DNCAKES=70  pancake/main.cc -o pancake/70pancake_solver