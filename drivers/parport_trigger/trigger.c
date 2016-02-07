/**
 * @file    drivers/parport_trigger/trigger.c
 * @author  Vladimir Tishchenko <tishenko@pa.uky.edu>
 * @date    Sun Jan 22 16:35:30 2012
 * @date    Last-Updated: Wed Jul 16 12:05:57 2014 (-0400)
 *          By : Data Acquisition
 *          Update #: 205 
 * @version $Id$
 * 
 * @copyright (c) new (g-2) collaboration
 * 
 * @ingroup group_drivers
 * 
 * @brief   Linux kernel driver for parallel port
 * 
 * @details
 *
 * This driver is used to generate triggers in the exeperiment
 * throught the parallel port of a PC. 
 * It handles parallel port interrupts (normally IRQ=7).
 * 
 * Interrupts are used to wake-up processes in a user space 
 * though a special file /dev/trigger. 
 * Typically a master client polls the /dev/trigger file 
 * with infinite poll time. 
 * The driver writes \ref TRIGGER_INFO messages to special file /dev/trigger
 * on every trigger. 
 * The driver does not write into the file /dev/triggers if the file is not
 * empty (the previous trigger was not processed) or if the file is not opened
 * for reading by consumers.
 * 
 * Note that this version requiers the parallel port to be configured in EPP mode
 * 
 * TG added trigger_write() function to write char to output data lines
 *
 * @section Changelog
 * @verbatim
 * $Log$
 * @endverbatim
 */



#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <asm/irq_vectors.h>

#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/time.h>


#include "trigger.h"

/**
 *  IRQ number 
 */ 

// default parallel port IRQ
#define IRQ 7 

// fe01 parallel port adapter card IRQ ?
//#define IRQ 11

// fe01
#define PARALLEL_PORT_ADDR_BASE     0x378

// fe01 pcibus parallel port card ?
//#define PARALLEL_PORT_ADDR_BASE     0xacf0

//fe02
//#define PARALLEL_PORT_ADDR_BASE     0x278

#define PARALLEL_PORT_ADDR_STATUS   (PARALLEL_PORT_ADDR_BASE + 0x1)
#define PARALLEL_PORT_ADDR_CONTROL  (PARALLEL_PORT_ADDR_BASE + 0x2)
#define PARALLEL_PORT_ADDR_EPP_ADDR (PARALLEL_PORT_ADDR_BASE + 0x3)
#define PARALLEL_PORT_ADDR_EPP_DATA (PARALLEL_PORT_ADDR_BASE + 0x4)

/** major number of the /dev/trigger file */
#define MOD_MAJOR         61

DECLARE_WAIT_QUEUE_HEAD(waitq);

struct fasync_struct *async_queue;

static unsigned long devid;

static spinlock_t trigger_lock = SPIN_LOCK_UNLOCKED;       /* spin lock for busy check */
static int interrupt_ready;            /* crate/station which caused interrupt */
static int module_in_use = 0;          /* flag if device in use */
static int in_use_pid;                 /* PID of process which uses device */
static int interrupt_count=0;

static TRIGGER_INFO trigger_info;


/*---- fasync ------------------------------------------------------*/

int trigger_fasync(int fd, struct file *filep, int mode)
{
   printk("trigger: fasync %d\n", mode);
   return fasync_helper(fd, filep, mode, &async_queue);
}

/* cler time outbit in sttus register */

int clear_timeout(){

   unsigned char stat;

   stat = inb( PARALLEL_PORT_ADDR_STATUS);
   //printk("read status register before timeout clear (bit-0) 0x%04x\n",stat);

   // timeout bits are sometimes cleared by writing "0" and dometimes cleared by writing "1"

   //stat = (stat & 0xfe); // try clearing timeout bit by writing zero to bit-1, doesnt work on fe01
   //outb(stat, PARALLEL_PORT_ADDR_STATUS);

   stat = (stat | 0x01); // try clearing timeout bit by writing one to bit-1, does work on fe01
   outb(stat, PARALLEL_PORT_ADDR_STATUS);

   stat = inb( PARALLEL_PORT_ADDR_STATUS);
   //printk("read status register after timeout clear (bit-0) 0x%04x\n",stat);

   return 0;
}

