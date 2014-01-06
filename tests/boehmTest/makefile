SOURCE=b.cpp

all: output

output: $(SOURCE)
	g++ -std=c++11 $(SOURCE)  -L ../../ -l gc -l pthread -o output

clean:
	rm -rf output