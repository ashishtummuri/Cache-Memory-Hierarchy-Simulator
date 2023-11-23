#include<iostream>
#include<string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include<math.h>
#include<bitset>
#include "sim.h"
#include <bits/stdc++.h>

#include<sstream>
using namespace std;
/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/

struct cache
{	
	unsigned int valid;
	char dirty;
	unsigned int counter;
	string cache_tag;
	
};
cache** L1;//
cache** L2;

struct stream_buffer{
	int count;
	int sb_valid;
	int sb_tag;
};

stream_buffer** SB;
   unsigned int matched_hit=-1;
   unsigned int l1reads=0;
   unsigned int l1writes=0;
   unsigned int read_hit=0;
   unsigned int read_miss=0;
   unsigned int write_hit=0;
   unsigned int write_miss=0;
   
   unsigned int l2reads=0;
   unsigned int l2writes=0;
   unsigned int l2read_hit=0;
   unsigned int l2read_miss=0;
   unsigned int l2write_hit=0;
   unsigned int l2write_miss=0;
   
   unsigned int l1writebackl2_read=0;
   unsigned int l1writebackl2_write=0;
   unsigned int l1writebackmem=0;
   unsigned int l2writebackmem=0;
   //unsigned int memtraffic=0;
   //string newtag;
   
   unsigned int prefetch_hit=0;
   unsigned int prefetch_miss=0;  
   unsigned int L1_read = 0;
   unsigned int L1_write = 0;
   unsigned int L2_read = 0;
   unsigned int	L2_write = 0; 
   unsigned int L1_miss_buff_hit_read = 0;
   unsigned int L1_miss_buff_hit_write = 0;
   unsigned int L2_miss_buff_hit_read = 0;
   unsigned int L2_miss_buff_hit_write = 0;
   
    
   unsigned int binary_to_hex(string n)
      {
         string num = n;
         int dec_value = 0;
 
         // Initializing base value to 1, i.e 2^0
         int base = 1;
 
         int len = num.length();
         for (int i = len - 1; i >= 0; i--) {
            if (num[i] == '1')
            dec_value += base;
            base = base * 2;
         }
 
         return dec_value;
      }
   
void l2_write(string newtag,unsigned int l2size, unsigned int l2assoc, unsigned int l2blocksize,unsigned int L2_block_offset,unsigned int L2_index,unsigned int  L2_tagsize,unsigned int l2ways,unsigned int l2read_hit,int sb_number,int sb_memblock);
void l2_read(uint32_t addr, unsigned int l2size, unsigned int l2assoc, unsigned int l2blocksize,unsigned int L2_block_offset,unsigned int L2_index,unsigned int  L2_tagsize,unsigned int l2ways,unsigned int l2read_hit,int sb_number,int sb_memblock);

