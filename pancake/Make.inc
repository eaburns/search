BINS+=\
	pancake/50pancake_solver\
	pancake/55pancake_solver\
	pancake/60pancake_solver\
	pancake/65pancake_solver\
	pancake/70pancake_solver\

PCAKESRC=\
	pancake/pancake.cc\

PCAKEHDR=\
	pancake/pancake.hpp\

pancake/50pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(PCAKESRC)\
		-DNCAKES=50  pancake/main.cc -o pancake/50pancake_solver

pancake/55pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(PCAKESRC)\
		-DNCAKES=55  pancake/main.cc -o pancake/55pancake_solver

pancake/60pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(PCAKESRC)\
		-DNCAKES=60  pancake/main.cc -o pancake/60pancake_solver

pancake/65pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(PCAKESRC)\
		-DNCAKES=65  pancake/main.cc -o pancake/65pancake_solver

pancake/70pancake_solver: $(OBJS) $(HDRS) $(PCAKEHDR) $(PCAKESRC) pancake/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(PCAKESRC)\
		-DNCAKES=70  pancake/main.cc -o pancake/70pancake_solver