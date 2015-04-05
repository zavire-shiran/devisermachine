CC := g++ -g
CCFLAGS := -std=c++0x
LD := g++
LDFLAGS :=

deviser: main.o deviser.o
	$(LD) -o $@ $^ $(LDFLAGS)

main.o: main.cpp deviser.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

deviser.o: deviser.cpp deviser.hpp
	$(CC) -c $(CCFLAGS) -o $@ $<

clean:
	rm -f *.o deviser