void l1prefetch(unsigned int prefetch_addr,unsigned int matched_hit, int sb_number, int sb_memblock, unsigned int L1_read, unsigned int L1_write, unsigned int L2_read, unsigned int L2_write);
bitset<32> baddress;
int main (int argc, char *argv[]) {
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

   // Exit with an error if the number of command-line arguments is incorrect.
   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];
	
	
	unsigned int l2assoc= params.L2_ASSOC;
	unsigned int l2size= params.L2_SIZE;
	unsigned int l2blocksize= params.BLOCKSIZE;
	
	
	//unsigned int
	
   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
  // printf("===================================\n");
	//unsigned int eviction=0;
	unsigned int L1_block_offset,L1_index, L1_tagsize;
	//unsigned int match_index;
	unsigned int sets = params.L1_SIZE/(params.L1_ASSOC*params.BLOCKSIZE);
	L1_block_offset = log2(params.BLOCKSIZE);
	L1_tagsize = log2(params.L1_SIZE)-log2(params.BLOCKSIZE)-log2(params.L1_ASSOC);
	L1_index = log2(params.L1_SIZE/(params.L1_ASSOC*params.BLOCKSIZE));
	unsigned int l1ways= params.L1_ASSOC;
	
	  int sb_number, sb_memblock;
		sb_number = params.PREF_N;
		sb_memblock = params.PREF_M;
		//stream_buffer SB[sb_number][sb_memblock];
	//cache L1[sets][l1ways];
	L1 = new cache*[sets];
	 // Initiliaze the array to the number of cache sets and assoc
   for(unsigned int i = 0; i < sets; i++)
   {
      L1[i] = new cache[l1ways];
      
   }
	
	unsigned int L2_block_offset,L2_index, L2_tagsize;	
	unsigned int l2sets=0; //= params.L2_SIZE/(params.L2_ASSOC*params.BLOCKSIZE);
	L2_block_offset = log2(params.BLOCKSIZE);
	//unsigned int L2_tagsize=0;
	//unsigned int L2_index=0;
	
	unsigned int l2ways= params.L2_ASSOC;
	
	//cache L2[l2sets][l2ways];
	
	
	 // Initiliaze the array to the number of cache sets and assoc
	if(params.L2_SIZE!=0)
	{l2sets = params.L2_SIZE/(params.L2_ASSOC*params.BLOCKSIZE);
	L2_index = log2(params.L2_SIZE/(params.L2_ASSOC*params.BLOCKSIZE));
	L2_tagsize = log2(params.L2_SIZE)-log2(params.BLOCKSIZE)-log2(params.L2_ASSOC);
	L2 = new cache*[l2sets];
   for(unsigned int i = 0; i < l2sets; i++)
   {	
      L2[i] = new cache[l2ways];
      
   }
   }
   else
   {
   	l2sets=0;
   }
   
   SB = new stream_buffer*[sb_number];
	 // Initiliaze the array to the number of cache sets and assoc
   for(unsigned int i = 0; i < sb_number; i++)
   {
      SB[i] = new stream_buffer[sb_memblock];
      
   }
   
	for(unsigned int i=0;i<sets;i++)
		{	for(unsigned int j=0;j<l1ways;j++)
			{ L1[i][j].valid = 0;	
			   L1[i][j].counter = j;
			  L1[i][j].dirty = '0';
			  L1[i][j].cache_tag = "-1";
			}
		}
		
	for(unsigned int i=0;i<l2sets;i++)
		{	for(unsigned int j=0;j<l2ways;j++)
			{ L2[i][j].valid = 0;	
			   L2[i][j].counter = j;
			  L2[i][j].dirty = '0';
			  L2[i][j].cache_tag = "-1";
			}
		}
		
		

		for(int i = 0; i < sb_number; i++){
		for(int j=0; j< sb_memblock; j++){
			SB[i][j].sb_tag  = -1;
			SB[i][j].sb_valid  = 0;
			SB[i][j].count = i;
	}
} 

	//for(i=0;i<sets;i++)
	//	{	for(j=0;j<l1ways;j++)
	//			printf("v%d c%d d %c t%s\n", L1[i][j].valid,L1[i][j].counter,L1[i][j].dirty,L1[i][j].cache_tag.c_str());
	//	}
	
   

   
   
   
   
   //bitset<32> l2baddress;
   // Read requests from the trace file and echo them back.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
      if (rw == 'r')
	  {  
	    // unsigned int eviction = 0;
	     l1reads++;
         //printf("r %x\n", addr);
	 	 baddress = bitset<32>(addr);
	 	 string bin_addr = baddress.to_string();
		 //printf( addr << endl;
		 //printf( typeid(addr).name() << endl;
		// printf( "Type of baddress : " << typeid(baddress).name() << endl;
		//printf("no. of index bits %d\n", L1_index);		 
		 //printf("%s\n",bin_addr.c_str());
		 string sblock = bin_addr.substr(32-L1_block_offset,L1_block_offset);
		 string indexx = bin_addr.substr(32-L1_block_offset-L1_index,L1_index);
		 string tag = bin_addr.substr(0,32-L1_block_offset-L1_index);
		 //printf("boffset %s\n",sblock.c_str());
		 //printf("indexx bits %s\n",indexx.c_str());
		 //printf("tag bits %s\n",tag.c_str());
		
		 stringstream ss(sblock);
		unsigned int block;
		ss>>block;//converting string to int
		//cout<< ss.str());
		stringstream ssindex(indexx);
		unsigned int index;
		ssindex>>index;
		
	
		index =  binary_integer(index);
		unsigned int count=0;
		unsigned int data_hit=0;
		//unsigned int L1_miss_buff_hit_read=0;
		//unsigned int match_index;
		for(unsigned int i=0;i<l1ways;i++)
		{	
			if((L1[index][i].cache_tag) == tag)
			{	
				//match_index=i;
				read_hit++;
				data_hit=1;
				matched_hit=data_hit;
				count = L1[index][i].counter;
				break; 
				
			}
		}
				
		if(data_hit==1)
		{	
			for(unsigned int i=0;i<l1ways;i++)
			{	
				if(L1[index][i].counter < count)//lru increment < lru items
					L1[index][i].counter++;				
				else if(L1[index][i].counter == count)
					L1[index][i].counter=0;	
			}		
			
		}
		if(data_hit==0)//not hit
		{	read_miss++;
		
				matched_hit =data_hit;
				for(unsigned int i=0; i< l1ways;i++)
				{
					if(L1[index][i].counter == l1ways-1)
					{ 	 
						if(L1[index][i].dirty=='1')
						{
							if(l2size!=0)
							{	string address= L1[index][i].cache_tag;
								string combindex= indexx;
								string newtag= address+combindex;
								//printf("newtag %s", newtag.c_str()); 
								l1writebackl2_read++;
								//eviction=1;
								//l2write(string newtag,);
								 l2_write(newtag, l2size,l2assoc,l2blocksize,L2_block_offset, L2_index,L2_tagsize, l2ways, l2read_hit,sb_number, sb_memblock);
								
							}
							else
							{
								//writebsck to mem
									l1writebackmem++;
							}
						}
						
						
		                 L1[index][i].cache_tag = tag; 
		                 L1[index][i].dirty = '0';
		                 L1[index][i].valid = 1;
		                 break;
					}		
				} 
				//not hit lru count
				for(unsigned int i=0;i<l1ways;i++)
				{	
					if(L1[index][i].counter < l1ways-1)//lru increment < lru items
						L1[index][i].counter++;				
					else 
						L1[index][i].counter=0;	
				}
			
				if(l2size!=0)
				{
					//l2read()
					l2_read( addr,  l2size,  l2assoc,  l2blocksize, L2_block_offset, L2_index,  L2_tagsize,l2ways,l2read_hit,sb_number, sb_memblock);
				}
			}//L1 miss 	 	
			
			if((params.PREF_N!=0) && (params.PREF_M!=0) && params.L2_SIZE==0 )
			{
				////prefetch
				string address= tag;
				string combindex= indexx;
				string newtag= address+combindex;
				stringstream ss(newtag);
				unsigned int prefetch_addr;
				//L1_miss_buff_hit_read = 1;
				L1_read = 1;
				L1_write = 0;
				L2_read = 0;
				L2_write = 0;
		        //ss>>prefetch_addr;
				prefetch_addr = binary_to_hex(newtag);
				//printf(" Prefetch_addr: %x\n", prefetch_addr);
				l1prefetch(prefetch_addr, matched_hit,sb_number,sb_memblock,L1_read,L1_write,L2_read,L2_write);
				
			}
	  }
	  
	
	  
	  
      else if (rw == 'w')
      { l1writes++; 
      	//printf("w_addrss %x\n",addr);
      	//printf("w %x\n", addr);
        
		//unsigned int eviction = 0;
		baddress = bitset<32>(addr);
		 //printf("no. of index bits %d\n", L1_index);
		 string bin_addr = baddress.to_string();		 
		 string sblock = bin_addr.substr(32-L1_block_offset,L1_block_offset);
		 string indexx = bin_addr.substr(32-L1_block_offset-L1_index,L1_index);
		 string tag = bin_addr.substr(0,32-L1_block_offset-L1_index);
		 //printf("write boffset %s\n",sblock.c_str());
		 //printf("write indexx bits %s\n",indexx.c_str());
		 //printf("write tag bits %s\n",tag.c_str());
		
		 stringstream ss(sblock);
		unsigned int block;
		ss>>block;//converting string to int
		//cout<< ss.str());
		stringstream ssindex(indexx);
		unsigned int index;
		ssindex>>index;
		index =  binary_integer(index);
		unsigned int count=0;
		unsigned int data_hit=0;
		unsigned int L1_miss_buff_hit_write=0;
		//unsigned int match_index;
		for(unsigned int i=0;i<l1ways;i++)
		{	
			if((L1[index][i].cache_tag) == tag)
			{	
				//match_index=i;
				write_hit++;
				data_hit=1;
				matched_hit=data_hit;
				count = L1[index][i].counter;
				L1[index][i].dirty='1';
				
				break; 
				
			}
		}
				
		if(data_hit==1)
		{	
			for(unsigned int i=0;i<l1ways;i++)
			{	
				if(L1[index][i].counter < count)//lru increment < lru items
					L1[index][i].counter++;				
				else if(L1[index][i].counter == count)
					L1[index][i].counter=0;	
			}
			
		}
		if(data_hit==0)
		{	write_miss++;
		    //l2(addr,)
		    
		    matched_hit= data_hit;
			for(unsigned int i=0; i< l1ways;i++)
			{	
			  
				if(L1[index][i].counter == l1ways-1)
				{ // Look for max counter
					 if(L1[index][i].dirty=='1')
					 {
					   
					 	 if(l2size!=0)
						{	string address= L1[index][i].cache_tag;
								string combindex= indexx;
								string newtag= address+combindex; 
								l1writebackl2_write++;
								
							//	cout<< "value"<<l1writebackl2_write);
								//eviction=1;
								//l2write(string newtag,);
								//printf("newtag in l2 before %s\n",newtag.c_str());
								l2_write(newtag, l2size,l2assoc,l2blocksize,L2_block_offset, L2_index,L2_tagsize, l2ways, l2read_hit,sb_number, sb_memblock);
								
						  }
							else
							{
								//writebsck to mem
								l1writebackmem++;
							}
					 	
					 	//l2write();
					 	//printf("newtag in req l2 %s\n",newtag.c_str());
					 	//l2_write(newtag, l2size,l2assoc,l2blocksize,L2_block_offset, L2_index,L2_tagsize, l2ways, l2read_hit ,eviction);
					 }
					 
					 
                     L1[index][i].cache_tag = tag; //update tag
                     L1[index][i].dirty = '1';
                     L1[index][i].valid = 1;
                     
                     
                     break;
				}		
			} 
			
			for(unsigned int i=0;i<l1ways;i++)
			{	
				if(L1[index][i].counter < l1ways-1)//lru increment < lru items
					L1[index][i].counter++;				
				else 
					L1[index][i].counter=0;	
			}	 	
			if(l2size!=0)
				{
					//l2read()
					l2_read( addr,  l2size,  l2assoc,  l2blocksize, L2_block_offset, L2_index,  L2_tagsize,l2ways,l2read_hit,sb_number, sb_memblock);
				} 
				
			
	  }
		if((params.PREF_N!=0) && (params.PREF_M!=0) && params.L2_SIZE==0 )
			{
				////prefetch
				string address= tag;
				string combindex= indexx;
				string newtag= address+combindex;
				//printf( "New_tag:" << newtag << endl;
				//stringstream ss(newtag);
				unsigned int prefetch_addr;
				L1_read = 0;
				L1_write = 1;
				L2_read = 0;
				L2_write = 0;
		        //ss>>prefetch_addr;
		        prefetch_addr = binary_to_hex(newtag);
		        //printf(" Prefetch_addr: %x\n", prefetch_addr);
		        //printf( "Prefetch_addr:" << prefetch_addr << endl;	 
		        
		        l1prefetch(prefetch_addr, matched_hit, sb_number,sb_memblock,L1_read,L1_write,L2_read,L2_write);
		        
				
			}
	  }
      else {
         printf("Error: Unknown request type %c.\n", rw);
	 exit(EXIT_FAILURE);
      }
	
      ///////////////////////////////////////////////////////
      // Issue the request to the L1 cache instance here.
      ///////////////////////////////////////////////////////	
    }
    unsigned int L1_print[sets][l1ways];
    string L1_sprint[sets][l1ways];
    for(unsigned int i=0;i<sets; i++)
    {
    	for(unsigned int j =0;j<l1ways;j++)
    	{
    		L1_sprint[i][j]=L1[i][j].cache_tag;
    		L1_print[i][j]= binary_to_hex(L1_sprint[i][j]);
    	}
    }
    
    	printf("\n");
