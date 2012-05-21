SEARCHOBJS:=\
	search/search.o\

include $(SEARCHOBJS:.o=.d)

search: search/search.a

EVERYTHING+=search

search/search.a: $(SEARCHOBJS)
	@echo $@
	@$(AR) rsc $@ $(SEARCHOBJS)

CLEAN+=$(SEARCHOBJS) search/search.a