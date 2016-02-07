/**
 * @file    frontends/master/master.h
 * @author  
 * @date    Tue Jun  5 15:51:04 2012
 * @date    Last-Updated: Tue Jun  5 15:54:56 2012 (-0500)
 *          By : Data Acquisition
 *          Update #: 3 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @todo Document this code 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef master_h
#define master_h


/**  trigger timing information */
typedef struct s_trigger_time_info
{
  DWORD trigger_nr;            ///< trigger number (from RPC message from master)
  DWORD trigger_mask;          ///< trigger mask (from RPC message from master)
  DWORD time_s;                ///< trigger time (from RPC message from master), seconds
  DWORD time_us;               ///< trigger time (from RPC message from master ), microseconds
  DWORD time_recv_s;           ///< recepience time of RPC message, seconds
  DWORD time_recv_us;          ///< recepience time of RPC message, microseconds
  DWORD time_done_s;           ///< end time of event readout, seconds
  DWORD time_done_us;          ///< end time of event readout, microseconds  
}  S_TRIGGER_TIME_INFO;


#endif // master.h defined