printf("===== L1 contents =====\n");
      for(unsigned int k=0; k<sets; k++){
      printf("set     %d:", k);      
      for(unsigned int i =0; i < params.L1_ASSOC; i++){
      for(unsigned int j=0; j < params.L1_ASSOC; j++) {
          if(L1[k][j].counter == i){
              char q;
              if(L1[k][j].dirty == '1')
                  q = 'D';
              else
                  q = ' ';
              printf("   %x %c", L1_print[k][j], q);
          }
      }
}
      printf("\n");
}

if (params.L2_SIZE != 0) {
   unsigned int L2_print[l2sets][l2ways];
   string L2_sprint[l2sets][l2ways];
   for (unsigned int i=0; i<l2sets; i++){
      for (unsigned j=0; j<l2ways; j++){
         L2_sprint[i][j] = L2[i][j].cache_tag ;
         L2_print[i][j] = binary_to_hex(L2_sprint[i][j]) ;
      }
   }
      printf("\n");
printf("===== L2 contents =====\n");
      for(unsigned int k=0; k<l2sets; k++){
      printf("set      %d:", k);
      for(unsigned int i =0; i < params.L2_ASSOC; i++){
      for(unsigned int j=0; j < params.L2_ASSOC; j++) {
          if(L2[k][j].counter == i){
              char q;
              if(L2[k][j].dirty== '1')
                  q = 'D';
              else
                  q = ' ';
              printf("   %x %c", L2_print[k][j], q);
          }
      }
}
      printf("\n");
}
}
	if (params.PREF_M != 0 &&  params.PREF_N != 0) {
   unsigned int max_count = 0;
      printf("\n");
printf("===== Stream Buffer(s) contents =====\n");
      for(unsigned int k=0; k<sb_number; k++){
      for(unsigned int i =0; i < sb_number; i++){
         if(SB[i][0].count == max_count){
            max_count ++ ;
            printf(" ");
            for(unsigned int j=0; j < sb_memblock; j++) {
              printf("%x", SB[i][j].sb_tag);
              printf("  ");
            }
            printf("\n"); 
         }    
      }
    }     
}
    
    double L1_miss_rate = double(read_miss + write_miss)/double(l1reads + l1writes);
    double l2_miss_rate = double(l2read_miss)/double(read_miss+write_miss);  
      unsigned int L1_mem_traffic = read_miss + write_miss + l1writebackmem;
      unsigned int L2_mem_traffic = l2read_miss + l2write_miss + l2writebackmem ;
      if(params.PREF_N != 0 && params.PREF_M != 0 && params.L2_SIZE == 0)
      {
      read_miss = read_miss - L1_miss_buff_hit_read;
      write_miss = write_miss - L1_miss_buff_hit_write;
      L1_miss_rate = double(read_miss + write_miss)/double(l1reads + l1writes);
      L1_mem_traffic = read_miss + write_miss + l1writebackmem + prefetch_hit + prefetch_miss;
      
      }
      if(params.PREF_N != 0 && params.PREF_M != 0 && params.L2_SIZE != 0)
      {
      l2read_miss = l2read_miss - L2_miss_buff_hit_read;
      l2write_miss = l2write_miss - L2_miss_buff_hit_write;
      l2_miss_rate = double(l2read_miss)/double(read_miss+write_miss);
      L2_mem_traffic = l2read_miss + l2write_miss + l2writebackmem + prefetch_hit + prefetch_miss ;
      
      }
      
      if(params.L2_SIZE == 0  && params.L2_ASSOC == 0)//L1 -only
      {
      
         printf("\n");
      printf("===== Measurements =====\n");
      printf("a. L1 reads:                   %u\n",l1reads);
      printf("b. L1 read misses:             %u\n",read_miss);
      printf("c. L1 writes:                  %u\n",l1writes);
      printf("d. L1 write misses:            %u\n",write_miss);
      printf("e. L1 miss rate:               %.4f\n",L1_miss_rate);
      printf("f. L1 writebacks:              %u\n",l1writebackmem);
      printf("g. L1 prefetches:              %u\n",prefetch_hit+prefetch_miss);
      printf("h. L2 reads (demand):          0\n");
      printf("i. L2 read misses (demand):    0\n");
      printf("j. L2 reads (prefetch):        0\n");
      printf("k. L2 read misses (prefetch):  0\n");
      printf("l. L2 writes:                  0\n");
      printf("m. L2 write misses:            0\n");
      printf("n. L2 miss rate:               0.000\n");
      printf("o. L2 writebacks:              0\n");
      printf("p. L2 prefetches:              0\n");
      printf("q. memory traffic:             %u",L1_mem_traffic);
      }

      if(params.L2_SIZE != 0  && params.L2_ASSOC != 0)//L1_L2
      {
         printf("\n");
      printf("===== Measurements =====\n");
      printf("a. L1 reads:                   %u\n",l1reads);
      printf("b. L1 read misses:             %u\n",read_miss);
      printf("c. L1 writes:                  %u\n",l1writes);
      printf("d. L1 write misses:            %u\n",write_miss);
      printf("e. L1 miss rate:               %.4f\n",L1_miss_rate);
      printf("f. L1 writebacks:              %u\n",l1writebackl2_read + l1writebackl2_write);
      printf("g. L1 prefetches:              0\n");
      printf("h. L2 reads (demand):          %u\n",read_miss+write_miss);
      printf("i. L2 read misses (demand):    %u\n",l2read_miss);
      printf("j. L2 reads (prefetch):        0\n");
      printf("k. L2 read misses (prefetch):  0\n");
      printf("l. L2 writes:                  %u\n",l2writes);
      printf("m. L2 write misses:            %u\n",l2write_miss);
      printf("n. L2 miss rate:               %.4f\n",l2_miss_rate);
      printf("o. L2 writebacks:              %u\n",l2writebackmem);
      printf("p. L2 prefetches:              %u\n",prefetch_hit+prefetch_miss);
      printf("q. memory traffic:             %u\n",L2_mem_traffic);
      }
    
    return(0);
}
void l2_read(uint32_t addr, unsigned int l2size, unsigned int l2assoc, unsigned int l2blocksize,unsigned int L2_block_offset,unsigned int L2_index,unsigned int  L2_tagsize,unsigned int l2ways,unsigned int l2read_hit,int sb_number,int sb_memblock)
{
		 baddress = bitset<32>(addr);
	 	 string bin_addr = baddress.to_string();		 
		 string sblock = bin_addr.substr(32-L2_block_offset,L2_block_offset);
		 string l2indexx = bin_addr.substr(32-L2_block_offset-L2_index,L2_index);
		 string l2tag = bin_addr.substr(0,32-L2_block_offset-L2_index);
		 stringstream ss(sblock);
		 
		 unsigned int block;
		 ss>>block;		 
		 stringstream ssindex(l2indexx);
		 unsigned int l2index;
		 ssindex>>l2index;
		 l2index =  binary_integer(l2index);
		 
		unsigned int l2count=0;
		unsigned int l2data_hit=0;
		//unsigned int match_index;
		for(unsigned int i=0;i<l2ways;i++)
		{	
			if((L2[l2index][i].cache_tag) == l2tag)
			{	
				
				l2read_hit++;
				l2data_hit=1;
				matched_hit = l2data_hit;
				l2count = L2[l2index][i].counter;
				break;				
			}
			
		}
		if(l2data_hit==1)
		{	
			for(unsigned int i=0;i<l2ways;i++)
			{	
				if(L2[l2index][i].counter < l2count)//lru increment < lru items
					L2[l2index][i].counter++;				
				else if(L2[l2index][i].counter == l2count)
					L2[l2index][i].counter=0;	
			}		
			
		}
		
		if(l2data_hit==0)//not hit
		{	l2read_miss++;
		    matched_hit = l2data_hit;
			for(unsigned int i=0; i< l2ways;i++)
			{
				if(L2[l2index][i].counter == l2ways-1)
				{ 	 if(L2[l2index][i].dirty=='1')
					 {  
					 	l2writebackmem++;
					 }
					 
                     	L2[l2index][i].cache_tag = l2tag; 
                     	L2[l2index][i].dirty = '0';
                     	L2[l2index][i].valid = 1;
                }
            }
                     	                     
                   		for(unsigned int i=0;i<l2ways;i++)
						{	
							if(L2[l2index][i].counter < l2ways-1)//lru increment < lru items
								L2[l2index][i].counter++;				
							else 
								L2[l2index][i].counter=0;	
						}
		 }
                     
	   
		if((sb_number!=0) && (sb_memblock!=0) && l2size!=0 )
			{
				////prefetch
				string address= l2tag;
				string combindex= l2indexx;
				string newtag= address+combindex;
				//printf( "New_tag:" << newtag << endl;
				//stringstream ss(newtag);
				unsigned int prefetch_addr;
				L1_read = 0;
				L1_write = 0;
				L2_read = 1;
				L2_write = 0;
		        //ss>>prefetch_addr;
		        prefetch_addr = binary_to_hex(newtag);
		        //printf(" Prefetch_addr: %x\n", prefetch_addr);
		        //printf( "Prefetch_addr:" << prefetch_addr << endl;	 
		        
		        l1prefetch(prefetch_addr, matched_hit, sb_number,sb_memblock,L1_read,L1_write,L2_read,L2_write);
		        
				
			} 
		 
}


