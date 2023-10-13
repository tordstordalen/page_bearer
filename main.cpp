#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "util.h"

#include "pbs_map_and_vec.cpp"
#include "pbs_linear_probing.cpp"

// PBS = page bearer structure
template<typename PBS>
struct TestStructure {
    std::set<u64> data;
    const u64 epsilon = 8;
    PBS pbs = PBS(epsilon); 
    u64 total_time = 0;
    u64 naive_time = 0;

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
        u64 ta,tb,tc,td;
        u64 pbs_pred;
        tc = nowNanos();
        auto pt = pbIterator(x);
        td = nowNanos();
        naive_time += td - tc;
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
        u64 tc = nowNanos();
        auto inserted = false;
        u64 td = nowNanos();
        naive_time += td - tc;
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
        std::cout << "The time by the naive solution is " << (double)naive_time / 1000000 << " milliseconds\n";
    }

    void printStructureInfo(){
        std::cout << "Running test with structure '" << pbs.structure_name << "'\n";
    }

};

typedef std::mt19937 MTRng;  
const u32 seed_val = 53315113;    
MTRng rng;

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
    ts.printSizeStatistics();

   
    //std::cout << "total size should be " << n_updates << " and is " << ts.pbsSize() << "\n";

    std::cout << "Total queries: " << n_queries << "\nWrong answers: " << n_error << "\n";
}

struct BBST_PBS {

    std::set<u64> data;
    const char *structure_name = "PBS using a std::set";

    BBST_PBS(u64 epsilon){

    }

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

    u64 size(){
        return data.size();
    }

    void printStatistics(){}
};

int main(void){
    
    runInsertAndPredecessorTest<MapAndVecPBS>(2000000);
    
    return 0;
}
