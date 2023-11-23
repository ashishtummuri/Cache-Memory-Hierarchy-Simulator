//Header files:
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <utility>

/* Storing the results
result[0] - L1 cache
result[1] - L2 cache
result[2] - Main Memory*/
struct results{
    long read;
    long readmiss;
    long write;
    long writemiss;
    long writeback;
    long traffic;
    double missrate;
    long prefetch;
    long read_prefetch;
    long readmiss_prefetch;
}result[3];

// Storing the cache specifications
struct CacheParams{
    int BLOCKSIZE;
    int L1_SIZE;
    int L1_ASSOC;
    int L2_SIZE;
    int L2_ASSOC;
    int PREF_N;
    int PREF_M;
    std::string trace_file;
};

// Storing the values for each cache block
struct Cache_flags {
    bool valid;
    bool dirty;
    int lru;
    int tag;
    int address;
};

// Storing the values for each stream buffer
struct Stream_flags{
    bool valid;
    int address;
    int tag;
    int lru;
};

// Storing the translated addresses for L1 & L2 cache
struct para{
    int offset;
    int index;
    int tag;
    int SETS;
};

// Declaring the L1 and L2 cache flags
Cache_flags** cache;
Cache_flags** cache2;
Stream_flags** streamBuffer;

// Alloting memory to L1 and L2 cache
void allot_mem(CacheParams &params){
    // L1 cache allotment
    const int SETS_L1 = params.L1_SIZE / ((params.L1_ASSOC * params.BLOCKSIZE)==0? 1: (params.L1_ASSOC * params.BLOCKSIZE));
    cache = new Cache_flags*[SETS_L1];
    for (int i = 0; i < SETS_L1; i++) {
        cache[i] = new Cache_flags[params.L1_ASSOC];
        for (int j = 0; j < params.L1_ASSOC; j++) {
            // Initialize cache entries
            cache[i][j].valid = false; 
            cache[i][j].dirty = false;
            cache[i][j].lru=j;
        }
    }

    if(params.L2_SIZE!=0){
        // L2 cache allotment
        const int SETS_L2 = params.L2_SIZE / ((params.L2_ASSOC * params.BLOCKSIZE)==0? 1: (params.L2_ASSOC * params.BLOCKSIZE));
        cache2 = new Cache_flags*[SETS_L2];
        for (int i = 0; i < SETS_L2; i++) {
            cache2[i] = new Cache_flags[params.L2_ASSOC];
            for (int j = 0; j < params.L2_ASSOC; j++) {
                // Initialize cache entries
                cache2[i][j].valid = false;
                cache2[i][j].dirty = false;
                cache2[i][j].lru=j;
            }
        }
    }

    if(params.PREF_N !=0){
        //Stream Buffer allotment
        streamBuffer = new Stream_flags*[params.PREF_N];
        for (int i = 0; i < params.PREF_N; i++) {
            streamBuffer[i] = new Stream_flags[params.PREF_M];
            streamBuffer[i]->valid=false;
            streamBuffer[i]->lru=i;
            for (int j = 0; j < params.PREF_M; j++) {
                streamBuffer[i][j].valid = false;
            }
        }
    }
}

// Cleaning up memory alloted to L1 and L2 cache
void cleanup(int SETS_L1, int SETS_L2, CacheParams &params){
    for (int i = 0; i < SETS_L1; i++) {
        delete[] cache[i];
    }
    delete[] cache;

    if(params.L2_SIZE!=0){
        for (int i = 0; i < SETS_L2; i++) {
            delete[] cache2[i];
        }
        delete[] cache2;
    }

    if(params.PREF_N !=0){
        for(int i = 0; i< params.PREF_N; i++){
            delete[] streamBuffer[i];
        }
        delete[] streamBuffer;
    }
}