void l2_write(string newtag,unsigned int l2size, unsigned int l2assoc, unsigned int l2blocksize,unsigned int L2_block_offset,unsigned int L2_index,unsigned int  L2_tagsize,unsigned int l2ways,unsigned int l2read_hit,int sb_number, int sb_memblock )
{		string newaddress= newtag;
	l2writes++;
	 string bin_addr = newaddress;//.to_string();		 
		 string sblock = bin_addr.substr(32-L2_block_offset,L2_block_offset);
		 string l2indexx = bin_addr.substr(32-L2_block_offset-L2_index,L2_index);
		 string l2tag = bin_addr.substr(0,32-L2_block_offset-L2_index);
		 //printf("write boffset %s\n",sblock.c_str());
		 //printf("write indexx bits %s\n",indexx.c_str());
		// printf("write tag bits %s\n",l2tag.c_str());
		
		 stringstream ss(sblock);
		unsigned int block;
		ss>>block;//converting string to int
		//cout<< ss.str());
		stringstream ssindex(l2indexx);
		unsigned int l2index;
		ssindex>>l2index;
		l2index =  binary_integer(l2index);
		unsigned int l2count=0;
		unsigned int l2data_hit=0;
		unsigned int match_index;
							for(unsigned int i=0; i< l2ways;i++)
							{
								if((L2[l2index][i].cache_tag) == l2tag)
									{	
										//match_index=i;
										l2write_hit++;
										l2data_hit=1;
										matched_hit = l2data_hit;
										l2count = L2[l2index][i].counter;
										
										
												L2[l2index][i].dirty='1';
												
											
										break; 
										
									}
							}
							if(l2data_hit==1)
							{	
								for(unsigned int i=0;i<l2ways;i++)
								{	
									if(L2[l2index][i].counter < l2count)//lru increment < lru items
										L2[l2index][i].counter++;				
									else if(L2[l2index][i].counter == l2count)
										L2[l2index][i].counter=0;	
								}
								
							}
							
							if(l2data_hit==0)
							{	l2write_miss++;
							    matched_hit = l2data_hit;
								//l2(addr,)
								for(unsigned int i=0;i<l2ways;i++)
							{	
								if(L2[l2index][i].counter == l2ways-1)
								{ 	 if(L2[l2index][i].dirty=='1')
					   					{
					   						l2writebackmem++;
					   					}
					   					L2[l2index][i].dirty='1';
					   					 
					   					/*if(eviction==1)
					   					{
					   						L2[l2index][i].dirty='1';	
				   					}
					   					else
					   					{
					   						L2[l2index][i].dirty='0';
					   					}*/
					   			L2[l2index][i].cache_tag=l2tag;//miss when cahce is ful
					   			L2[l2index][i].valid=1;
					   			
					   					break;	//not sure		   						
					   			}	//miss							
							}
								
								
								for(unsigned int i=0;i<l2ways;i++)
								{	
									if(L2[l2index][i].counter < l2ways-1)//lru increment < lru items
										L2[l2index][i].counter++;				
									else 
										L2[l2index][i].counter=0;	
								}
							}	
							
							if((sb_number!=0) && (sb_memblock!=0) && l2size!=0 )
			{
				////prefetch
				string address= l2tag;
				string combindex= l2indexx;
				string newtag= address+combindex;
				//printf( "New_tag:" << newtag << endl;
				//stringstream ss(newtag);
				unsigned int prefetch_addr;
				L1_read = 0;
				L1_write = 0;
				L2_read = 0;
				L2_write = 1;
		        //ss>>prefetch_addr;
		        prefetch_addr = binary_to_hex(newtag);
		        //printf(" Prefetch_addr: %x\n", prefetch_addr);
		        //printf( "Prefetch_addr:" << prefetch_addr << endl;	 
		        
		        l1prefetch(prefetch_addr, matched_hit, sb_number,sb_memblock,L1_read,L1_write,L2_read,L2_write);
		        
				
			} 	
}

