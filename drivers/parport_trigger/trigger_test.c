/*
 * file: trigger_test.c
 * Linux Devide Driver
 * Kernel Module
 * Interrupt handler
 * 
 * this module simply prints out kernel message
 * for every interrupt
 */



#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <asm/io.h>


#define IRQ 7
#define PARALLEL_PORT 0x278

static unsigned long devid;


// interrupt handler
static irqreturn_t trigger_interrupt_handler(int irq, void *data, struct pt_regs *reqs)
{

  static int interruptcount=0;

  // do stuff
  interruptcount++;
  printk(">>> PARALLEL PORT INT HANDLED: interruptcount=%d\n", interruptcount);
  
  // wake up (unblock) for reading data from userspace
  // and ignore first interrupt generated in module init
  //if (interruptcount > 1) {
  //  data_not_ready = 0;
  //  wake_up_interruptible(&skeleton_wait);
  // }
  
  return IRQ_HANDLED;
}


static int __init trigger_init_module(void)
{

  // REGISTER IRQ handler
  int ret = request_irq(IRQ, &trigger_interrupt_handler,
			//SA_INTERRUPT | SA_SHIRQ, "parallelport", (void *)&devid);
			 SA_INTERRUPT, "parallelport", (void *)&devid);
  //enable_irq(IRQ);

  if (ret) { 
    printk ("parint: error requesting irq: returned %d\n", ret); 
  }

  // set port to interrupt mode
  outb_p(0x10, PARALLEL_PORT + 2 ); 
  //outb_p(0x0,  PARALLEL_PORT); 
  //outb_p(0xFF,  PARALLEL_PORT); 


  
  printk("module initialized successfully \n");
  
  return ret;

}

static void __exit trigger_cleanup_module(void)
{
  printk("cleanup module...\n");
  //disable_irq(IRQ);
  free_irq(IRQ, (void *)&devid);
}

module_init(trigger_init_module);
module_exit(trigger_cleanup_module);
MODULE_AUTHOR("tishenko@pa.uky.edu");
MODULE_LICENSE("BSD");
MODULE_DESCRIPTION("Linux Device Driver with Interrupt Handler");
