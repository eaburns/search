BINS:=\
	15unitmd_solver\

15unitmd_solver: $(OBJS) tiles/tiles.o tiles/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) tiles/main.cc -o 15unitmd_solver