/**
 * @file    analyzer/modules/sis3350/sis3350.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Jan 28 18:04:01 2012
 * @date    Last-Updated: Mon Nov 11 18:36:36 2013 (-0600)
 *          By : Data Acquisition
 *          Update #: 31 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   main header file for SIS3350 
 * 
 * @details Raw waveforms from SIS3350. 
 *          Defines the number of crates with SIS3350 and 
 *          the number of modules per crate
 * 
 * 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef SIS3350_H
#define SIS3350_H

#ifdef sis3350_c
#define EXTERN
#else
#define EXTERN extern
#endif

#include <vector>

/**
 * Number of SIS3350 crates in the experiment
 */
#define SIS3350_NUMBER_OF_CRATES    1

/**
 * Number of SIS3350 boards per crate
 */
#define SIS3350_NUMBER_OF_BOARDS_PER_CRATE   2

/**
 * Number of channels in SIS3350 board
 */
#define SIS3350_NUMBER_OF_CHANNELS   4

/**
 *  @brief Waveforms from SIS3350
 */
typedef struct s_sis3350_waveform 
{
  unsigned long long int time;          ///< waveform time
  std::vector<unsigned short int>adc;   ///< ADC samples
} SIS3350_WAVEFORM;

/**
 * Array of waveforms stored as std::vectors. 
 */ 
EXTERN std::vector<SIS3350_WAVEFORM> sis3350_waveforms[SIS3350_NUMBER_OF_CRATES+1][SIS3350_NUMBER_OF_BOARDS_PER_CRATE+1][SIS3350_NUMBER_OF_CHANNELS+1];

#undef EXTERN

#endif // SIS3350_H defined
