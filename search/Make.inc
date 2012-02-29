SEARCHOBJS:=\
	search/search.o\
	search/main.o\

include $(SEARCHOBJS:.o=.d)

search: search/search.a

EVERYTHING+=search

search/search.a: $(SEARCHOBJS)
	$(AR) rsc $@ $(SEARCHOBJS)

CLEAN+=$(SEARCHOBJS) search/search.a