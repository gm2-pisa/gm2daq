/**
 * @file    analyzer/modules/amc13/amc13.h
 * @author  Wes Gohn <gohn@pa.uky.edu>
 * @date    Fri May 23 15:38:01 2014
 * @date    Last-Updated:
 *          By : Data Acquisition
 *          Update #: 44
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   main header file for AMC13 
 * 
 * @details Raw waveforms from AMC13. 
 *          Defines the number of shelves with AMC13 and 
 *          the number of modules per shelf
 * 
 * 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef AMC13_H
#define AMC13_H

#ifdef amc13_c
#define EXTERN
#else
#define EXTERN extern
#endif

#include <vector>

/**
 * Number of AMC13 shelves in the experiment
 */
#define AMC13_NUMBER_OF_SHELVES    1

/**
 * Number of AMC13 boards per shelf
 */
<<<<<<< HEAD
//#define AMC13_NUMBER_OF_AMCS_PER_SHELF   11
=======
>>>>>>> f8c0416457313cc760bd73779643ec6f54e6f01d
#define AMC13_NUMBER_OF_AMCS_PER_SHELF   1

/**
 * Number of channels in AMC13 board
 */

#define AMC13_NUMBER_OF_CHANNELS   5

/**
 * Size of data from AMC13
 */
//#define AMC13_DATA_SIZE 0xf000
////#define AMC13_DATA_SIZE 61440
//#define AMC13_DATA_SIZE 0x100
//#define AMC13_DATA_SIZE 0x3fff0
<<<<<<< HEAD
=======

//#define AMC13_DATA_SIZE 32440320 // 11 AMCs * 5 chans * 589824 samples/chan 

//#define AMC13_DATA_SIZE 412
#define AMC13_DATA_SIZE 2621440  // single 5-chan rider tests 2621440 8*65536*5
>>>>>>> f8c0416457313cc760bd73779643ec6f54e6f01d

//#define AMC13_DATA_SIZE 32440320 // 11 AMCs * 5 chans * 589824 samples/chan 
#define AMC13_DATA_SIZE 2621440  // single 5-chan rider tests 2621440 8*65536*5

/**
 *  @brief Waveforms from AMC13
 */
typedef struct s_amc13_waveform 
{
  unsigned long long int time;          ///< waveform time
  std::vector<unsigned short int>adc;   ///< ADC samples
} AMC13_WAVEFORM;


EXTERN std::vector<AMC13_WAVEFORM> amc13_waveforms[AMC13_NUMBER_OF_SHELVES+1][AMC13_NUMBER_OF_AMCS_PER_SHELF+1][AMC13_NUMBER_OF_CHANNELS+1];

#undef EXTERN

#endif // AMC13_H defined
