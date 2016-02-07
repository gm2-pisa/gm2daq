#ifndef emc_h
#define emc_h

#include <vector>

/**
 *  EMC hit structure 
 */
struct EMC_HIT {
  int wire;         // wires: X: 1-48; Y: 49-96
  long int time;    // time
};

/**
 * Array of ALL EMC hits as std::vectors (unsorted). 
 */ 
extern std::vector<EMC_HIT> emc_hits;

/**
 * X-Y coincidences in EMC
 */
struct EMC_HIT_XY {
  EMC_HIT x; 
  EMC_HIT y;
};
extern std::vector<EMC_HIT_XY> emc_hits_xy;


#endif // emc_h defined
