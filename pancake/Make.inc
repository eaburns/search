BINS+=\
	pancake/8pancake_solver\

PCAKESRC=\
	pancake/pancake.cc\

PCAKEHDR=\
	pancake/pancake.hpp\

pancake/8pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(PCAKESRC)\
		-DNCAKES=8  pancake/main.cc -o pancake/8pancake_solver