//Header files:
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <cstdint> 

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
}result[3];

// Storing the cache specifications
struct CacheParams{
    uint32_t BLOCKSIZE;
    uint32_t L1_SIZE;
    uint32_t L1_ASSOC;
    uint32_t L2_SIZE;
    uint32_t L2_ASSOC;
    uint32_t PREF_N;
    uint32_t PREF_M;
    std::string trace_file;
};

// Storing the values to be stored in each cache blog
struct Cache_flags {
    bool valid;
    bool dirty;
    int lru;
    uint32_t tag;
    uint32_t address;
};

struct Stream_flags{
    bool valid;
    uint32_t address;
    uint32_t tag;
    int lru;
};

// Storing the translated addresses for L1 & L2 cache
struct para{
    uint32_t offset;
    uint32_t index;
    uint32_t tag;
    int SETS;
};

// Declaring the L1 and L2 cache flags
Cache_flags** cache;
Cache_flags** cache2;
Stream_flags** streamBuffer;

// Alloting memory to L1 and L2 cache
void allot_mem(CacheParams &params){
    // L1 cache allotment
    const int SETS_L1 = params.L1_SIZE / (params.L1_ASSOC * params.BLOCKSIZE);
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

    // L2 cache allotment
    const int SETS_L2 = params.L2_SIZE / (params.L2_ASSOC * params.BLOCKSIZE);
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

// Cleaning up memory alloted to L1 and L2 cache
void cleanup(int SETS_L1, int SETS_L2, CacheParams &params){
    for (int i = 0; i < SETS_L1; i++) {
        delete[] cache[i];
    }
    delete[] cache;

    for (int i = 0; i < SETS_L2; i++) {
        delete[] cache2[i];
    }
    delete[] cache2;

    for(int i = 0; i< params.PREF_N; i++){
        delete[] streamBuffer[i];
    }
    delete[] streamBuffer;

}

// Printing the results 
void Print_results(){
    std::cout << "\n===== Measurements =====\n";
    /****************L1*********************/
    std::cout << "a. L1 reads:"
        << "\t\t\t" <<std::dec <<result[0].read << "\n";
    std::cout << "b. L1 read misses:"
        << "\t\t" << result[0].readmiss << "\n";
    std::cout << "c. L1 writes:"
        << "\t\t\t" << result[0].write<< "\n";
    std::cout << "d. L1 write misses:"
        << "\t\t" << result[0].writemiss << "\n";
    std::cout << "e. L1 miss rate:"
        << "\t\t" << std::fixed << std::setprecision(4) << static_cast<float>(result[0].readmiss + result[0].writemiss) / static_cast<float>(result[0].read + result[0].write) << "\n";
    std::cout << "f. L1 writebacks:"
        << "\t\t" << result[1].writeback << "\n";
    std::cout << "g. L1 prefetches:"
        << "\t\t" << result[0].prefetch << "\n";
    /****************L2*********************/
    std::cout << "h. L2 reads (demand):"
        << "\t\t" << result[1].read << "\n";
    std::cout << "i. L2 read misses (demand):"
        << "\t" << result[1].readmiss << "\n";
    std::cout << "j. L2 reads (prefetch):"
        << "\t\t" << "\n";
    std::cout << "k. L2 read misses (prefetch):"
        << "\t\t" << "\n";
    std::cout << "l. L2 writes:"
        << "\t\t\t" << result[1].write<< "\n";
    std::cout << "m. L2 write misses:"
        << "\t\t" << result[1].writemiss << "\n";
    std::cout << "n. L2 miss rate:"
        << "\t\t" << std::fixed << std::setprecision(4) << static_cast<float>(result[1].readmiss + result[1].writemiss) / static_cast<float>(result[1].read + result[1].write) << "\n";

    /**************MEMORY*********************/
    std::cout << "o. L2 writebacks:"
        << "\t\t" << result[2].writeback << "\n";
    std::cout << "p. L2 prefetches:"
        << "\t" << result[1].prefetch << "\n";
    std::cout << "q. memory traffic:"
        << "\t\t" << result[2].traffic << "\n";
}

void updateMRU(int stream_num, CacheParams &params){
    int current_mru;
    current_mru=streamBuffer[stream_num]->lru;
    for(int i = 0; i < params.PREF_N;i++){
        if(streamBuffer[i]->lru<current_mru){
            streamBuffer[i]->lru++;
        }
    }
    streamBuffer[stream_num]->lru=0;
}

//Searching inside the stream buffers
int search_Buffer(uint32_t address, CacheParams &params){
    int found=-1;
    int mru=std::numeric_limits<int>::max();
    for(int i =0 ; i < params.PREF_N ;i++){
        if(streamBuffer[i]->valid == 0)
            continue;
        for(int j=0; j< params.PREF_M;j++){
            if(streamBuffer[i][j].address == address && streamBuffer[i]->lru < mru){
                found=i;
                mru=streamBuffer[i]->lru;
            }       
        }
    }

    return found;
}

// Initialising the buffer
void init_Buffer(uint32_t address,int lru, CacheParams &params ){
    if(lru == -1){
        for(int i =0 ; i < params.PREF_N ;i++){
            if(streamBuffer[i]->lru == 0){
                streamBuffer[i]->valid=true;
                for(int j=0; j< params.PREF_M; j++){
                    streamBuffer[i][j].address=address+j+1;
                    result[2].prefetch++;
                }
                break;
            }
        }
    }
    else{
        for(int i=0;i<params.PREF_M;i++){
            streamBuffer[lru][i].address=address+i+1;
        }
    }
}

// Getting and storing the translated address for the cache
para getPara(uint32_t address, uint32_t SIZE, uint32_t ASSOC, uint32_t BLOCKSIZE){
    para temp;
    // Calculate cache parameters
    const int SETS = SIZE / (ASSOC * BLOCKSIZE);
    const int INDEX = log2(SETS);
    const int OFFSET = log2(BLOCKSIZE);

    // Extract tag, index, and offset from the address
    temp.offset = address & ((1 << OFFSET) - 1);
    temp.index = (address >> OFFSET) & ((1 << INDEX) - 1);
    temp.tag = address >> (OFFSET + INDEX);
    temp.SETS=SETS;
    return temp;
}

// Printing the addresses and translated address in the input file
void Addresses(const CacheParams& params, int SETS){
    // Opening the file
    std::ifstream file(params.trace_file);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << params.trace_file << std::endl;
    }

    std::string line;
    while (getline(file, line)) {
        char rw;
        uint32_t addr;
        std::istringstream iss(line);
        //Reading the address from the file
        if (iss >> rw >> std::hex >> addr) {
            if (rw == 'r' || rw == 'w') {
                std::cout << "Operation: " << rw << std::endl;
                //Translating the addresses
                const int INDEX = log2(SETS);
                const int OFFSET = log2(params.BLOCKSIZE);
                uint32_t offset = addr & ((1 << OFFSET) - 1);
                uint32_t index = (addr >> OFFSET) & ((1 << INDEX) - 1);
                uint32_t tag = addr >> (OFFSET + INDEX);
                // Printing the addresses
                std::cout << "Address: " << std::hex << addr << std::dec << std::endl;
                std::cout << "Tag: " << std::hex << tag << std::endl;
                std::cout << "Index: " << std::hex <<index << std::endl;
                std::cout << "Offset: " << std::dec << offset << std::endl;
            } else {
                std::cerr << "Error: Unknown request type " << rw << std::endl;
            }
        } else {
            std::cerr << "Error: Invalid line format in the trace file" << std::endl;
        }
    }
    std::cout<<"\n"<<std::endl;
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
                if(cache[P.index][i].lru != maximum){
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
                if(cache2[P.index][i].lru != maximum){
                    cache2[P.index][i].lru++;
                }
            }
            cache2[P.index][max_index].lru=0;
        }
    }
}

