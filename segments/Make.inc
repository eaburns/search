segments:\
	segments/segments_solver\
	segments/mkinst\

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