void l1prefetch(unsigned int prefetch_addr,unsigned int matched_hit, int sb_number, int sb_memblock, unsigned int L1_read, unsigned int L1_write, unsigned int L2_read, unsigned int L2_write)
{
	//Streamed Buffers

//counter_arr, index_arr, valid_counter_max
int counter_arr[sb_number], index_arr[sb_number], valid_counter_max=-1;
pair<int, int> counter_index[sb_number];

for (int i = 0; i < sb_number; i++) {
	counter_index[i].first = SB[i][0].count;
	counter_index[i].second = i;
	//printf( "counter_index_first: " << counter_index[i].first << endl;
	//printf( "counter_index_second: " << counter_index[i].second << endl;
	
	//printf(" prefetch");
	if((SB[i][0].sb_valid == 1) && (SB[i][0].count > valid_counter_max)){
		valid_counter_max = SB[i][0].count;
		//printf( "valid_counter_max:" << valid_counter_max << endl;
	}
}

sort(counter_index, counter_index + sb_number);

for (int i = 0; i < sb_number; i++) {
	counter_arr[i] = counter_index[i].first;
	index_arr[i] = counter_index[i].second;
	//printf( "counter_arr: " << counter_arr[i] << endl;
	//printf( "index_arr: " << index_arr[i] << endl;
	}

// Check for hits
int hit_check_buffer = -1;
int hit_check_counter = -1;
int hit_check_depth = -1;
int buffer_num = -1;
for (int i = 0; i <= valid_counter_max; i++){
	buffer_num = index_arr[i];
	//printf( "buff_num" << buffer_num );
	
	//Hit check
	for (int j=0; j < sb_memblock; j++){
		if ((SB[buffer_num][j].sb_tag == prefetch_addr) && (SB[buffer_num][0].sb_valid==1)){
			hit_check_counter = i;
			hit_check_buffer = buffer_num;
			hit_check_depth = j;
			//printf( "hit_depth" << hit_check_depth << endl;
			//printf(" prefetchhit\n");
			if(matched_hit == 0 && L1_read == 1){L1_miss_buff_hit_read++;}
			if(matched_hit == 0 && L1_write == 1){L1_miss_buff_hit_write++;}
			if(matched_hit == 0 && L2_read == 1){L2_miss_buff_hit_read++;}
			if(matched_hit == 0 && L2_write == 1){L2_miss_buff_hit_write++;}
			
			break;
		}
	}
	
	// Check if hit
	if (hit_check_buffer!= -1) {
		//prefetch_hit++;
		//Update preread hits, depth_array
		prefetch_hit = prefetch_hit + hit_check_depth + 1;
		for (int j=0; j < sb_memblock; j++){
			SB[hit_check_buffer][j].sb_tag = prefetch_addr + j + 1;
		}
		break;
	}
}
// Update counter value in case of hit
if (hit_check_counter!= -1){
	for (int i =0; i<sb_number; i++){
		if(SB[i][0].count<hit_check_counter){
			SB[i][0].count = SB[i][0].count + 1;
		}
		else if(SB[i][0].count == hit_check_counter){
			SB[i][0].count = 0;
		}
	}
}
else 
	{
	if(matched_hit==0){// It is a miss
	//prefetch_hit++;
	//printf(" prefetch miss\n");
	
	prefetch_miss = prefetch_miss + sb_memblock;
	buffer_num = index_arr[sb_number-1]; //Index of buffer that contains maximum counter
	// Update depth_array
	for (int j=0; j < sb_memblock; j++){
		SB[buffer_num][0].sb_valid = 1;
		SB[buffer_num][j].sb_tag = prefetch_addr + j + 1;
	}
	// Update Counter value in case of miss
	for (int i =0; i<sb_number; i++){
		if(i != buffer_num){
			SB[i][0].count = SB[i][0].count + 1;
		}
		else {
			SB[i][0].count = 0;
		}
	}
	}
}


}
