search:\
	search/search.a\
	search/anyprof/mkprof\
	search/anyprof/sptcounts\

SEARCHOBJS:=\
	search/search.o\
	search/anyprof.o\
	search/anyprof/profile.o\

include $(SEARCHOBJS:.o=.d)

search: search/search.a search/mkprof

EVERYTHING+=search

search/search.a:\
	$(SEARCHOBJS)
	@echo $@
	@$(AR) rsc $@ $(SEARCHOBJS)

CLEAN+=$(SEARCHOBJS) search/search.a

search/mkprof:\
	search/mkprof.cc\
	$(SEARCHOBJS)\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=search/mkprof

search/anyprof/mkprof:\
	search/anyprof/mkprof.cc\
	$(SEARCHOBJS)\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=search/anyprof/mkprof

search/anyprof/sptcounts:\
	search/anyprof/sptcounts.cc\
	$(SEARCHOBJS)\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=search/anyprof/sptcounts