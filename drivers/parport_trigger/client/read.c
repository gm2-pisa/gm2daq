#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

#include "../trigger.h"

int main(int argc, char **argv)
{

  TRIGGER_INFO trigger_info;

  //int fd = open("/dev/trigger",O_RDONLY );
  int fd = open("/dev/trigger",O_RDWR );

  // *** clean buffer ***
  int n_read = 0;

  struct pollfd pfds;
  
  pfds.fd = fd;
  pfds.events = POLLIN;

  int i = 0;

#if 0
  int ret = poll( &pfds, 1, -1);
  printf("poll result: %i\n", ret);
  
  if ( pfds.revents > 0 ) {
    printf("%i LAMs in the buffer\n",pfds.revents);
    //n_read = read(fd, xxx, 3) ;
    //printf("Initial bytes read: %i, data: %i, irq # %i\n", n_read, xxx[0], xxx[1]);
  } else {
    printf("No data available\n");
  }
#endif 

#if 0
  if ( poll( &pfds, 1, 10) > 0 ) {
    n_read = read(fd, &trigger_info, sizeof(trigger_info));
    printf("Initial bytes read: %i, data: %i\n", n_read, xxx[0]);   
  } else {
    printf("No data available\n");
  }
#endif 

  printf("Polling triggers....\n");
  while ( 1 ) {

    int timeout = 10000;  // timeout in milliseconds
    int ret = poll( &pfds, 1, timeout);
    if ( ret <= 0 )
      {
        printf("WARNING! No events for %i milliseconds\n",timeout);
        continue;
      }

    i++;
    printf("LAM %i: \n",i);
    usleep(10000);

#if 1

        if ( (n_read = read(fd, &trigger_info, sizeof(trigger_info))) <= 0 )
      {
        printf("***ERROR reading from file descriptor\n");
        break;
      }
    
    printf("bytes read: %i, irq: %i, trigger # %i, mask: 0x%02x\n", 
           n_read, 
           trigger_info.irq, 
           trigger_info.trigger_nr, 
           trigger_info.mask);
#endif
   
#if 1
    int status;
    char datum;
    
    /*
    usleep(50000.);
    datum = 0x7f;  
    printf("write datum %c 0x%02x, size(datum) %d, status %d\n", datum, datum, sizeof(datum), status);
    status = write(fd, &datum, sizeof(datum));
    usleep(100.);
    */
    datum = 0x00;  
    printf("write datum %c 0x%02x, size(datum) %d, status %d\n", datum, datum, sizeof(datum), status);
    status = write(fd, &datum, sizeof(datum));
    

#endif 
    
#if 0
    while ( 1 ) {
      xxx = 0;
      n_read = read(fd, &xxx, 1) ;
      printf("bytes read: %i, data: %i\n", n_read, xxx);
      if ( n_read <= 0 ) break;
    }
#endif

#if 0
    if ( read(fd, &xxx, 1) > 0 ) {
      printf("***ERROR! More then one LAM in buffer!\n");
      while ( read(fd, &xxx, 1) > 0 );
    }
#endif

  }


  close(fd);

  return 0;

}