// Printing the results 
void Print_results(){
    std::cout << "\n===== Measurements =====\n";
    /*****L1********/
    std::cout << "a. L1 reads:"
        << "\t\t\t" <<std::dec <<result[0].read << "\n";
    std::cout << "b. L1 read misses:"
        << "\t\t" << result[0].readmiss << "\n";
    std::cout << "c. L1 writes:"
        << "\t\t\t" << result[0].write<< "\n";
    std::cout << "d. L1 write misses:"
        << "\t\t" << result[0].writemiss << "\n";
    if(result[0].read + result[0].write !=0)
        std::cout << "e. L1 miss rate:"
            << "\t\t" << std::fixed << std::setprecision(4) << static_cast<float>(result[0].readmiss + result[0].writemiss) / static_cast<float>(result[0].read + result[0].write) << "\n";
    else
        std::cout << "e. L1 miss rate:"
            << "\t\t" << std::fixed << std::setprecision(4) << 0.f << "\n";
    std::cout << "f. L1 writebacks:"
        << "\t\t" << result[1].writeback << "\n";
    std::cout << "g. L1 prefetches:"
        << "\t\t" << result[0].prefetch << "\n";
    /*****L2********/
    std::cout << "h. L2 reads (demand):"
        << "\t\t" << result[1].read << "\n";
    std::cout << "i. L2 read misses (demand):"
        << "\t" << result[1].readmiss << "\n";
    std::cout << "j. L2 reads (prefetch):"
        << "\t\t" << result[1].read_prefetch<< "\n";
    std::cout << "k. L2 read misses (prefetch):"
        << "\t" << result[1].readmiss_prefetch <<"\n";
    std::cout << "l. L2 writes:"
        << "\t\t\t" << result[1].write<< "\n";
    std::cout << "m. L2 write misses:"
        << "\t\t" << result[1].writemiss << "\n";
    if(result[1].read !=0)
        std::cout << "n. L2 miss rate:"
            << "\t\t" << std::fixed << std::setprecision(4) << static_cast<float>(result[1].readmiss) / static_cast<float>(result[1].read)<< "\n";
    else    
        std::cout << "n. L2 miss rate:"
            << "\t\t" << std::fixed << std::setprecision(4)<< 0.f << "\n";

    /*****MEMORY********/
    std::cout << "o. L2 writebacks:"
        << "\t\t" << result[2].writeback << "\n";
    std::cout << "p. L2 prefetches:"
        << "\t\t" << result[1].prefetch << "\n";
    std::cout << "q. memory traffic:"
        << "\t\t" << result[2].traffic << "\n";
}

//Updating LRU for stream buffer
void updateStreamLRU(int num, CacheParams &params){
    int current_stream_lru=streamBuffer[num]->lru;
    for(int i = 0; i < params.PREF_N;i++){
        if(streamBuffer[i]->lru < current_stream_lru){
            streamBuffer[i]->lru++;
        }
    }
    cache[num]->lru=0;
}

// Initialising the buffer
void init_Buffer(int address, CacheParams &params ){
    bool full = true;
    for(int i =0 ; i < params.PREF_N ;i++){
        if(streamBuffer[i]->valid==0){
            streamBuffer[i]->valid=true;
            for(int j=0; j< params.PREF_M; j++){
                streamBuffer[i][j].address=address+j+1;
                // result[1].prefetch++;
            }
            updateStreamLRU(i,params);
            break;
        }
    }

    if(full){
        for(int i =0 ; i < params.PREF_N ;i++){
            if(streamBuffer[i]->lru == params.PREF_M-1){
                streamBuffer[i]->valid=true;
                for(int j=0; j< params.PREF_M; j++){
                    streamBuffer[i][j].address=address+j+1;
                }
                updateStreamLRU(i,params);
                break;
            }
        }
    }
}

// Searching the Buffer
std::pair<int,int> search_Buffer(int address, CacheParams &params){
    //search the stream buffers in order of recency and return the first hit
    std::pair<int,int> hit(-1,-1);
    int hit_lru=std::numeric_limits<int>::max();
    for(int i = 0; i < params.PREF_N; i++){
        for(int j=0; j< params.PREF_M; j++){
            if(streamBuffer[i][j].address == address && streamBuffer[i]->lru < hit_lru){
                //found a hit
                hit.first=i;
                hit.second=j;
                hit_lru=streamBuffer[i]->lru;
            }
        }
    }
    return hit;
}