int set_nstrobe(){

   /* initialize control register */
   unsigned char ctl;
   ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
   ctl = (ctl & 0xff) | 0x1; // set bit-0 to one nstrobe
   outb(ctl, PARALLEL_PORT_ADDR_CONTROL);
   printk("set nstrobe, ctl 0x%04x\n",ctl);

   return 0;
}

int clear_nstrobe(){

   /* clear_nstrobe */
   unsigned char ctl;
   ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
   ctl = (ctl & 0xfe); // set bit-0 to zero for nstrobe
   outb(ctl, PARALLEL_PORT_ADDR_CONTROL);
   printk("clear nstrobe , ctl 0x%04x\n",ctl);

   return 0;
}


static int trigger_open(struct inode *inode, struct file *file)
{
   unsigned int minor;
   unsigned long flags;
   if (!try_module_get(THIS_MODULE)) {
     return -EBUSY;
   }
   minor = MINOR(inode->i_rdev);
   printk(KERN_INFO "trigger: open called by PID %u, on minor %d\n", current->pid,
          minor);

   spin_lock_irqsave(&trigger_lock,flags);

   if (module_in_use) {
      /* return if device already open */
      spin_unlock_irqrestore(&trigger_lock,flags);
      module_put(THIS_MODULE);
      return -EBUSY;
   }

   /* mark usage */
   module_in_use = 1;
   in_use_pid = current->pid;
   interrupt_ready = 0;
   interrupt_count = 0;
   spin_unlock_irqrestore(&trigger_lock,flags);

   /* initialize control register */
   unsigned char ctl;

   ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
   printk("read ctl register before initializing 0x%04x\n",ctl);

   ctl = (ctl & 0xf0) | 0x4; // initialize control register
   outb(ctl, PARALLEL_PORT_ADDR_CONTROL);

   ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
   printk("read ctl register after initializing 0x%04x\n",ctl);

   ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
   printk("read ctl register before clearing bit-5 for forward/output direction 0x%04x\n",ctl);

   //ctl = (ctl & 0xef); // clear bit-5 register for output
   ctl = (ctl & 0xef) | 0x10; // set bit-5 register
   outb(ctl, PARALLEL_PORT_ADDR_CONTROL);

   ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
   printk("after ctl register before clearing bit-5 for forward/output direction  0x%04x\n",ctl);

   if (clear_timeout() < 0) {
     printk("failed to clear timeout bit\n");
   }

   return 0;
}


/*---- close -------------------------------------------------------*/

static int trigger_release(struct inode *inode, struct file *filep)
{
   unsigned int minor;
   unsigned long flags;
   minor = MINOR(inode->i_rdev);
   printk(KERN_INFO "trigger: release called by PID %d, on minor %d\n", current->pid,
          minor);

   /* mark device unused */
   spin_lock_irqsave(&trigger_lock,flags);
   module_in_use = 0;
   spin_unlock_irqrestore(&trigger_lock,flags);

   /* remove file from asynchronous readers */
   trigger_fasync(-1, filep, 0);

   module_put(THIS_MODULE);
   return 0;
}



/*---- read --------------------------------------------------------*/

