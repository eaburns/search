bench: bench/mark

EVERYTHING+=bench

bench: bench/mark

bench/mark: bench/mark.cc utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

CLEAN+=bench/mark