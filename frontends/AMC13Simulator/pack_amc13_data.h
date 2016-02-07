/**
   @file   pack_amc13_data.h
   @author Wes Gohn <gohn@pa.uky.edu>
   @date   Mon Jan 12 2014
**/

#ifndef pack_amc13_data_h
#define pack_amc13_data_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_WORDS_PER_BLOCK  4096;
#define nAMC 11
#define nCHAN 5
#define AMC_SIZE_MASK 0xffffff
#define BLOCK_MAX_MASK (MAX_WORDS_PER_BLOCK-1)
#define NSAMPW ((700*800)/4)
#define NWFDW (NSAMPW*nCHAN)

typedef struct 
  {
    uint64_t header;
    uint64_t payload[nAMC][4096];
    uint64_t tail;
  } amc_block;
  
typedef struct 
{
  int16_t         *data;         /* simulated data + rider headers / trailers */
  unsigned int      data_size;     /* size of the data array of simulated data + rider headers / trailers */
} RIDER_INFO;

RIDER_INFO rider_info;

int pack_rider_data();
int pack_amc13_data(uint64_t* amc13_packed_data,  int n);
  

#ifdef __cplusplus
}
#endif
#endif