ssize_t trigger_read(struct file *filep, char *buf, size_t count, loff_t * ppos)
{

  ssize_t copied;

  while (!interrupt_ready) 
    {
      if (filep->f_flags & O_NONBLOCK)
	return -EAGAIN;
      
      if (signal_pending(current))
	return -ERESTARTSYS;
      
      //  sleep until a condition gets true 
      wait_event_interruptible(waitq,interrupt_ready);
    }

  if ( clear_timeout() < 0) {
    printk("failed to clear timeout bit\n");
  }
  

  //trigger_info.mask = inb_p(PARALLEL_PORT_ADDR_STATUS); 
  //printk(">>> PARALLEL PORT read: interrupt %i, status register: 0x%08x\n",interrupt_count,trigger_info.mask);
  
  //trigger_info.mask = inb_p(PARALLEL_PORT_ADDR_CONTROL); 
  //printk(">>> PARALLEL PORT read: interrupt %i, control register: 0x%08x\n",interrupt_count,trigger_info.mask);
  
  //trigger_info.mask = inb_p(PARALLEL_PORT_ADDR_EPP_ADDR); 
  //printk(">>> PARALLEL PORT read: interrupt %i, epp address: 0x%08x\n",interrupt_count,trigger_info.mask);

  //trigger_info.mask = inb(PARALLEL_PORT_ADDR_EPP_DATA); // read epp data
  //trigger_info.mask = inb(PARALLEL_PORT_ADDR_BASE); //read spp data
  //printk(">>> PARALLEL PORT read: interrupt %i, epp data: 0x%08x\n",interrupt_count,trigger_info.mask);

  trigger_info.irq  = IRQ;
  trigger_info.trigger_nr = interrupt_count;
  if ( copy_to_user(buf, &trigger_info,  sizeof(trigger_info)) != 0 )
    {
      printk("***ERROR! Trigger was unable to copy data to user space");
      copied = 0;
    }
  else
    copied = sizeof(trigger_info);
  
  interrupt_ready = 0;
  
  return copied;

}

/*---- write --------------------------------------------------------*/

ssize_t trigger_write(struct file *filep, const char *buf, size_t count, loff_t * ppos)
{
  
  char datum;
  datum = *buf;

  /*
in epp write mode the handshaking with peripheral is generated by hardware and our write will timeout. I'm
therefore writing the data to the spp data address (base+0) where program would do the hand-shaking. I'm
also not clearing the timeout bit.
  */

  /*
  if ( clear_timeout() < 0) {
    printk("failed to clear timeout bit\n");
  }
  */

  /*
  unsigned char ctl, status;
  ctl = inb( PARALLEL_PORT_ADDR_CONTROL); // is control write only?
  printk(">>> PARALLEL PORT write: control register read  0x%04x\n",ctl);
  status = inb(PARALLEL_PORT_ADDR_STATUS); // is status only?
  printk(">>> PARALLEL PORT write: status register read  0x%04x\n",status);
  */

  //ctl = (ctl & 0xef); // clear bit-5 register for output
  //outb(ctl, PARALLEL_PORT_ADDR_CONTROL);

  outb( datum, PARALLEL_PORT_ADDR_BASE); // write spp data
  //outb( datum, PARALLEL_PORT_ADDR_EPP_DATA); // write epp data
  //printk(">>> PARALLEL PORT write: , *buf 0x%02x, datum 0x%02x, count %d \n", *buf, datum, count);

  //ctl = (ctl & 0xef) | 0x10; // set bit-5 register
  //outb(ctl, PARALLEL_PORT_ADDR_CONTROL);
 
  // if (clear_timeout() < 0) {
  //   printk("failed to clear timeout bit\n");
  // }

  //if (datum) {
  //  set_nstrobe();
  //} else {
  //  clear_nstrobe();
  //}

  return 0;
}

/*---- poll --------------------------------------------------------*/

unsigned int trigger_poll(struct file *filep, struct poll_table_struct *wait)
{
  poll_wait(filep, &waitq, wait);

  if (interrupt_ready)
    return POLLIN | POLLRDNORM;
  
  return 0;
}



