CC := g++ -g -Wall -Wextra
CCFLAGS := -std=c++0x
LD := g++
LDFLAGS := -ledit

deviser: main.o deviser.o lineeditor.o
	$(LD) -o $@ $^ $(LDFLAGS)

main.o: main.cpp deviser.hpp lineeditor.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

deviser.o: deviser.cpp deviser.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

lineeditor.o: lineeditor.cpp lineeditor.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f *.o deviser
