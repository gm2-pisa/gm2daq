/**
 * @file    analyzer/modules/ats9870/ats9870.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sat Jan 28 18:04:01 2012
 * @date    Last-Updated: Fri Apr 13 19:59:43 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 36 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   main header file for ATS9870
 * 
 * @details Decode data from ATS9870. 
 *          Make raw waveforms from ATS9870. 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef ATS9870_H
#define ATS9870_H

#include <vector>
#include <sys/types.h>

/**
 * Number of channels in ATS9870 board
 */
#define ATS9870_NUMBER_OF_CHANNELS   2

/**
 *  @brief Waveforms from ATS9870
 */
typedef struct s_ats9870_waveform 
{
  u_int64_t time;                   ///< waveform time
  std::vector<unsigned char>adc;    ///< ADC samples
} ATS9870_WAVEFORM;

/**
 * Array of waveforms stored as std::vectors. 
 */ 
extern std::vector<ATS9870_WAVEFORM> ats9870_waveforms[ATS9870_NUMBER_OF_CHANNELS+1];

#undef EXTERN

#endif // ATS9870_H defined