// Managing the buffer
void manage_Buffer(int address, std::pair<int,int> hit, CacheParams &params){
    for(int i=0;i<params.PREF_M;i++){
        streamBuffer[hit.first][i].address=address + i + 1;
    }   
}

// Getting and storing the translated address for the cache
para getPara(int address, int SIZE, int ASSOC, int BLOCKSIZE){
    para temp;
    // Calculate cache parameters
    if(ASSOC!=0){
        const int SETS = SIZE / (ASSOC * BLOCKSIZE);
        const int INDEX = log2(SETS);
        const int OFFSET = log2(BLOCKSIZE);

        // Extract tag, index, and offset from the address
        temp.offset = address & ((1 << OFFSET) - 1);
        temp.index = (address >> OFFSET) & ((1 << INDEX) - 1);
        temp.tag = address >> (OFFSET + INDEX);
        temp.SETS=SETS;
    }
    return temp;
}

// LRU Algorithm
void updateLRU(int index, para P, int column, bool hit, CacheParams &params){
    int current_lru;
    if(hit){
        if(index == 0){
            current_lru=cache[P.index][column].lru;
            for(int i = 0; i < params.L1_ASSOC;i++){
                if(cache[P.index][i].lru<current_lru){
                    cache[P.index][i].lru++;
                }
            }
            cache[P.index][column].lru=0;
        }
        if(index == 1){
            current_lru=cache2[P.index][column].lru;
            for(int i = 0; i < params.L2_ASSOC;i++){
                if(cache2[P.index][i].lru<current_lru){
                    cache2[P.index][i].lru++;
                }
            }
            cache2[P.index][column].lru=0;
        }
    }
    else{
        int maximum=std::numeric_limits<int>::min();
        int max_index=0;
        if(index == 0){
            for(int i = 0; i < params.L1_ASSOC;i++){
                if(cache[P.index][i].lru> maximum){
                    maximum = cache[P.index][i].lru;
                    max_index=i;
                }
            }
            for(int i = 0; i < params.L1_ASSOC;i++){
                if(max_index!=i){
                    cache[P.index][i].lru++;
                }
            }
            cache[P.index][max_index].lru=0;
        }
        if(index == 1){
            for(int i = 0; i < params.L2_ASSOC;i++){
                if(cache2[P.index][i].lru> maximum){
                    maximum = cache2[P.index][i].lru;
                    max_index=i;
                }
            }
            for(int i = 0; i < params.L2_ASSOC;i++){
                if(max_index!=i){
                    cache2[P.index][i].lru++;
                }
            }
            cache2[P.index][max_index].lru=0;
        }
    }
}
 
void writeBack(Cache_flags** cache, int index, int address, int SIZE, int ASSOC, int BLOCKSIZE, CacheParams &params){
    // Getting translated addresses
    para P=getPara(address, SIZE, ASSOC, BLOCKSIZE);
    bool hit = false;
    // Searching for the tag in the cache
    for (int i = 0; i < ASSOC; i++) {
        // Case 1: tag found and dirty bit is 1
        if(cache2[P.index][i].valid && cache2[P.index][i].tag == P.tag){
            hit=true;
            cache2[P.index][i].dirty = true;
            result[index].write++;
            result[index].writeback++;// L1 to L2
            updateLRU(index, P, i, hit, params);
            break;
        }
    }

    // If tag is not found in cache
    if (!hit) {
        if(index == 1){
            // Writing the entry to cache & updating results
            result[index].write++;
            result[index].writeback++;
            bool full=true;
            for(int i =0; i < ASSOC ; i++){
                if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                    cache2[P.index][i].tag=P.tag;
                    cache2[P.index][i].valid=true;
                    cache2[P.index][i].dirty=true;
                    cache2[P.index][i].address=address;
                    updateLRU(index, P, i, true , params);
                    full=false;
                }
            }
            if(full){
                for (int i = 0; i < ASSOC; i++) {
                    if (cache2[P.index][i].lru == (ASSOC-1)) {
                        if(cache2[P.index][i].dirty == 1){
                            result[index+1].writeback++;
                            result[index+1].traffic++;
                        }
                        cache2[P.index][i].tag=P.tag;
                        cache2[P.index][i].valid=true;
                        cache2[P.index][i].dirty=true;
                        cache2[P.index][i].address=address;
                        updateLRU(index, P, i, true , params);
                        break;
                    }
                }
            }
        }
    } 
}

