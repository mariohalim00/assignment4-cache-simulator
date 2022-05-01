#include "cacheSim.h"

// In this question, we will assume DRAM will take a 4-byte values starting from 0 to 
/**
Cache has two Levels
A cache block is 16 bytes // the BIB bit size is Log2(16) = 4
	...bbbb
Each address is accessing 1 bytes of data 
L1 is 64 Bytes, 2-way
	// 2 blocks per set
	// 1CB = 16 Bytes
	// L1 has 64 Bytes * 1/16 blocks/ Bytes = // 4 Blocks //
	// 4 blocks / 2 block per set = // 2 sets // set id bits : Log2(2) = 1
	// address (? bits) = ...tttsbbbb
L2 is 256 Bytes, 4-Way
	// 4 blocks per set 
	// 1CB = 16 Bytes
	// L2 has 256 Bytes * 1/16 Blocks per Bytes = // 16 Blocks //
	// 16 Blocks / 4 blocks per set = // 4 sets // set id bits: log2(4) = 2
	// address (? bits) = ...tttttssbbbb
L1's data is a subset of L2's data (to simplify the design of the caching)
FIFO cache replacement policy
**/
void init_DRAM()
{
	unsigned int i=0;
	DRAM = malloc(sizeof(char) * DRAM_SIZE);
	for(i=0;i<DRAM_SIZE/4;i++)
	{
		*((unsigned int*)DRAM+i) = i;
	}
}

void printCache()
{
	int i,j,k;
	printf("===== L1 Cache Content =====\n");
	for(i=0;i<2;i++)
	{
		printf("Set %d :", i);
		for(j=0;j<2;j++)
		{
			printf(" {(TAG: 0x%x)", (unsigned int)(L1_cache[i][j].tag));
			for(k=0;k<16;k++)
				printf(" 0x%x,", (unsigned int)(L1_cache[i][j].data[k]));
			printf(" |");
		}
		printf("\n");
	}
	printf("===== L2 Cache Content =====\n");
	for(i=0;i<4;i++)
	{
		printf("Set %d :", i);
		for(j=0;j<4;j++)
		{
			printf(" {(TAG: 0x%x)", (unsigned int)(L2_cache[i][j].tag));
			for(k=0;k<16;k++)
				printf(" 0x%x,", (unsigned int)(L2_cache[i][j].data[k]));
			printf(" |");
		}
		printf("\n");
	}
}

u_int32_t read_fifo(u_int32_t address)
{
  
	// step 1: get BIB
	int mask = (1 << 4) + ~0;
	int bib = address & mask;
	// step 2: get set id
	int setL1 = getL1SetID(address);
	int setL2 = getL2SetID(address);
	// step 3: get tag:
	int tagL1 = getL1Tag(address);
	int tagL2 = getL2Tag(address);
	//check for address in L1 and L2
	int flagL1 = L1lookup(address);
	int flagL2 = L2lookup(address);

	if(flagL1){
		// if data is found on l1: loop through the set and then update and return data
		for(int i = 0; i < 2; i++){
			if(L1_cache[setL1][i].tag == tagL1 ){
				L1_cache[setL1][i].timeStamp = cycles;
				// printf("read in L1: %x\n", L1_cache[setL1][i].data[bib]);
				return L1_cache[setL1][i].data[bib];
			}
		}
	}else{
		//if CB empty:
		int full_counter = 0;
		for(int i = 0; i < 2; i++){
			if(L1_cache[setL1][i].tag == 0  ){
				L1_cache[setL1][i].tag = tagL1;
				L1_cache[setL1][i].data[bib] = 0;
				L1_cache[setL1][i].data[bib + 1] = 0;
				L1_cache[setL1][i].data[bib + 2] = 0;
				L1_cache[setL1][i].data[bib + 3] = 0;
				L1_cache[setL1][i].timeStamp = cycles;
				break;
			}else{
				full_counter +=1;
			}
		}

		if(full_counter == 2){
			// if full evict first in 
			int min = 0;
			for(int i = 0; i < 2; i++){
				if(L1_cache[setL1][i].timeStamp < L1_cache[setL1][min].timeStamp ){
					min = i;
				}
			}
				L1_cache[setL1][min].tag = tagL1;
				L1_cache[setL1][min].data[bib] = 0;
				L1_cache[setL1][min].data[bib + 1] = 0;
				L1_cache[setL1][min].data[bib + 2] = 0;
				L1_cache[setL1][min].data[bib + 3] = 0;
				L1_cache[setL1][min].timeStamp = cycles;
		}
	}

	if(flagL2){
		// if data is found on l2 loop through the set and return data
		for(int i = 0; i < 4; i++){
			if(L2_cache[setL2][i].tag == tagL2 ){
				L2_cache[setL2][i].timeStamp = cycles;
				// printf("read in L2: %x\n", L2_cache[setL2][i].data[bib]);
				return L2_cache[setL2][i].data[bib];
			}
		}
	}else{
	//if CB empty:
	int full_counter = 0;
	for(int i = 0; i < 4; i++){
		if(L2_cache[setL2][i].tag == 0 ){
			L2_cache[setL2][i].tag = tagL2;
			L2_cache[setL2][i].data[bib] = 0;
			L2_cache[setL2][i].data[bib + 1] = 0;
			L2_cache[setL2][i].data[bib + 2] = 0;
			L2_cache[setL2][i].data[bib + 3] = 0;
			L2_cache[setL2][i].timeStamp = cycles;
			break;
		}else{
			full_counter +=1;
		}
	}


		if(full_counter == 4){
			// if full evict first in 
			int min = 0;
			for(int i = 1; i < 4; i++){
				if(L2_cache[setL2][i].timeStamp < L2_cache[setL2][min].timeStamp ){
					min = i;
				}
			}
				L2_cache[setL2][min].tag = tagL2;
				L2_cache[setL2][min].data[bib] = 0;
				L2_cache[setL2][min].data[bib + 1] = 0;
				L2_cache[setL2][min].data[bib + 2] = 0;
				L2_cache[setL2][min].data[bib + 3] = 0;
				L2_cache[setL2][min].timeStamp = cycles;
		}
	}
}
	

