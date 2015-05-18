#!/usr/bin/python
from subprocess import *
class GC:
    def __init__(self, name, flags=[]):
        self.name = name
        self.flags = flags

    def compile(self, test="GCBench.cpp", flags = []):
	args = ["g++", "-o", self.name.lower(), test ,"-D" + self.name ] + flags + self.flags
        call(args)

    def run(self, parser=id):
        args = ["./" + self.name.lower()]
        p = Popen(args, stdout = PIPE, stderr=STDOUT)
        return parser(p.stdout.readlines())

def gcbench_parser(lines):
    top = 0.0
    bottom = 0.0
    total = 0.0
    gc = 0
    for line in lines:
       line = line.strip()
       if line.startswith("Top"):
           top += float(line.split()[-2])
       elif line.startswith("Bottom"):
           bottom += float(line.split()[-2])
       elif line.startswith("Compl"):
           total = float(line.split()[-2])
       elif line.startswith("gc"):
           gc += 1
    return (top, bottom, total, gc)    

if __name__ == '__main__':
    NUM = 20
    GCs = [
           GC("NO_GC"),
           GC("SHARED", ["-std=c++11"]), 
           GC("MOSTLY_PRECISE", ["-std=c++11","-lmsmalloc","-lprecisegc", "-lpthread"]),
           GC("GCPP", ["-std=c++11", "-lgc++", "-I/usr/include/gc++"]),
          ]
    print("---- Boehm gc bench ----")
    print(" top | bottom | total (in ms average of %d)" % (NUM, ))
    for size in [8, 10, 12, 14, 16, 18]:
        print("======= Depth: %d =======" % size)
    	for gc in GCs:
            gc.compile("GCBench.cpp", ['-DTREE_SIZE=' + str(size)])
            top = 0.0
            bottom = 0.0
            total = 0.0
            for i in xrange(NUM):
                data = gc.run(gcbench_parser)
                top += data[0]
                bottom += data[1]
                total += data[2]     
	    print "%s : %dms %dms %dms" % (gc.name, top / NUM, bottom / NUM, total / NUM) 
        print " "     
