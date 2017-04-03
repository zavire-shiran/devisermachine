CC := g++
#Debug/unit test
CCFLAGS := -g -O0 -Wall -Wextra -std=c++0x #-fprofile-arcs -ftest-coverage
#Release
#CCFLAGS := -O2 -Wall -Wextra -std=c++0x
LD := g++
LDFLAGS := -ledit -fprofile-arcs -ftest-coverage
TESTLDFLAGS := -fprofile-arcs -ftest-coverage

all: deviser interpretertests

deviser: main.o deviser.o lineeditor.o
	$(LD) -o $@ $^ $(LDFLAGS)

main.o: main.cpp deviser.hpp lineeditor.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

deviser.o: deviser.cpp deviser.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

lineeditor.o: lineeditor.cpp lineeditor.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

interpretertests: deviser.o tests/test.o
	$(LD) -o $@ $^ ../gmock-1.7.0/make/gmock_main.a $(TESTLDFLAGS) 

tests/test.o: tests/test.cpp deviser.hpp
	$(CC) -c $(CCFLAGS) -I../gmock-1.7.0/gtest/include -o $@ $<

runtests: interpretertests deviser
	lcov --directory . --zerocounters
	./interpretertests
	./deviser -e '(import (testsuite))' -e '(test)'
	./deviser -e '(import (assembler-test))' -e '(test)'
	mkdir -p testout
	lcov --directory . --capture --output-file testout/interpretertest.out
	(cd testout; genhtml interpretertest.out)

.PHONY: clean
clean:
	rm -f *.o tests/*.o deviser interpretertests
	find . -iname \*.gcno -delete
	find . -iname \*.gcda -delete
	rm -rf testout
