segments:\
	segments/segments_solver\
	segments/mkinst\
	segments/watch\

SEGMENTSOBJ=\
	segments/segments.o\

include $(SEGMENTSOBJ:.o=.d)
CLEAN+=$(SEGMENTSOBJ)

EVERYTHING+=segments

segments/segments_solver:\
	$(SEGMENTSOBJ)\
	segments/main.o\
	utils/utils.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include segments/main.d
CLEAN+=segments/segments_solver segments/main.o

segments/mkinst:\
	$(SEGMENTSOBJ)\
	segments/mkinst.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include segments/mkinst.d
CLEAN+=segments/mkinst segments/mkinst.o

segments/watch:\
	$(SEGMENTSOBJ)\
	segments/watch.cc\
	graphics/graphics.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include segments/watch.d
CLEAN+=segments/watch segments/watch.o