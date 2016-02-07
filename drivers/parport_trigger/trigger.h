/**
 * @file    drivers/parport_trigger/trigger.h
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Mon Jan 23 14:39:38 2012
 * @date    Last-Updated: Tue Feb 14 10:12:48 2012 (-0500)
 *          By : g minus two
 *          Update #: 13 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @brief   
 * 
 * @details 
 * 
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */
#ifndef TRIGGER_H
#define TRIGGER_H

/**
 * @brief Trigger 
 *
 * @details Messages of this type are passed from the kernel space
 * to the user space trhough the special file /dev/trigger
 *
 */ 
typedef struct {
  int            irq;         /**< irq number (normally 7 for parallel port) */
  int            trigger_nr;  /**< trigger number */
  unsigned char  mask;        /**< data read from parallel port */
  struct timeval time;        /**< trigger time */
} TRIGGER_INFO;


#endif /* TRIGGER_H defined */
