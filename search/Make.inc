SEARCHOBJS:=\
	search/search.o\
	search/main.o\
	search/anyprof.o\

include $(SEARCHOBJS:.o=.d)

search: search/search.a search/mkprof

EVERYTHING+=search

search/search.a: $(SEARCHOBJS)
	$(AR) rsc $@ $(SEARCHOBJS)

CLEAN+=$(SEARCHOBJS) search/search.a

search/mkprof: search/mkprof.cc $(SEARCHOBJS) rdb/rdb.a utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=search/mkprof