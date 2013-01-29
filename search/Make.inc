search:\
	search/search.a\
	search/anyprof/sptcounts\
	search/anyprof/sptpolicy\
	search/anyprof/bestwt\
	search/learnh/hlms\

SEARCHOBJS:=\
	search/search.o\
	search/anyprof/profile.o\

include $(SEARCHOBJS:.o=.d)

search: search/search.a search/anyprof/mkprof

EVERYTHING+=search

search/search.a:\
	$(SEARCHOBJS)\
	 $(RDBOBJS)
	@echo $@
	@$(AR) rsc $@ $(SEARCHOBJS) $(RDBOBJS)

CLEAN+=$(SEARCHOBJS) search/search.a

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

search/anyprof/sptpolicy:\
	search/anyprof/sptpolicy.cc\
	$(SEARCHOBJS)\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=search/anyprof/sptpolicy

search/anyprof/bestwt:\
	search/anyprof/bestwt.cc\
	$(SEARCHOBJS)\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=search/anyprof/bestwt

search/learnh/hlms:\
	search/learnh/leastsquares.cc\
	search/learnh/hlms.cc\
	$(SEARCHOBJS)\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ -llapack $(LDFLAGS) -o $@
CLEAN+=search/learnh/hlms

TESTS+=.search_test

.search_test: ./search/test
	./search/test ".*test" && touch $@

CLEAN+=.search_test

search/test_closedlist.o: search/closedlist.hpp

search/test:\
	search/test_closedlist.o\
	search/test.cc\
	utils/utils.a\
	structs/structs.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@