//  Function to read from cache
void readCache(Cache_flags** cache_R, int index, char rw, int address, int SIZE, int ASSOC, int BLOCKSIZE, CacheParams &params){
    
    // Getting translated addresses  
    para P=getPara(address, SIZE, ASSOC, BLOCKSIZE);
    bool hit = false;
    // Search for the tag in the cache
    if(index==0){
        for (int i = 0; i < ASSOC; i++) {
            if (cache[P.index][i].valid && cache[P.index][i].tag == P.tag) {
                hit = true;
                if (rw == 'r') {
                    result[index].read++;
                }
                updateLRU(index, P, i, hit, params);
                break;
            }
        }
    }
    else if(index == 1){
        for (int i = 0; i < ASSOC; i++) {
            if (cache2[P.index][i].valid == 1 && cache2[P.index][i].tag == P.tag) {
            hit = true;
            if (rw == 'r') {
                result[index].read++;
            }
            updateLRU(index, P, i, hit, params);
            break;
            }
        }
    }

    if(hit){
        std::pair<int,int> search=search_Buffer(address, params);
        if(search.first != -1){
            updateStreamLRU(search.first, params);
            manage_Buffer(address,search, params);
        }
    }

    // If tag is not found in cache
    if (!hit) {
        if (rw == 'r') {
            result[index].read++;
            result[index].readmiss++;
            if(index == 0){// If this is L1
                // Sending read command to L2
                if(params.L2_SIZE!=0){
                    // Writing the returned value to L1
                    bool full=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                            cache[P.index][i].tag=P.tag;
                            cache[P.index][i].valid=true;
                            cache[P.index][i].dirty=false;
                            cache[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            full=false;
                            break;
                        }
                    }
                    if(full){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache[P.index][i].lru == (ASSOC-1)) {
                                if(cache[P.index][i].dirty == 1){
                                    writeBack(cache2, index+1, cache[P.index][i].address, params.L2_SIZE, params.L2_ASSOC, BLOCKSIZE, params);
                                }
                                cache[P.index][i].tag=P.tag;
                                cache[P.index][i].valid=true;
                                cache[P.index][i].dirty=false;
                                cache[P.index][i].address=address; 
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                    readCache(cache2, 1, 'r', address, params.L2_SIZE, params.L2_ASSOC, params.BLOCKSIZE, params);

                }
                
                else{
                    if(params.PREF_N!=0){
                        std::pair<int,int> search = search_Buffer(address, params);
                        if(search.first == -1){
                            init_Buffer(address,params);
                            result[index+2].traffic++;
                            // Writing to L2 cache
                            bool full=true;
                            for(int i =0; i < ASSOC ; i++){
                                if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                                    cache[P.index][i].tag=P.tag;
                                    cache[P.index][i].valid=true;
                                    cache[P.index][i].dirty=false;
                                    cache[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    full=false;
                                    break;
                                }
                            }
                            if(full){
                                for (int i = 0; i < ASSOC; i++) {
                                    if (cache[P.index][i].lru == (ASSOC-1)) {
                                        if(cache[P.index][i].dirty == 1){
                                            result[index+1].writeback++;
                                            result[index+2].traffic++;
                                        }
                                        cache[P.index][i].tag=P.tag;
                                        cache[P.index][i].valid=true;
                                        cache[P.index][i].dirty=false;
                                        cache[P.index][i].address=address;
                                        updateLRU(index, P, i, true , params);
                                        break;
                                    }
                                }
                            }
                        }
                    
                        else if(search.first != -1){
                            result[index].readmiss--;
                            // result[index].read--;
                            // result[index].prefetch++;
                            updateStreamLRU(search.first, params);
                            manage_Buffer(address,search,params);
                            bool full=true;
                            for(int i =0; i < ASSOC ; i++){
                                if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                                    cache[P.index][i].tag=P.tag;
                                    cache[P.index][i].valid=true;
                                    cache[P.index][i].dirty=false;
                                    cache[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    full=false;
                                    break;
                                }
                            }
                            if(full){
                                for (int i = 0; i < ASSOC; i++) {
                                    if (cache[P.index][i].lru == (ASSOC-1)) {
                                        if(cache[P.index][i].dirty == 1){
                                            result[index+1].writeback++;
                                            result[index+2].traffic++;
                                        }
                                        cache[P.index][i].tag=P.tag;
                                        cache[P.index][i].valid=true;
                                        cache[P.index][i].dirty=false;
                                        cache[P.index][i].address=address;
                                        updateLRU(index, P, i, true , params);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else{
                        bool full=true;
                        result[index+2].traffic++;
                        for(int i =0; i < ASSOC ; i++){
                            if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                                cache[P.index][i].tag=P.tag;
                                cache[P.index][i].valid=true;
                                cache[P.index][i].dirty=false;
                                cache[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                full=false;
                                break;
                            }
                        }
                        if(full){
                            for (int i = 0; i < ASSOC; i++) {
                                if (cache[P.index][i].lru == (ASSOC-1)) {
                                    if(cache[P.index][i].dirty == 1){
                                        result[index+1].writeback++;
                                        result[index+2].traffic++;
                                    }
                                    cache[P.index][i].tag=P.tag;
                                    cache[P.index][i].valid=true;
                                    cache[P.index][i].dirty=false;
                                    cache[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    break;
                                }
                            }
                        }
                    }
                } 
            }
            
            if(index == 1){// If this is L2
                // getting data from memory
                if(params.PREF_N!=0){
                    std::pair<int,int> search = search_Buffer(address, params);
                    if(search.first == -1){
                        init_Buffer(address,params);
                        result[index+1].traffic++;
                        // Writing to L2 cache
                        bool full=true;
                        for(int i =0; i < ASSOC ; i++){
                            if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=false;
                                cache2[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                full=false;
                                break;
                            }
                        }
                        if(full){
                            for (int i = 0; i < ASSOC; i++) {
                                if (cache2[P.index][i].lru == (ASSOC-1)) {
                                    if(cache2[P.index][i].dirty == 1){
                                        result[index+1].writeback++;
                                        result[index+1].traffic++;
                                    }
                                    cache2[P.index][i].tag=P.tag;
                                    cache2[P.index][i].valid=true;
                                    cache2[P.index][i].dirty=false;
                                    cache2[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    break;
                                }
                            }
                        }
                    }
                    
                    else if(search.first != -1){
                        result[index].readmiss--;
                        // result[index].read--;
                        // result[index].prefetch++;
                        updateStreamLRU(search.first, params);
                        manage_Buffer(address,search,params);
                        bool full=true;
                        for(int i =0; i < ASSOC ; i++){
                            if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=false;
                                cache2[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                full=false;
                                break;
                            }
                        }
                        if(full){
                            for (int i = 0; i < ASSOC; i++) {
                                if (cache2[P.index][i].lru == (ASSOC-1)) {
                                    if(cache2[P.index][i].dirty == 1){
                                        result[index+1].writeback++;
                                        result[index+1].traffic++;
                                    }
                                    cache2[P.index][i].tag=P.tag;
                                    cache2[P.index][i].valid=true;
                                    cache2[P.index][i].dirty=false;
                                    cache2[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    break;
                                }
                            }
                        }
                    }
                }

                else{
                    bool full=true;
                    result[index+1].traffic++;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=false;
                            cache2[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            full=false;
                            break;
                        }
                    }
                    if(full){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    result[index+1].writeback++;
                                    result[index+1].traffic++;
                                }
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=false;
                                cache2[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Function to write to cache
void writeCache(Cache_flags** cache_R, int index, char rw, int address, int SIZE, int ASSOC, int BLOCKSIZE, CacheParams &params){
    
    para P=getPara(address, SIZE, ASSOC, BLOCKSIZE);
    bool hit = false;
    // Search for the address in the L1 cache
    if(index==0){
        for (int i = 0; i < ASSOC; i++) {
            // Check if the L1 cache entry is valid and the tag matches
                if (cache[P.index][i].valid && cache[P.index][i].tag == P.tag) {
                // L1 cache hit
                hit = true;
                if (rw == 'w') {
                    result[index].write++;
                    updateLRU(index, P, i, hit, params);
                    if(index==0)
                        cache[P.index][i].dirty = true;
                    else if(index==1)
                        cache2[P.index][i].dirty = true;
                }
                break;
            }
        }
    }
    else if(index ==1 ){
        for (int i = 0; i < ASSOC; i++) {
                if (cache2[P.index][i].valid && cache2[P.index][i].tag == P.tag) {
                hit = true;
                if (rw == 'w') {
                    result[index].write++;
                    updateLRU(index, P, i, hit, params);
                    if(index==0)
                        cache[P.index][i].dirty = true;
                    else if(index==1)
                        cache2[P.index][i].dirty = true;
                }
                break;
            }
        }
    }

    if(hit){
        std::pair<int,int> search=search_Buffer(address, params);
        if(search.first != -1){
            updateStreamLRU(search.first, params);
            manage_Buffer(address, search, params);
        }
    }

    if (!hit) {
        if (rw == 'w') {
            result[index].write++;
            result[index].writemiss++;
            if(index == 0){// If this is L1
                // Search in L2 cache
                if(params.L2_SIZE!=0){
                    //Writing returned tag to L1
                    bool full=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                            cache[P.index][i].tag=P.tag;
                            cache[P.index][i].valid=true;
                            cache[P.index][i].dirty=true;
                            cache[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            full=false;
                            break;
                        }
                    }
                    if(full){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache[P.index][i].lru == (ASSOC-1)) {
                                if(cache[P.index][i].dirty == 1){
                                    writeBack(cache2, index+1, cache[P.index][i].address, params.L2_SIZE, params.L2_ASSOC, BLOCKSIZE, params);
                                }
                                cache[P.index][i].tag=P.tag;
                                cache[P.index][i].valid=true;
                                cache[P.index][i].dirty=true;
                                cache[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                    readCache(cache2, 1, 'r', address, params.L2_SIZE, params.L2_ASSOC, params.BLOCKSIZE, params);

                }
                
                else{
                    std::pair<int,int> search = search_Buffer(address, params);
                    if(search.first == -1){
                        init_Buffer(address, params);
                        result[index+2].traffic++;
                        // Writing to L2 cache
                        bool full=true;
                        for(int i =0; i < ASSOC ; i++){
                            if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                                cache[P.index][i].tag=P.tag;
                                cache[P.index][i].valid=true;
                                cache[P.index][i].dirty=true;
                                cache[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                full=false;
                                break;
                            }
                        }
                        if(full){
                            for (int i = 0; i < ASSOC; i++) {
                                if (cache[P.index][i].lru == (ASSOC-1)) {
                                    if(cache[P.index][i].dirty == 1){
                                        result[index+1].writeback++;
                                        result[index+2].traffic++;
                                    }
                                    cache[P.index][i].tag=P.tag;
                                    cache[P.index][i].valid=true;
                                    cache[P.index][i].dirty=true;
                                    cache[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    break;
                                }
                            }
                        }
                    }
                    
                    else if(search.first != -1){
                        result[index].writemiss--;
                        // result[index].write--;
                        // result[index].prefetch++;
                        updateStreamLRU(search.first, params);
                        manage_Buffer(address,search,params);
                        bool full=true;
                        for(int i =0; i < ASSOC ; i++){
                            if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                                cache[P.index][i].tag=P.tag;
                                cache[P.index][i].valid=true;
                                cache[P.index][i].dirty=true;
                                cache[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                full=false;
                                break;
                            }
                        }
                        if(full){
                            for (int i = 0; i < ASSOC; i++) {
                                if (cache[P.index][i].lru == (ASSOC-1)) {
                                    if(cache[P.index][i].dirty == 1){
                                        result[index+1].writeback++;
                                        result[index+2].traffic++;
                                    }
                                    cache[P.index][i].tag=P.tag;
                                    cache[P.index][i].valid=true;
                                    cache[P.index][i].dirty=true;
                                    cache[P.index][i].address=address;
                                    updateLRU(index, P, i, true , params);
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            else if(index == 1){
                if(params.PREF_N!=0){
                std::pair<int,int> search = search_Buffer(address, params);
                if(search.first == -1){
                    init_Buffer(address, params);
                    result[index+1].traffic++;
                    // Writing to L2 cache
                    bool full=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=true;
                            cache2[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            full=false;
                            break;
                        }
                    }
                    if(full){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    result[index+1].writeback++;
                                    result[index+1].traffic++;
                                }
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=true;
                                cache2[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                }
                
                else if(search.first != -1){
                    result[index].writemiss--;
                    // result[index].write--;
                    // result[index].prefetch++;
                    updateStreamLRU(search.first, params);
                    manage_Buffer(address,search,params);
                    bool full=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=true;
                            cache2[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            full=false;
                            break;
                        }
                    }
                    if(full){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    result[index+1].writeback++;
                                    result[index+1].traffic++;
                                }
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=true;
                                cache2[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                }
                }
                else{
                    result[index+1].traffic++;
                    // Writing to L2 cache
                    bool full=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=true;
                            cache2[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            full=false;
                            break;
                        }
                    }
                    if(full){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    result[index+1].writeback++;
                                    result[index+1].traffic++;
                                }
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=true;
                                cache2[P.index][i].address=address;
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                }
            }
        } 
    }
}

// Starting the simulator
void Start(CacheParams &params){
    std::ifstream file(params.trace_file.c_str()); 
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << params.trace_file << std::endl;
    }

    std::string line;
    while (getline(file, line)) {
        char rw;
        int addr;
        std::istringstream iss(line);
        if (iss >> rw >> std::hex >> addr) {
            if (rw == 'r') {
                readCache(cache, 0, rw, addr, params.L1_SIZE, params.L1_ASSOC, params.BLOCKSIZE, params);
            } 
            else if(rw == 'w'){
                writeCache(cache, 0, rw, addr, params.L1_SIZE, params.L1_ASSOC, params.BLOCKSIZE, params);
            }
            else {
                std::cerr << "Error: Unknown request type " << rw << std::endl;
            }
        } 
        else {
            std::cerr << "Error: Invalid line format in the trace file" << std::endl;
        }
    }
}

// Printing the stream buffer
void printBuffer(CacheParams &params){
    for (int i = 0; i < params.PREF_N; i++) {
        std::cout<<"Buffer number: "<<i+1<<" | valid: "<<streamBuffer[i]->valid<<" | ";
        for (int j = 0; j < params.PREF_M; j++) {
            std::cout<<" "<<std::hex<<streamBuffer[i][j].address<<", ";
        }
        std::cout<<std::endl;
    }
}

void simulationConfig(CacheParams &params){
    std::cout << "===== Simulator configuration =====" << std::endl;
    std::cout << "BLOCKSIZE:  " << std::dec<< params.BLOCKSIZE << std::endl;
    std::cout << "L1_SIZE:    " << params.L1_SIZE << std::endl;
    std::cout << "L1_ASSOC:   " << params.L1_ASSOC << std::endl;
    std::cout << "L2_SIZE:    " << params.L2_SIZE << std::endl;
    std::cout << "L2_ASSOC:   " << params.L2_ASSOC << std::endl;
    std::cout << "PREF_N:     " << params.PREF_N << std::endl;
    std::cout << "PREF_M:     " << params.PREF_M << std::endl;
    std::cout << "trace_file: " << params.trace_file << std::endl;

}

void printCache1(CacheParams &params) {
    const int SETS_L1 = params.L1_SIZE / ((params.L1_ASSOC * params.BLOCKSIZE)==0? 1: (params.L1_ASSOC * params.BLOCKSIZE));
    const int SETS_L2 = params.L2_SIZE / ((params.L2_ASSOC * params.BLOCKSIZE)==0? 1: (params.L2_ASSOC * params.BLOCKSIZE));

    std::cout << "===== L1 contents =====" << std::endl;
    for (int j = 0; j < SETS_L1; j++) {
        std::cout << "set " << std::setw(6) <<std::dec<< j << ":";
        for (int lru = 0; lru < params.L1_ASSOC; lru++) {
            for (int i = 0; i < params.L1_ASSOC; i++) {
                if (cache[j][i].lru == lru) {
                    std::cout << "   "  << std::setw(6) << std::hex << cache[j][i].tag << " " << (cache[j][i].dirty ? "D" : " ") << " ";
                }
            }
        }
        std::cout << std::endl;
    }
    if(params.L2_SIZE!=0){
        std::cout << '\n'; 
        std::cout << "===== L2 contents =====" << std::endl;
        for (int j = 0; j < SETS_L2; j++) {
            std::cout << "set " << std::setw(6) <<std::dec << j << ":";
            for (int lru = 0; lru < params.L2_ASSOC; lru++) {
                for (int i = 0; i < params.L2_ASSOC; i++) {
                    if (cache2[j][i].lru == lru) {
                        std::cout << "   " << std::setw(6) << std::hex  << cache2[j][i].tag << " " << (cache2[j][i].dirty ? "D" : " ") << " ";
                    }
                }
            }
            std::cout << std::endl;
        }
    }
}

// The main function
int main(int argc, char* argv[]) {
    if (argc != 9) {
        std::cerr << "Error: Expected 8 command-line arguments but received " << (argc - 1) << std::endl;
        return EXIT_FAILURE;
    }
    
    CacheParams params;
    params.BLOCKSIZE = atoi(argv[1]);
    params.L1_SIZE = atoi(argv[2]);
    params.L1_ASSOC = atoi(argv[3]);
    params.L2_SIZE = atoi(argv[4]);
    params.L2_ASSOC = atoi(argv[5]);
    params.PREF_N = atoi(argv[6]);
    params.PREF_M = atoi(argv[7]);
    params.trace_file = argv[8];
    std::ifstream file(params.trace_file.c_str());  
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << params.trace_file << std::endl;
        return EXIT_FAILURE;
    }
    
    //Alloting memory to cache
    allot_mem(params);
    const int SETS_L1 = params.L1_SIZE / ((params.L1_ASSOC * params.BLOCKSIZE)==0? 1: (params.L1_ASSOC * params.BLOCKSIZE));
    const int SETS_L2 = params.L2_SIZE / ((params.L2_ASSOC * params.BLOCKSIZE)==0? 1: (params.L2_ASSOC * params.BLOCKSIZE));

    //Start the simulation
    Start(params);
    simulationConfig(params);
    std::cout << '\n'; 
    printCache1(params);
    if(params.PREF_N!=0){
        printBuffer(params);
    }
    // //Printing results
    Print_results();

    //Deleting alloted memory
    cleanup(SETS_L1, SETS_L2, params);
    return 0;
}