// Function to handle write backs from L1 cache to Memory
void writeBack(Cache_flags** cache, int index, uint32_t address, uint32_t SIZE, uint32_t ASSOC, uint32_t BLOCKSIZE, CacheParams &params){
    // Getting translated addresses
    para P=getPara(address, SIZE, ASSOC, BLOCKSIZE);
    bool hit = false;
    // Searching for the tag in the cache
    for (int i = 0; i < ASSOC; i++) {
        // Case 1: tag found and dirty bit is 1
        if(cache2[P.index][i].valid && cache2[P.index][i].tag == P.tag && cache2[P.index][i].dirty){
            std::cout<<"WriteBack hit-dirty=1"<<std::endl;
            hit=true;
            cache2[P.index][i].dirty = false;
            result[index].write++;
            std::cout<<"Write Back, L2.write ++"<<std::endl;
            result[index].writeback++;// L1 to L2
            result[index+1].writeback++;//L2 to MEM
            result[index+1].traffic++;
            updateLRU(index, P, i, hit, params);
            break;
        }
        // Case 2: tag found and dirty bit is 0
        else if (cache2[P.index][i].valid && cache2[P.index][i].tag == P.tag && !cache2[P.index][i].dirty) {
            std::cout<<"WriteBack hit-dirty=0 at "<<P.index << ", "<<i <<std::endl;
            hit = true;
            cache2[P.index][i].dirty = true;
            result[index].write++;
            result[index].writeback++;
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
            bool empty=true;
            for(int i =0; i < ASSOC ; i++){
                if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                    cache2[P.index][i].tag=P.tag;
                    cache2[P.index][i].valid=true;
                    cache2[P.index][i].dirty=true;
                    cache2[P.index][i].address=address;
                    updateLRU(index, P, i, true , params);
                    empty=false;
                }
            }
            if(empty){
                for (int i = 0; i < ASSOC; i++) {
                    if (cache2[P.index][i].lru == (ASSOC-1)) {
                        if(cache2[P.index][i].dirty == 1){
                            // result[index+1].writeback++;
                            // result[index+1].traffic++;
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
void readCache(Cache_flags** cache_R, int index, char rw, uint32_t address, uint32_t SIZE, uint32_t ASSOC, uint32_t BLOCKSIZE, CacheParams &params){
    
    // Getting translated addresses  
    para P=getPara(address, SIZE, ASSOC, BLOCKSIZE);
    bool hit = false;
    // Search for the tag in the cache
    for (int i = 0; i < ASSOC; i++) {
        // Case 1: tag found 
        // if (cache_R[P.index][i].valid && cache_R[P.index][i].tag == P.tag && !cache_R[P.index][i].dirty) {
            if (cache_R[P.index][i].valid && cache_R[P.index][i].tag == P.tag) {
            hit = true;
            if (rw == 'r') {
                result[index].read++;
            }
            updateLRU(index, P, i, hit, params);
            break;
        }
        // Case 2: tag found and dirty bit is 1
        // else if(cache_R[P.index][i].valid && cache_R[P.index][i].tag == P.tag && cache_R[P.index][i].dirty){
        //     hit = true;
        //     result[index].read++;
        //     updateLRU(index, P, i, hit, params);
        //     if(index==1){
        //         cache2[P.index][i].dirty=false;
        //         // result[2].writeback++;
        //     }
        //     if(index == 0){
        //         cache[P.index][i].dirty=false;
        //         std::cout<<"WriteBack called"<<std::endl;
        //         writeBack(cache2, index+1, address, params.L2_SIZE, params.L2_ASSOC, BLOCKSIZE, params);
        //     }
        //     break;
        // }
    }

    if(hit){
        int search=search_Buffer(address, params);
        if(search != -1){
            std::cout<<"Initing Stream buffer in readcache"<<std::endl;
            init_Buffer(address, search, params);
        }
    }

    // If tag is not found in cache
    if (!hit) {
        if (rw == 'r') {
            result[index].read++;
            result[index].readmiss++;
            std::cout<<"Read Missed on: "<<std::hex<<address<<" at L"<<index+1<<std::endl;
            if(index == 0){// If this is L1
                // Sending read command to L2
                readCache(cache2, 1, 'r', address, params.L2_SIZE, params.L2_ASSOC, params.BLOCKSIZE, params);
                // Writing the returned value to L1
                bool empty=true;
                for(int i =0; i < ASSOC ; i++){
                    if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                        cache[P.index][i].tag=P.tag;
                        cache[P.index][i].valid=true;
                        cache[P.index][i].dirty=false;
                        cache[P.index][i].address=address;
                        std::cout<<"Written to L1 on: "<<std::hex<<address<<std::endl;
                        updateLRU(index, P, i, true , params);
                        empty=false;
                        break;
                    }
                }
                if(empty){
                    for (int i = 0; i < ASSOC; i++) {
                        if (cache[P.index][i].lru == (ASSOC-1)) {
                            if(cache[P.index][i].dirty == 1){
                            writeBack(cache2, index+1, cache[P.index][i].address, params.L2_SIZE, params.L2_ASSOC, BLOCKSIZE, params);
                            }
                            cache[P.index][i].tag=P.tag;
                            cache[P.index][i].valid=true;
                            cache[P.index][i].dirty=false;
                            cache[P.index][i].address=address; 
                            std::cout<<"Written to L1 on: "<<std::hex<<address<<std::endl;
                            updateLRU(index, P, i, true , params);
                            break;
                        }
                    }
                }
            }
            if(index == 1){// If this is L2
                // getting data from memory
                std::cout<<"Searching address "<<std::dec<<address<<std::endl;
                int search = search_Buffer(address, params);
                if(search == -1){
                    init_Buffer(address,search,params);
                    result[index+1].traffic++;
                    // Writing to L2 cache
                    bool empty=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=false;
                            cache2[P.index][i].address=address;
                            std::cout<<"Written to L2 on empty cache: "<<std::hex<<address<<std::endl;
                            updateLRU(index, P, i, true , params);
                            empty=false;
                            break;
                        }
                    }
                    if(empty){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    // result[index+1].writeback++;
                                    // result[index+1].traffic++;
                                }
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=false;
                                cache2[P.index][i].address=address;
                                std::cout<<"Written to L2 on full cache: "<<std::hex<<address<<std::endl;
                                updateLRU(index, P, i, true , params);
                                break;
                            }
                        }
                    }
                }
                
                else if(search != -1){
                    std::cout<<"Hit in stream buffer number "<<search+1<<std::endl;
                    result[index].readmiss--;
                    init_Buffer(address,search,params);
                    bool empty=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=false;
                            cache2[P.index][i].address=address;
                            std::cout<<"Written to L2 on empty cache: "<<std::hex<<address<<std::endl;
                            updateLRU(index, P, i, true , params);
                            empty=false;
                            break;
                        }
                    }
                    if(empty){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    // result[index+1].writeback++;
                                    // result[index+1].traffic++;
                                }
                                cache2[P.index][i].tag=P.tag;
                                cache2[P.index][i].valid=true;
                                cache2[P.index][i].dirty=false;
                                cache2[P.index][i].address=address;
                                std::cout<<"Written to L2 on full cache: "<<std::hex<<address<<std::endl;
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
void writeCache(Cache_flags** cache_R, int index, char rw, uint32_t address, uint32_t SIZE, uint32_t ASSOC, uint32_t BLOCKSIZE, CacheParams &params){
    
    para P=getPara(address, SIZE, ASSOC, BLOCKSIZE);
    bool hit = false;
    // Search for the address in the L1 cache
    for (int i = 0; i < ASSOC; i++) {
        // Check if the L1 cache entry is valid and the tag matches
        // if (cache_R[P.index][i].valid && cache_R[P.index][i].tag == P.tag && !cache_R[P.index][i].dirty) {
            if (cache_R[P.index][i].valid && cache_R[P.index][i].tag == P.tag) {
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
        // if(cache_R[P.index][i].valid && cache_R[P.index][i].tag == P.tag && cache_R[P.index][i].dirty){
        //     hit = true;
        //     result[index].write++;
        //     updateLRU(index, P, i, hit, params);
        //     if(index ==1)
        //         cache2[P.index][i].dirty = false;
        //     if(index==0){
        //         cache[P.index][i].dirty = false;
        //         std::cout<<"WriteBack called"<<std::endl;
        //         writeBack(cache2, 1, address, params.L2_SIZE, params.L2_ASSOC, BLOCKSIZE, params);
        //     }
        //     break;
        // }
    }

    if(hit){
        int search=search_Buffer(address, params);
        if(search != -1){
            std::cout<<"Initing Stream buffer in writecache"<<std::endl;
            init_Buffer(address, search, params);
        }
    }

    if (!hit) {
        if (rw == 'w') {
            result[index].write++;
            result[index].writemiss++;
            std::cout<<"Write Missed on: "<<std::hex<<address<<" at L"<<index+1<<std::endl;
            if(index == 0){// If this is L1
                // Search in L2 cache
                readCache(cache2, 1, 'r', address, params.L2_SIZE, params.L2_ASSOC, params.BLOCKSIZE, params);
                //Writing returned tag to L1
                bool empty=true;
                for(int i =0; i < ASSOC ; i++){
                    if(cache[P.index][i].valid == 0 && cache[P.index][i].dirty == 0){
                        cache[P.index][i].tag=P.tag;
                        cache[P.index][i].valid=true;
                        cache[P.index][i].dirty=true;
                        cache[P.index][i].address=address;
                        updateLRU(index, P, i, true , params);
                        std::cout<<"Written to L1 on when empty: "<<std::hex<<address<<std::endl;
                        empty=false;
                        break;
                    }
                }
                if(empty){
                    for (int i = 0; i < ASSOC; i++) {
                        if (cache[P.index][i].lru == (ASSOC-1)) {
                            if(cache[P.index][i].dirty == 1){
                                // writeBack(cache2, index+1, cache[P.index][i].address, params.L2_SIZE, params.L2_ASSOC, BLOCKSIZE, params);
                            }
                            cache[P.index][i].tag=P.tag;
                            cache[P.index][i].valid=true;
                            cache[P.index][i].dirty=true;
                            cache[P.index][i].address=address;
                            std::cout<<"Written to L1 on when full: "<<std::hex<<address<<std::endl;
                            updateLRU(index, P, i, true , params);
                            break;
                        }
                    }
                }
            }
            else if(index == 1){
                std::cout<<"Searching address "<<std::dec<<address<<std::endl;
                int search = search_Buffer(address, params);
                if(search == -1){
                    result[index+1].traffic++;
                    // Writing to L2 cache
                    bool empty=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=true;
                            cache2[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            empty=false;
                            break;
                        }
                    }
                    if(empty){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    // result[index+1].writeback++;
                                    // result[index+1].traffic++;
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
                else if(search != -1){
                    std::cout<<"Hit in stream buffer number "<<search+1<<std::endl;
                    result[index].readmiss--;
                    init_Buffer(address,search,params);
                    bool empty=true;
                    for(int i =0; i < ASSOC ; i++){
                        if(cache2[P.index][i].valid == 0 && cache2[P.index][i].dirty == 0){
                            cache2[P.index][i].tag=P.tag;
                            cache2[P.index][i].valid=true;
                            cache2[P.index][i].dirty=true;
                            cache2[P.index][i].address=address;
                            updateLRU(index, P, i, true , params);
                            empty=false;
                            break;
                        }
                    }
                    if(empty){
                        for (int i = 0; i < ASSOC; i++) {
                            if (cache2[P.index][i].lru == (ASSOC-1)) {
                                if(cache2[P.index][i].dirty == 1){
                                    // result[index+1].writeback++;
                                    // result[index+1].traffic++;
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
    std::ifstream file(params.trace_file);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << params.trace_file << std::endl;
    }

    std::string line;
    while (getline(file, line)) {
        char rw;
        uint32_t addr;
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
        //19094
        //19348
        //19604
        //19344
        //64151
        if(addr == 64151)
            break;
    }
}

// Printing the stream buffer
void printBuffer(CacheParams &params){
    for (int i = 0; i < params.PREF_N; i++) {
        std::cout<<"Buffer number: "<<i+1<<" | valid: "<<streamBuffer[i]->valid<<" | ";
        for (int j = 0; j < params.PREF_M; j++) {
            std::cout<<" "<<std::dec<<streamBuffer[i][j].address<<", ";
        }
        std::cout<<std::endl;
    }
}

// Printing the cache state
// #include <iostream>
// #include <iomanip>
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
    const int SETS_L1 = params.L1_SIZE / (params.L1_ASSOC * params.BLOCKSIZE);
    const int SETS_L2 = params.L2_SIZE / (params.L2_ASSOC * params.BLOCKSIZE);

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
    std::cout << '\n'; 
    std::cout << "===== L2 contents =====" << std::endl;
    for (int j = 0; j < SETS_L2; j++) {
        std::cout << "set " << std::setw(6) <<std::dec << j << ":";
        for (int lru = 0; lru < params.L2_ASSOC; lru++) {
            for (int i = 0; i < params.L2_ASSOC; i++) {
                if (cache2[j][i].lru == lru) {
                    std::cout << "   " << std::hex << std::setw(6) << cache2[j][i].tag << " " << (cache2[j][i].dirty ? "D" : " ") << " ";
                }
            }
        }
        std::cout << std::endl;
    }
}

void printCacheVerify(CacheParams &params) {
    const int SETS_L1 = params.L1_SIZE / (params.L1_ASSOC * params.BLOCKSIZE);
    const int SETS_L2 = params.L2_SIZE / (params.L2_ASSOC * params.BLOCKSIZE);
    std::cout<<"L1 CACHE"<<std::endl;
    for(int j=0;j< SETS_L1;j++){ 
        for (int i = 0; i < params.L1_ASSOC; i++) {
            if(cache[j][i].tag != 0)
                std::cout<< std::hex <<j<<"!!!!TAG: "<<cache[j][i].tag<<" VALID: "<<cache[j][i].valid<<" DIRTY: "<<cache[j][i].dirty<<" LRU: "<<cache[j][i].lru<<std::endl;
            // else
                // std::cout<<"TAG: "<<cache[j][i].tag<<" VALID: "<<cache[j][i].valid<<" DIRTY: "<<cache[j][i].dirty<<" LRU: "<<cache[j][i].lru<<std::endl;

        }
    }
    std::cout<<"L2 CACHE"<<std::endl;
    for(int j=0;j< SETS_L2;j++){ 
        for (int i = 0; i < params.L2_ASSOC; i++) {
            if(cache2[j][i].tag != 0)
                std::cout<<j<<"!!!!TAG: "<<cache2[j][i].tag<<" VALID: "<<cache2[j][i].valid<<" DIRTY: "<<cache2[j][i].dirty<<" LRU: "<<cache2[j][i].lru<<std::endl;
            // else
                // std::cout<<"TAG: "<<cache2[j][i].tag<<" VALID: "<<cache2[j][i].valid<<" DIRTY: "<<cache2[j][i].dirty<<" LRU: "<<cache2[j][i].lru<<std::endl;
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

    std::ifstream file(params.trace_file);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << params.trace_file << std::endl;
        return EXIT_FAILURE;
    }
    
    //Alloting memory to cache
    allot_mem(params);
    // init_Buffer(19094, 0,params);
    // init_Buffer(25690, 1,params);
    // init_Buffer(31000, 2,params);
    // int s1=search_Buffer(19100,params);
    // int s2=search_Buffer(65000,params);
    // int s3=search_Buffer(25699,params);
    // std::cout<<"19100 address found in buffer number "<<s1<<std::endl;
    // std::cout<<"65000 address found in buffer number "<<s2<<std::endl;
    // std::cout<<"25699 address found in buffer number "<<s3<<std::endl;
    const int SETS_L1 = params.L1_SIZE / (params.L1_ASSOC * params.BLOCKSIZE);
    const int SETS_L2 = params.L2_SIZE / (params.L2_ASSOC * params.BLOCKSIZE);
    std::cout<<"SETS L1: "<< SETS_L1<<" SETS L2: "<<SETS_L2<<std::endl;
    //Printing the address for L1 and L2
    std::cout<<"L1:................."<<std::endl;
    // Addresses(params, SETS_L1); 
    std::cout<<"L2:................."<<std::endl;
    // Addresses(params, SETS_L2);


    //Start the simulation
    Start(params);
    
    simulationConfig(params);
    std::cout << '\n'; 
    printCache1(params);
    printBuffer(params);
//     std::cout << '\n'; 
//    printCacheVerify(params); 
    // //Printing results
    std::cout << '\n'; 
    Print_results();

    //Deleting alloted memory
    cleanup(SETS_L1, SETS_L2, params);
    return 0;
}