#pragma once

#include "Compat.h"
#include "FixedSizeMap.h"
#include "BigAlloc.h"


const int MAX_K = 31;


// Computes the edit distance between two strings without returning the edits themselves.
class LandauVishkin {
public:
    LandauVishkin(int cacheSize = 0);

    ~LandauVishkin();

    // Compute the edit distance between two strings, if it is <= k, or return -1 otherwise.
    // For LandauVishkin instances with a cache, the cacheKey should be a unique identifier for
    // the text and pattern combination (e.g. (readID << 33) | direction << 32 | genomeLocation).
    int computeEditDistance(
            const char* text,
            int textLen, 
            const char* pattern,
            const char *qualityString,
            int patternLen,
            int k,
            double *matchProbability,
            _uint64 cacheKey = 0);

    // Version that does not requre match probability and quality string
    inline int computeEditDistance(
            const char* text,
            int textLen,
            const char* pattern,
            int patternLen,
            int k)
    {
        return computeEditDistance(text, textLen, pattern, NULL, patternLen, k, NULL);
    }

    // Clear the cache of distances computed.
    void clearCache();

    void *operator new(size_t size) {return BigAlloc(size);}
    void operator delete(void *ptr) {BigDealloc(ptr);}

    static void setProbabilities(double *i_indelProbabilities, double *i_phredToProbability, double mutationProbability);
    static void initializeProbabilitiesToPhredPlus33();

private:
    // TODO: For long reads, we should include a version that only has L be 2 x (2*MAX_K+1) cells
    int L[MAX_K+1][2 * MAX_K + 1];
    
    // Action we did to get to each position: 'D' = deletion, 'I' = insertion, 'X' = substitution.  This is needed to compute match probability.
    char A[MAX_K+1][2 * MAX_K + 1];

    // Arrays for backtracing the actions required to match two strings
    char backtraceAction[MAX_K+1];
    int  backtraceMatched[MAX_K+1];
    int  backtraceD[MAX_K+1];

    struct LVResult {
        int k;
        int result;
        double matchProbability;

        LVResult() { k = -1; result = -1; }

        LVResult(int k_, int result_, double matchProbability_) { k = k_; result = result_; matchProbability = matchProbability_;}

        inline bool isValid() { return k != -1; }
    };

    FixedSizeMap<_uint64, LVResult> *cache;

    static double *indelProbabilities;  // Maps indels by length to probability of occurance.
    static double *phredToProbability;  // Maps ASCII phred character to probability of error, including 
    static double *perfectMatchProbability; // Probability that a read of this length has no mutations
};


// Computes the edit distance between two strings and returns a CIGAR string for the edits.

enum CigarFormat
{
    COMPACT_CIGAR_STRING = 0,
    EXPANDED_CIGAR_STRING = 1,
    COMPACT_CIGAR_BINARY = 2,
};

class LandauVishkinWithCigar {
public:
    LandauVishkinWithCigar();

    // Compute the edit distance between two strings and write the CIGAR string in cigarBuf.
    // Returns -1 if the edit distance exceeds k or -2 if we run out of space in cigarBuf.
    int computeEditDistance(const char* text, int textLen, const char* pattern, int patternLen, int k,
                            char* cigarBuf, int cigarBufLen, bool useM, CigarFormat format = COMPACT_CIGAR_STRING);
private:
    int L[MAX_K+1][2 * MAX_K + 1];
    
    // Action we did to get to each position: 'D' = deletion, 'I' = insertion, 'X' = substitution.
    char A[MAX_K+1][2 * MAX_K + 1];

    // Arrays for backtracing the actions required to match two strings
    char backtraceAction[MAX_K+1];
    int backtraceMatched[MAX_K+1];
    int backtraceD[MAX_K+1];
};
