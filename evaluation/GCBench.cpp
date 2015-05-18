//#define SHARED
//#define NO_GC
// This is adapted from a benchmark written by John Ellis and Pete Kovac
// of Post Communications.
// It was modified by Hans Boehm of Silicon Graphics.
// Translated to C++ 30 May 1997 by William D Clinger of Northeastern Univ.
//
//      This is no substitute for real applications.  No actual application
//      is likely to behave in exactly this way.  However, this benchmark was
//      designed to be more representative of real applications than other
//      Java GC benchmarks of which we are aware.
//      It attempts to model those properties of allocation requests that
//      are important to current GC techniques.
//      It is designed to be used either to obtain a single overall performance
//      number, or to give a more detailed estimate of how collector
//      performance varies with object lifetimes.  It prints the time
//      required to allocate and collect balanced binary trees of various
//      sizes.  Smaller trees result in shorter object lifetimes.  Each cycle
//      allocates roughly the same amount of memory.
//      Two data structures are kept around during the entire process, so
//      that the measured performance is representative of applications
//      that maintain some live in-memory data.  One of these is a tree
//      containing many pointers.  The other is a large array containing
//      double precision floating point numbers.  Both should be of comparable
//      size.
//
//      The results are only really meaningful together with a specification
//      of how much memory was used.  It is possible to trade memory for
//      better time performance.  This benchmark should be run in a 32 MB
//      heap, though we don't currently know how to enforce that uniformly.
//
//      Unlike the original Ellis and Kovac benchmark, we do not attempt
//      measure pause times.  This facility should eventually be added back
//      in.  There are several reasons for omitting it for now.  The original
//      implementation depended on assumptions about the thread scheduler
//      that don't hold uniformly.  The results really measure both the
//      scheduler and GC.  Pause time measurements tend to not fit well with
//      current benchmark suites.  As far as we know, none of the current
//      commercial Java implementations seriously attempt to minimize GC pause
//      times.

using namespace std;
#include <iostream>
#include <sys/time.h>
#include "config.h"

//  These macros were a quick hack for the Macintosh.
//
//  #define currentTime() clock()
//  #define elapsedTime(x) ((1000*(x))/CLOCKS_PER_SEC)

#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

/* Get the current time in milliseconds */

unsigned
stats_rtclock( void )
{
  struct timeval t;
  struct timezone tz;

  if (gettimeofday( &t, &tz ) == -1)
    return 0;
  return (t.tv_sec * 1000 + t.tv_usec / 1000);
}

static const int kStretchTreeDepth    = TREE_SIZE;      // about 16Mb
static const int kLongLivedTreeDepth  = 16;  // about 4Mb
static const int kArraySize  = 500000;  // about 4Mb
static const int kMinTreeDepth = 4;
static const int kMaxTreeDepth = 16;

struct Node0 {
        ptype(Node0) left;
        ptype(Node0) right;
        int i, j;
        Node0(ptype(Node0) l, ptype(Node0) r) { left = l; right = r; }
        Node0() { setnull(left); setnull(right); }
#       if HAS_DESTRUCTOR == 1
          ~Node0() { if (left) free(left); if (right) free(right); }
#	endif
};

struct GCBench {

        // Nodes used by a tree of a given size
        static int TreeSize(int i) {
                return ((1 << (i + 1)) - 1);
        }

        // Number of iterations to use for a given tree depth
        static int NumIters(int i) {
                return 2 * TreeSize(kStretchTreeDepth) / TreeSize(i);
        }

        // Build tree top down, assigning to older objects.
        static void Populate(int iDepth, ptype(Node0) thisNode) {
                if (iDepth<=0) {
                        return;
                } else {
                        iDepth--;
                        thisNode->left  = alloc(Node0);
                        thisNode->right = alloc(Node0);
                        Populate (iDepth, thisNode->left);
                        Populate (iDepth, thisNode->right);
                }
        }

        // Build tree bottom-up
        static ptype(Node0) MakeTree(int iDepth) {
                if (iDepth<=0) {
               	    return alloc(Node0);
                } else {
                    return alloc_args(Node0, MakeTree(iDepth-1), MakeTree(iDepth-1));
                }
        }

        static void PrintDiagnostics() {
#if 0
                long lFreeMemory = Runtime.getRuntime().freeMemory();
                long lTotalMemory = Runtime.getRuntime().totalMemory();

                System.out.print(" Total memory available="
                                 + lTotalMemory + " bytes");
                System.out.println("  Free memory=" + lFreeMemory + " bytes");
#endif
        }

        static void TimeConstruction(int depth) {
                long            tStart, tFinish;
                int             iNumIters = NumIters(depth);
                ptype(Node0)    tempTree;

                cout << "Creating " << iNumIters
                     << " trees of depth " << depth << endl;
                
                tStart = currentTime();
                for (int i = 0; i < iNumIters; ++i) {
                    tempTree = alloc(Node0);
                    Populate(depth, tempTree);
                    free(tempTree);
                    setnull(tempTree);
                }
                tFinish = currentTime();
                cout << "\tTop down construction took "
                     << elapsedTime(tFinish - tStart) << " msec" << endl;
                     
                tStart = currentTime();
                for (int i = 0; i < iNumIters; ++i) {
                        tempTree = MakeTree(depth);
                        free(tempTree);
                        setnull(tempTree);
                }
                tFinish = currentTime();
                cout << "\tBottom up construction took "
                     << elapsedTime(tFinish - tStart) << " msec" << endl;

        }

        void main() {
                ptype(Node0)    root;
                ptype(Node0)    longLivedTree;
                ptype(Node0)    tempTree;
                long    tStart, tFinish;
                long    tElapsed;

#ifdef GC
// GC_full_freq = 30;
GC_enable_incremental();
#endif
                cout << "Garbage Collector Test" << endl;
                cout << " Live storage will peak at "
                     << 2 * sizeof(Node0) * TreeSize(kLongLivedTreeDepth) +
                        sizeof(double) * kArraySize
                     << " bytes." << endl << endl;
                cout << " Stretching memory with a binary tree of depth "
                     << kStretchTreeDepth << endl;
                PrintDiagnostics();
               
                tStart = currentTime();
                
                // Stretch the memory space quickly
                tempTree = MakeTree(kStretchTreeDepth);
                free(tempTree);
                setnull(tempTree);

                // Create a long lived object
                cout << " Creating a long-lived binary tree of depth "
                     << kLongLivedTreeDepth << endl;
                longLivedTree = alloc(Node0);
                Populate(kLongLivedTreeDepth, longLivedTree);

                // Create long-lived array, filling half of it
              //  cout << " Creating a long-lived array of "
              //       << kArraySize << " doubles" << endl;
              //  ptype(double) array = alloc_array(double, kArraySize);
              //  for (int i = 0; i < kArraySize/2; ++i) {
              //          array[i] = 1.0/i;
              //  }
             //   PrintDiagnostics();

                for (int d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2)
{
                        TimeConstruction(d);
                }

              //  if (longLivedTree == 0 || array[1000] != 1.0/1000)
               //         cout << "Failed" << endl;
                                        // fake reference to LongLivedTree
                                        // and array
                                        // to keep them from being optimized away

                tFinish = currentTime();
                tElapsed = elapsedTime(tFinish-tStart);
                PrintDiagnostics();
                cout << "Completed in " << tElapsed << " msec" << endl;
#		ifdef GC
		  cout << "Completed " << GC_gc_no << " collections" <<endl;
		  cout << "Heap size is " << GC_get_heap_size() << endl;
#		endif
        }
};

main () {
    GCBench x;
    x.main();
}

