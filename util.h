#ifndef PAGE_BEARER_UTIL
#define PAGE_BEARER_UTIL

#include <cstdint>
#include <chrono>
#include <utility>

typedef int64_t   i64;
typedef int32_t   i32;
typedef uint64_t  u64; 
typedef uint32_t  u32;

using std::pair; 


u64 nowNanos(){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
#endif