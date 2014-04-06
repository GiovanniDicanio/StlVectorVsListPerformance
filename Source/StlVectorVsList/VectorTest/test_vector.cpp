///////////////////////////////////////////////////////////////////////////////
//
// Benchmarking std::vector random insert and removal.
//
// by Giovanni Dicanio <giovanni.dicanio@gmail.com>
//
// 2014, April 5th
//
// Description
// ===========
// A pseudo-random sequence of integers 1,2,3,...N is generated.
// These integers are first inserted in a std::vector in sorted order.
// Then they are removed from pseudo-random positions.
// The insertion and removal time is measured.
//
//
// Compilation notes
// =================
// To comple in test mode from command line, can do:
//
//      cl /EHsc /W4 /nologo /DTEST_MODE test_vector.cpp
//
// To compile in optimized mode from command line, can do:
//
//      cl /EHsc /W4 /MT /GL /O2 /nologo test_vector.cpp
//
///////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//                              Includes
//-----------------------------------------------------------------------------

#include <algorithm>        // For std::lower_bound(), std::shuffle()
#include <exception>        // For std::exception
#include <iostream>         // For console output
#include <random>           // For std::random_device, std::mt19937, 
                            // std::uniform_int_distribution
#include <stdexcept>        // For standard C++ exception classes
#include <string>           // For std::stoi()
#include <vector>           // For std::vector
#include <windows.h>        // For high-resolution timers
using namespace std;


// Compile in test mode to verify that the logic of the code is correct. 
//#define TEST_MODE

#ifdef TEST_MODE

// Prints the content of a sequence (e.g. a std::vector).
template <typename Sequence>
void print(const Sequence& s) {
    if (s.empty()) {
        cout << "<< empty >>";
        return;
    }
    
    for (const auto & x : s) {
        cout << x << ' ';
    }
    cout << endl;
}
#endif // TEST_MODE


//-----------------------------------------------------------------------------
//                          Performance Measurement
//-----------------------------------------------------------------------------

long long counter() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}
 
long long frequency() {
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    return li.QuadPart;
}

void print_time(const long long start, const long long finish, 
                const char * const s) {
    cout << s << ": " << (finish - start) * 1000.0 / frequency() << " ms" 
         << endl;
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//                          The Main Benchmark
//-----------------------------------------------------------------------------

// Console mode app entry-point.
// Specify number of items as the only command line parameter.
int main(int argc, char* argv[]) {
    static const int kExitOk = 0;
    static const int kExitError = 0;
    
    try {
        cout << "std::vector insertion/remove benchmark -- by Giovanni Dicanio"
             << endl << endl;
             
#ifndef TEST_MODE

        // Parse the command line to get the number of items.
        if (argc != 2) {
            cout << "Invalid command line.\n" << endl;
            cout << "Syntax: " << argv[0] << " <N>\n"
                 << "where <N> is an integer representing the number"
                 << " of items used for testing." << endl;
            return kExitError;
        }
        
        // Get the number of items from the command line string parameter.
        const int N = stoi(argv[1]);
        if (N <= 0) {
            throw invalid_argument(
                "Invalid number of items (must be > 0)");
        }
#else
        // In test mode, just assume 5 items.
        const int N = 5;
        cout << "*** TEST MODE (assuming " << N << " items) ***\n" << endl;
#endif // TEST_MODE        
        
        
        // Used to initialize the seed for the psuedo-random number generator.
        random_device rd;
        
        // Pseudo-random number generator, with seed from random_device.
        mt19937 prng(rd());
        
        // Numbers 1,2,3...,N in pseudo random order.
        // First generate the 1,2,3,...,N sequence, then shuffle it.
        vector<int> values(N);
        for (int i = 0; i < N; ++i) {
            values[i] = (i+1);
        }
        shuffle(values.begin(), values.end(), prng);
        

        // Generate a pseudo-random sequence of remove indexes.
        vector<int> remove_indexes(N);
        for (int i = 0; i < (N-1); ++i) {    
            // Note that the maximum remove index changes (decremented by one)
            // after each removal.
            //
            // i = 0   -->   valid indexes: 0,1,2...,(N-1)
            // i = 1   -->   valid indexes: 0,1,2...,(N-2)
            // i = 2   -->   valid indexes: 0,1,2,..,(N-3)
            // :::
            // i = N-1 -->   valid index: 0            

            // Generate pseudo-random number in range 0,1,2,...,(N-i-1).
            uniform_int_distribution<> dist(0, N-i-1);
            const int index = dist(prng);            
            remove_indexes[i] = index;
        }
        // There's just the last item left in the sequence to be removed,
        // and since the sequence has just one item, its index is 0.
        remove_indexes[N-1] = 0; 


        long long start = 0;
        long long finish = 0;

        // *** TIME MEASUREMENT START ***
        start = counter();
        {
            // Insert the psuedo-random sequence [1, N] in the vector,
            // in sorted order.
            vector<int> v;
            for (const auto& x : values) {            
                auto pos = lower_bound(v.begin(), v.end(), x);
                v.insert(pos, x); 
        
#ifdef TEST_MODE
            cout << "Inserting " << x << ":  ";
            print(v);
#endif // TEST_MODE
            }

#ifdef TEST_MODE
            cout << "\nComplete sequence: ";
            print(v); 
            cout << endl;
#endif // TEST_MODE
           
            // Remove from pseudo-random positions
            // (previously stored in remove_indexes).
            for (int index : remove_indexes) {
#ifndef TEST_MODE
                v.erase(v.begin() + index);
#else
                // Test mode code
                const int value = v[index];
                v.erase(v.begin() + index);
                cout << "Removing " << value 
                     << " (at index " << index << "):  "; 
                print(v);
#endif // TEST_MODE
            }

#ifdef TEST_MODE       
            cout << '\n' << endl;
#endif // TEST_MODE
               
        } // Measure time including destructors!
               
        // *** TIME MEASUREMENT END ***
        finish = counter();
        
        // Print total time.
        print_time(start, finish, "std::vector time");

        // All right.
        return kExitOk;
    
    } catch(const exception& e) {
        // Trap exceptions and print the corresponding error message.
        cerr << "\n*** ERROR: " << e.what() << endl;
        return kExitError;
    }
}

///////////////////////////////////////////////////////////////////////////////
