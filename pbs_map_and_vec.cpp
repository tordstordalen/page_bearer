#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "util.h"

struct MapAndVecPBS {


    std::unordered_map<u64, std::vector<u64>> map;
    const u64 epsilon;
    const char *structure_name = "PBS using std::map and std::vec";

    MapAndVecPBS(u64 epsilon) : epsilon(epsilon){
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

    inline pair<bool, u64> tryPredecessorInPage(u64 x, u64 id){
        if (pbHash(id) % epsilon != 0) return {false,0};
        
        auto pt = map.find(id);
        if (pt == map.end()) return {false,0};
        
        // elements is never empty
        auto elements = pt->second;
        u64 best = 0;
        bool found = false;
        for (auto e : elements){
            if (e >= best && e <= x) found = true, best = e;
        }
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

        double running = 0;
        for (auto e : bucket_lengths) lengths_summary[e]++;
        for (auto e : lengths_summary){
            running += (double)e / n_buckets;
            std::cout << running << " ";
            //std::cout << e << " ";
        } 
        std::cout << "\n";
    
    }
}; // PageBearerStructure end