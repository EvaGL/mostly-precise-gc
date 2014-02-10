SOURCE=b.cpp
CPPC=g++
LOAD_LIBRARY=-L /usr/local/lib/ -l gc -l pthread

all: output

output: $(SOURCE)
	$(CPPC) -std=c++11 $(SOURCE) $(LOAD_LIBRARY) -o output

clean:
	rm -rf output