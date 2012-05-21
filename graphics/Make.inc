GRAPHICSOBJS:=\
	graphics/scene.o\
	graphics/ui.o\

GRAPHICSLD:=-lSDL -lSDL_image -lGLU -lGL

include $(GRAPHICSOBJS:.o=.d)

CLEAN+=$(GRAPHICSOBJS)

graphics: graphics/graphics.a graphics/test

EVERYTHING+=graphics

graphics/graphics.a: $(GRAPHICSOBJS)
	@echo $@
	@$(AR) rsc $@ $(GRAPHICSOBJS)

CLEAN+=graphics.a

graphics/test: $(GRAPHICSOBJS) graphics/test.o utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include graphics/test.d
CLEAN+=graphics/test.o graphics/test