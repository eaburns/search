graphics: graphics/graphics.a graphics/test

GRAPHICSOBJS:=\
	graphics/image.o\
	graphics/scene.o\
	graphics/ui.o\

GRAPHICSLD:=-lSDL -lSDL_image -lGLU -lGL

include $(GRAPHICSOBJS:.o=.d)

CLEAN+=$(GRAPHICSOBJS)

EVERYTHING+=graphics

graphics/graphics.a: $(GRAPHICSOBJS)
	@echo $@
	@$(AR) rsc $@ $(GRAPHICSOBJS)

CLEAN+=graphics/graphics.a

# A simple test binary
graphics/test:\
	$(GRAPHICSOBJS)\
	graphics/test.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include graphics/test.d
CLEAN+=graphics/test.o graphics/test