int L1lookup(u_int32_t address)
{
	// step 1: get set id
	int set = getL1SetID(address);
	// step 2: get BIB
	// int mask = (1 << 4) + ~0;
	// int bib = address & mask;
	// step 3: get tag:
	int tag = getL1Tag(address);
	// return 1 if cache hit
	for(int i = 0; i < 2; i++){
		if(L1_cache[set][i].tag == tag ){
			return 1;
		}
	}
	// return 0 if cache miss
return 0;
}

int L2lookup(u_int32_t address)
{
	// step 1: get set id
	int set = getL2SetID(address);
	// step 2: get BIB
	// int mask = (1 << 4) + ~0;
	// int bib = address & mask;
	// step 3: get tag:
	int tag = getL2Tag(address);
	// return 1 if cache hit
	for(int i = 0; i < 4; i++){
		if(L1_cache[set][i].tag == tag ){
			return 1;
		}
	}
	// return 0 if cache miss
	return 0;
}

unsigned int getL1SetID(u_int32_t address)
{
return (address >> 4) & 1;
}

unsigned int getL2SetID(u_int32_t address)
{
return (address >> 5) & 3;
}

unsigned int getL1Tag(u_int32_t address)
{
return (address >> 5);
}

unsigned int getL2Tag(u_int32_t address)
{
return (address >> 6);
}


