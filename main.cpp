

#include <iostream>
#include <cstdint>
#include <vector>
#include <cassert>
#include <random>
#include <random>
#include <algorithm>

typedef std::mt19937 MTRng;  




typedef int64_t  i64;
typedef int32_t  i32;
typedef uint64_t u64; 
typedef uint32_t u32;

const u32 seed_val = 1337;    
MTRng rng;

//epsilon. The code assumes that e is a power of two. 
const u64 e = 8;

// a should be an odd integer chosen uniformly 
// at random at runtime for theoretical guarantees. 
inline u64 H(u64 x, u64 c){
    assert(c > 0 && "Output range must be non-empty");    
    const u64 a = 2187650952262969439;
    return (x * a) >> (64 - c);
}

struct RationalNumber {
    u64 enumerator;
    u64 denominator;
};

struct LineSegment { 
    RationalNumber slope; 
    RationalNumber intercept;
    u64 start; 
    u64 end;
};

// The data in sorted order, as well as the line segmets that represent the data. 
struct TestData {
    const std::vector<u64> data;              
    const std::vector<LineSegment> segments; 
};

void generateTestData(u64 n_data, u64 n_segments){
    std::cout << "printing test data\n";
    rng.seed(seed_val);

    std::uniform_int_distribution<u64> uniform;


    // Split the input data into n_segments chunks,
    // each of which have size at least two
    std::vector<u64> chunks;
    while (chunks.size() == 0){
        
        bool retry = false;
        // Generate chunks
        std::vector<u64> splits = {0, n_data};
        for (i32 i = 0; i < n_segments - 1; i++){
            splits.push_back(uniform(rng)%n_data);
        }
        std::sort(splits.begin(), splits.end());
        for (int i = 1; i < splits.size(); i++){
            u64 chunk = splits[i] - splits[i-1];
            if (chunk < 2){
                retry = true;
                break;
            }
            chunks.push_back(chunk);
        }
        if (retry) chunks.clear();
        
    }

    
    // Generate the n_segments lines
    std::vector<LineSegment> segments;
    u64 prev_value = uniform(rng) % 100000;

    for (i32 i = 0; i < n_segments; i++){
        RationalNumber slope; 

    }

    

}








int main(void){

    generateTestData(120000, 4);
    std::cout << "Hello world\n";

    return 0;
}