/** interrupt handler */
static irqreturn_t trigger_interrupt_handler(int irq, void *data)
{

  int status;

  // do stuff
  interrupt_count++;
  //printk(">>> PARALLEL PORT interrupt %d\n",interrupt_count);

  status = inb_p(PARALLEL_PORT_ADDR_EPP_DATA); 
  //status = inb_p(PARALLEL_PORT_ADDR_EPP_DATA); 
  //status = inb_p(PARALLEL_PORT_ADDR_EPP_DATA); 
  //status = inb_p(PARALLEL_PORT_ADDR_EPP_DATA); 
  //printk(">>> PARALLEL PORT interrupt handler, interrupt %i, status: 0 0x%08x\n",interrupt_count,status);

  // wake up (unblock) for reading data from userspace
  // and ignore first interrupt generated in module init
  //if (interrupt_count > 1) {
  //  data_not_ready = 0;
  //  wake_up_interruptible(&skeleton_wait);
  // }

  if ( interrupt_ready == 0 ) {
  
    interrupt_ready = 1;

    /* interrupt time */
    do_gettimeofday( (struct timeval *)&trigger_info.time );
    //printk("time: %i s %i us\n",trigger_info.time.tv_sec, trigger_info.time.tv_usec);

    /* wake user process */
    wake_up_interruptible(&waitq);
    
    /* send signal to user process */
    kill_fasync(&async_queue, SIGIO, POLL_IN);
    
  } else {
    //printk("ignore interrupt\n");
  }

  return IRQ_HANDLED;
}


static struct file_operations trigger_fops = {
   open:&trigger_open,
   read:&trigger_read,
   write:&trigger_write,
   poll:&trigger_poll,
   release:&trigger_release,
};



static int __init trigger_init_module(void)
{

  int ret;                    ///< function return value
  unsigned char epp_ctl;      ///< used to setup EPP port

  // REGISTER IRQ handler
  ret = request_irq(IRQ, &trigger_interrupt_handler,
		    //SA_SHIRQ, "parallelport", (void *)&devid);
		    IRQF_SHARED, "parallelport", (void *)&devid);
  //enable_irq(IRQ);

   if (register_chrdev(MOD_MAJOR, "trigger", &trigger_fops)) {
     printk(KERN_ERR "trigger: unable to get major %d\n", MOD_MAJOR);
     return -EBUSY;
   }


  if (ret) { 
    printk ("parint: error requesting irq: returned %d\n", ret); 
  }

  
  // Prepare EPP port for direct io
  epp_ctl = inb(PARALLEL_PORT_ADDR_CONTROL);
  epp_ctl = (epp_ctl & 0xF0) | 0x4;
  outb(epp_ctl, PARALLEL_PORT_ADDR_CONTROL);

  // clear any possible EPP timeouts
  if ((inb(PARALLEL_PORT_ADDR_CONTROL) & 0x01)) 
    {
      epp_ctl = inb(PARALLEL_PORT_ADDR_STATUS); 
      outb(epp_ctl | 0x01, PARALLEL_PORT_ADDR_STATUS); /* Some reset by writing 1 */ 
      outb(epp_ctl & 0xfe, PARALLEL_PORT_ADDR_STATUS); /* Others by writing 0 */
    }

  // set port to interrupt mode
  outb_p(0x10, PARALLEL_PORT_ADDR_CONTROL); 


  printk("module trigger initialized successfully \n");
  
  return ret;

}

static void __exit trigger_cleanup_module(void)
{
  printk("cleanup trigger module...\n");
  //disable_irq(IRQ);
  free_irq(IRQ, (void *)&devid);

  //if (unregister_chrdev(MOD_MAJOR, "trigger"))
  //  printk(KERN_ERR "trigger: can't unregister device\n");

  unregister_chrdev(MOD_MAJOR, "trigger");

}

module_init(trigger_init_module);
module_exit(trigger_cleanup_module);
MODULE_AUTHOR("tishenko@pa.uky.edu");
MODULE_LICENSE("BSD");
MODULE_DESCRIPTION("Linux Device Driver for Interrupt Handler of parallel port");