void write(u_int32_t address, u_int32_t data)
{
// ///// REPLACE THIS /////

	// step 1: get BIB
	int mask = (1 << 4) + ~0;
	int bib = address & mask;
	// printf("BIB = %d\n", bib);

	// step 2: get set id
	int setL1 = getL1SetID(address);
	int setL2 = getL2SetID(address);
	// printf("setL1 = %d\n", setL1);
	// printf("setL2 = %d\n", setL2);

	// step 3: get tag:
	int tagL1 = getL1Tag(address);
	int tagL2 = getL2Tag(address);
	// printf("tagL1 = %d\n", tagL1);
	// printf("tagL2 = %d\n", tagL2);

	//check for address in L1 and L2
	int flagL1 = L1lookup(address);
	int flagL2 = L2lookup(address);

	// write through policy 
	if(flagL1){
		//L1 hit
		//update L1 and L2
		for(int i = 0; i < 4; i++){
			if(L2_cache[setL2][i].tag == tagL2 ){
				L2_cache[setL2][i].data[bib] = data & 0x000000ff;
				L2_cache[setL2][i].data[bib+1] = data & 0x0000ff00;
				L2_cache[setL2][i].data[bib+2] = data & 0x00ff0000;
				L2_cache[setL2][i].data[bib+3] = data & 0xff000000;
				// printf("wl2: %d, %x\n", L2_cache[setL2][i].data[bib], data);
				L2_cache[setL2][i].timeStamp = cycles;
			}
		}	
		for(int i = 0; i < 2; i++){
			if(L1_cache[setL1][i].tag == tagL1 ){
				L1_cache[setL1][i].data[bib] = data & 0x000000ff;
				L1_cache[setL1][i].data[bib+1] = data & 0x0000ff00;
				L1_cache[setL1][i].data[bib+2] = data & 0x00ff0000;
				L1_cache[setL1][i].data[bib+3] = data & 0xff000000;
				// printf("Wl1: %d, %x\n", L1_cache[setL1][i].data[bib], data);
				L1_cache[setL1][i].timeStamp = cycles;
			}
		}	
		return;
	}
	else if(flagL2){
		//l2 hit, l1 miss
		//update l2
		for(int i = 0; i < 4; i++){
			if(L2_cache[setL2][i].tag == tagL2 ){
				L2_cache[setL2][i].data[bib] = data & 0x000000ff;
				L2_cache[setL2][i].data[bib+1] = data & 0x0000ff00;
				L2_cache[setL2][i].data[bib+2] = data & 0x00ff0000;
				L2_cache[setL2][i].data[bib+3] = data & 0xff000000;
				L2_cache[setL2][i].timeStamp = cycles;
			}
		}	
		// put data in l1
		int min = 0;
		for(int i = 1; i < 2; i++){
			if(L1_cache[setL1][i].timeStamp < L1_cache[setL1][min].timeStamp ){
				min = i;
			}
		}
		L1_cache[setL1][min].tag = tagL1;
		L1_cache[setL1][min].data[bib] = data & 0x000000ff;
		L1_cache[setL1][min].data[bib+1] = data & 0x0000ff00;
		L1_cache[setL1][min].data[bib+2] = data & 0x00ff0000;
		L1_cache[setL1][min].data[bib+3] = data & 0xff000000;
		L1_cache[setL1][min].timeStamp = cycles;		

		return;
	} 
	else{
		//if both is a miss: put data in both L1 and L2
		// put data in l1
		int min = 0;
		for(int i = 1; i < 2; i++){
			if(L1_cache[setL1][i].timeStamp < L1_cache[setL1][min].timeStamp ){
				min = i;
			}
		}
			L1_cache[setL1][min].tag = tagL1;
			//each index is 1 byte
			L1_cache[setL1][min].data[bib] = data & 0x000000ff;
			L1_cache[setL1][min].data[bib+1] = data & 0x0000ff00;
			L1_cache[setL1][min].data[bib+2] = data & 0x00ff0000;
			L1_cache[setL1][min].data[bib+3] = data & 0xff000000;
			L1_cache[setL1][min].timeStamp = cycles;	

			// put data in l1
		 min = 0;
		for(int i = 1; i < 4; i++){
			if(L2_cache[setL2][i].timeStamp < L2_cache[setL2][min].timeStamp ){
				min = i;
			}
		}
			L2_cache[setL2][min].tag = tagL1;
			L2_cache[setL2][min].data[bib] = data & 0x000000ff;
			L2_cache[setL2][min].data[bib+1] = data & 0x0000ff00;
			L2_cache[setL2][min].data[bib+2] = data & 0x00ff0000;
			L2_cache[setL2][min].data[bib+3] = data & 0xff000000;
			L2_cache[setL2][min].timeStamp = cycles;	
	}

return;
}


int main()
{
	init_DRAM();
	cacheAccess buffer;
	int timeTaken=0;
	FILE *trace = fopen("input.trace","r");
	int L1hit = 0;
	int L2hit = 0;
	cycles = 0;
	while(!feof(trace))
	{
		fscanf(trace,"%d %x %x", &buffer.readWrite, &buffer.address, &buffer.data);
		printf("Processing the request for [R/W] = %d, Address = %x, data = %x\n", buffer.readWrite, buffer.address, buffer.data);

		// Checking whether the current access is a hit or miss so that we can advance time correctly
		if(L1lookup(buffer.address))// Cache hit
		{
			timeTaken = 1;
			L1hit++;
		}
		else if(L2lookup(buffer.address))// L2 Cache Hit
		{
			L2hit++;
			timeTaken = 5;
		}
		else timeTaken = 50;
		if (buffer.readWrite) write(buffer.address, buffer.data);
		else read_fifo(buffer.address);
		cycles+=timeTaken;
	}
	printCache();
	printf("Total cycles used = %ld\nL1 hits = %d, L2 hits = %d\n", cycles, L1hit, L2hit);
	fclose(trace);
	free(DRAM);
	return 0;
}




