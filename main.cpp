
#include <cstdint>
#include <cassert>

#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>

using std::pair; 

typedef std::mt19937 MTRng;  

typedef int64_t   i64;
typedef int32_t   i32;
typedef uint64_t  u64; 
typedef uint32_t  u32;

const u32 seed_val = 53315113;    
MTRng rng;


u64 nowNanos(){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

struct MapAndVecPBS {


    std::unordered_map<u64, std::vector<u64>> map;
    const u64 epsilon = 8;
    const char *structure_name = "PBS using std::map and std::vec";

    MapAndVecPBS(){
        std::vector<u64> v; 
        v.push_back(0);
        map.insert({0,v});
    }

    // We use normal multiply-shift, which is not sufficient in theory due 
    // to not distributing uniformly. In particular, since we are hashing to 
    // 64-bit keys, odd inputs are never congruent 0 mod epsilon. We force
    // each input to be even for this reason. Note that a must be a uniformly
    // chosen integer at runtime for theoretical guarantees. 
    u64 pbHash(u64 x) {
        //x &= 0xFFFFFFFE; 
        const u64 a = 2187650952262969439;
        //const u64 b = 2349073786287317910;
        return (a * x); // + b;
    }

    bool tryInsertInPage(u64 x, u64 id){
        if (pbHash(id) % epsilon != 0) return false;
        auto pt = map.find(id);
        if (pt == map.end()) return false;

        u64 xid = x / epsilon;
        if (pbHash(xid) % epsilon != 0 || xid == id){
            // x is not a page bearer, or x is a page bearer but xid == id 
            for (auto e : pt->second){
                if (e == x) return true;
            }
            pt->second.push_back(x);
        }
        else {
            // x is a page bearer different from id; we split the page ID and create a new one
            std::vector<u64> x_page;
            map.insert({xid, x_page});
            auto xpt = map.find(xid);

            xpt->second.push_back(x);
            i32 i = 0; 
            while (i < pt->second.size()){
                if (pt->second[i] >= x) {
                    xpt->second.push_back(pt->second[i]);
                    pt->second[i] = pt->second.back();
                    pt->second.pop_back();
                }
                else {
                    i++;
                    
                }
            }
        }

        return true;
    }

    pair<bool, u64> tryPredecessorInPage(u64 x, u64 id){
        if (pbHash(id) % epsilon != 0) return {false,0};
        
        auto pt = map.find(id);
        if (pt == map.end()) return {false,0};
        
        // elements is never empty
        auto elements = pt->second;
        u64 best = 0;
        bool found = false;
        for (auto e : elements){
            if (e >= best && e <= x) found = true, best = e;
        }std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return {found, best};
    }

    bool tryDeleteInPage(u64 x, u64 id){
        std::cout << "Delete not implemented\n";
        exit(1);
    }

    u64 size(){
        u64 total_size = 0;
        for (auto& [key,val] : map) total_size += val.size();
        return total_size;
    }

    void print(){
        std::cout << "\n ----- The structure looks like ----- \n";

        for (auto& [key,val] : map){
            std::cout << key << ": ";
            for (auto e : val) std::cout << e << ", ";
            std::cout << "\n";
        }
        std::cout << "\n-------------------------\n";
    }


    void printStatistics(){
        u64 ratio = size()/epsilon;
        u64 n_buckets = map.size();
        std::cout << "#buckets = " << n_buckets << "\n"; 
        std::cout << "expected #buckets = " << ratio << "\n"; 
        std::cout << "the ratio is " << ((double)map.size())/((double)ratio) << "\n";

        std::vector<u64> bucket_lengths; 
        for (auto& [key,val] : map) bucket_lengths.push_back(val.size());
        std::sort(bucket_lengths.begin(), bucket_lengths.end());


        u64 average_bucket_length = (double)size()/(double)map.size();
        u64 max_bucket_length = bucket_lengths.back();
        std::cout << "Average bucket length: " << average_bucket_length << "\n";
        std::cout << "Median  bucket length: " << bucket_lengths[bucket_lengths.size()/2] << "\n";
        std::cout << "Largest bucket length: " << max_bucket_length << "\n";

        std::vector<u64> lengths_summary(bucket_lengths.back() + 1);

        for (auto e : bucket_lengths) lengths_summary[e]++;
        for (auto e : lengths_summary) std::cout << e << " ";
        std::cout << "\n";
    
    }
}; // PageBearerStructure end

// PBS = page bearer structure
template<typename PBS>
struct TestStructure {
    std::set<u64> data;
    PBS pbs;
    const u64 epsilon = 8;
    u64 total_time = 0;

    TestStructure(){
        data.insert(0);
        pbs.tryInsertInPage(0,0);
    }

    u64 predecessor(u64 x){
        auto pt = data.upper_bound(x);
        pt--;
        return *pt;
    } 

    auto pbIterator(u64 x){

        auto pt = data.lower_bound(x);
        if (pt == data.end() || *pt/epsilon > x/epsilon) pt--;
        pt++;
        return pt;
    }


    bool testPredecessor(u64 x){
        
        u64 real_predecessor = predecessor(x);
        u64 ta,tb;
        u64 pbs_pred;
        auto pt = pbIterator(x);
        while (pt != data.begin()){
            pt--;
            ta = nowNanos();
            auto [found,pred] = pbs.tryPredecessorInPage(x, *pt/epsilon);
            tb = nowNanos();
            total_time += tb - ta;
            if (found) {
                pbs_pred = pred;
                break;
            }
        }

        auto correct = real_predecessor == pbs_pred;
        //if (!correct) std::cout << "Predecessor of " << x << " is " << real_predecessor << " but got " << pbs_pred << "\n";

        return correct;
    }

    void insert(u64 x){
        if (x == 0) std::cout << "Inserting 0...\n";
        auto pt = pbIterator(x);
        auto inserted = false;
        u64 ta,tb;
        while (!inserted && pt != data.begin()){
            pt--;
            ta = nowNanos();
            inserted |= pbs.tryInsertInPage(x,*pt/epsilon);
            tb = nowNanos();
            total_time += tb - ta;
            //if (inserted) std::cout << "Inserted " << x << " at id " << *pt/epsilon << "\n";

        }
        if (!inserted) std::cout << "Could not insert " << x << "\n";
        
        data.insert(x);
    }

    void remove(u64 x){
        if (x == 0) std::cout << "Removing 0...\n";
        auto pt = data.upper_bound(x);
        while (pt != data.begin()){
            pt--;
            if (pbs.tryDeleteInPage(x, *pt/epsilon)) break;
        }
        data.erase(x);
    } 

    void printSizeStatistics(){
        std::cout << "The size should be " << data.size() << " and is " << pbs.size() << "\n";
        pbs.printStatistics();
    }

    void printTimeStatistics(){
        std::cout << "Total time taken by PBS structure (milliseconds): " << (double)total_time/1000000 << "\n";
    }

    void printStructureInfo(){
        std::cout << "Running test with structure '" << pbs.structure_name << "'\n";
    }

};

template<typename PBS>
void runInsertAndPredecessorTest(u64 n){
    rng.seed(seed_val);
    std::uniform_int_distribution<u64> uniform;


    TestStructure<PBS> ts;

    u64 n_updates = 0;
    u64 n_queries = 0;
    u64 n_error = 0; 


    ts.printStructureInfo();

    
    std::vector<u64> data; 
    for (int i = 0; i < n; i++){
        data.push_back(uniform(rng));
    }

    for (auto e : data){
        if (e & 0b1) {
            ts.insert(e >> 1);
            n_updates++;
        }
        else {
            n_queries++;
            if (!ts.testPredecessor(e >> 1)) n_error++;
        }
    }

    ts.printTimeStatistics();
    //ts.printSizeStatistics();

   
    //std::cout << "total size should be " << n_updates << " and is " << ts.pbsSize() << "\n";

    std::cout << "Total queries: " << n_queries << "\nWrong answers: " << n_error << "\n";
}

struct BBST_PBS {

    std::set<u64> data;
    const char *structure_name = "PBS using a std::set";

    pair<bool,u64> tryPredecessorInPage(u64 x, u64 page_id){
        auto pt = data.upper_bound(x);
        pt--;
        u64 pred = *pt;
        return {true, pred};
    }

    bool tryInsertInPage(u64 x, u64 page_id){
        data.insert(x);
        return true;
    }

    bool tryDeleteInPage(u64 x, u64 page_id){
        data.erase(x);
        return true;
    }
};

int main(void){
    
    runInsertAndPredecessorTest<MapAndVecPBS>(10000000);
    
    return 0;
}
