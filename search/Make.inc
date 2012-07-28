search: search/search.a

SEARCHOBJS:=\
	search/search.o\

include $(SEARCHOBJS:.o=.d)

EVERYTHING+=search

search/search.a:\
	$(SEARCHOBJS)
	@echo $@
	@$(AR) rsc $@ $(SEARCHOBJS)

CLEAN+=$(SEARCHOBJS) search/search.a