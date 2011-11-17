PLAT2DSRC=\
	plat2d/lvl.cc\
	plat2d/tile.cc\
	plat2d/geom.cc\
	plat2d/player.cc\

PLAT2DHDR=\
	plat2d/lvl.hpp\
	plat2d/tile.hpp\
	plat2d/geom.hpp\
	plat2d/player.hpp\

plat2d/*.o: $(PLAT2DHDR)

BINS+=\
	plat2d/draw\

plat2d/draw: $(OBJS) $(HDRS) $(PLAT2DHDR) $(PLAT2DSRC) plat2d/draw.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(PLAT2DSRC)\
		plat2d/draw.cc -o plat2d/draw