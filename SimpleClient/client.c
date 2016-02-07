#define _GNU_SOURCE    
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/time.h>

//for select
#include <sys/select.h>

//for open permission defines
#include <sys/stat.h> 
#include <fcntl.h>

//for signals
#include <signal.h>

//for addrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//for byte swa
#include <endian.h>

//for timespec and gettime
#include <time.h>

//for PRIx64 printf magic
#include <inttypes.h>

//for get_nprocs()
#include <sys/sysinfo.h> 

//for sched_setaffinity
#include <sched.h>

const int port = 0x1234;
uint64_t dataCount = 0;
static volatile uint8_t loopControl;
static volatile uint8_t ctrlCCount;
static volatile uint8_t printData;

struct timespec lastTimeSpec;

uint64_t sum;

//Signal handler to catch Ctrl-C
static void signalHandler(int signal)
{
  if(signal == SIGINT)
    {
      printf("Shutting down client\n");
      //Cause the main loop to end
      loopControl = 0x0;
      //Keep track of ctrl-Cs 
      ctrlCCount++;
      //We have taken away the right of every linux user to kill our program with Ctrl-C
      //If something is going wrong, we should accept their wishes after several Ctrl-Cs and kill our program
      if(ctrlCCount > 2)
	{
	  printf("Next Ctrl-C kills\n");
	  //Stop catching the Ctrl-C signal and let the user kill
	  sigset_t signalMask;              
	  sigemptyset (&signalMask);                        
	  sigaddset (&signalMask, SIGINT);                  
	  //install the new signal mask                     
	  sigprocmask(SIG_UNBLOCK, &signalMask, NULL);
	} 
    }
  else if(signal == SIGALRM)
    {
      //Reset the ctrl-C count 
      ctrlCCount = 0;
      //Print the readout status
      printData = 0x1;
      alarm(10);
    }
}


//A helper function to change the blocking/non-blocking status of a socket              
uint8_t SetSocketBlocking(int fd, uint8_t blocking)       
{               
  if (fd < 0)   
    {           
      return 0x0;                                 
    }           
  int flags = fcntl(fd, F_GETFL, 0);                
  if (flags < 0)
    {           
      return 0x0;                                 
    }           
  flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);                          
  return (fcntl(fd, F_SETFL, flags) == 0) ? 0x1 : 0x0;                               
}               
                
                
                
                
                
                             



int main(int argc, char ** argv)
{
  //==============================================================
  //Setup command line arguments and open files
  //==============================================================
  if(argc < 2)
    {
      printf("Usage: %s IP rawFile\n",argv[0]);
      return -1;
    }
  char * serverName=argv[1];
  int bufferSize =0x8000;//0x400; //1024;// 256;
  uint8_t buffer[bufferSize];

  //Open file for raw data
  int outFileFD = -1;
  if(argc > 2)
    {
      outFileFD = open(argv[2],
		       O_WRONLY | O_CREAT | O_TRUNC,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if(outFileFD < 0)
	{
	  printf("Error opening raw data file %s\n",argv[2]);
	  return -1;
	}      
    }
  //Load a usable value into lastTimeSpec
  clock_gettime(CLOCK_MONOTONIC,
		&lastTimeSpec);
  
                                                
  //==============================================================                      
  //Setup addrinfo                                  
  //==============================================================                      
  //Set hints   
  struct addrinfo addrHints;                        
  memset(&addrHints,0,sizeof(addrHints));           
  addrHints.ai_family = PF_UNSPEC;                  
  addrHints.ai_socktype = SOCK_STREAM;              
  addrHints.ai_protocol = 0;                        
  char portString[7];                               
  snprintf(portString,sizeof(portString),"%d",port);
                
  struct addrinfo * addrInfos = NULL;
  // serverName is char* ip address (e.g. "192.168.1.32") of amc13 10 GbE
  // portString is char[] port number(e.g. "1234") of amc13 10GbE
  // addrHints is structure containing the family, socktype, protocol, ..
  // addrInfos is returned structure containing socket information
  if(0 != getaddrinfo(serverName,           
                      portString,                   
                      &addrHints,                   
                      &addrInfos))                  
    {           
      printf("Error in addr (%d): %s\n",errno,strerror(errno));                               
      return -3;
    }           
                
  //==============================================================                      
  //Loop over returned addrinfo structures until one of them connects                   
  //==============================================================                      
  int socketFD = -1;
  struct addrinfo * serverAddr;
  for(serverAddr = addrInfos; serverAddr != NULL; serverAddr = serverAddr->ai_next)       
    {    
      //build a socket                              
      if(0 > (socketFD = socket(serverAddr->ai_family,                                 
				serverAddr->ai_socktype,                                
				serverAddr->ai_protocol)))                               
        {       
          //We couldn't make a socket for this serverAddr                               
          socketFD = -1;                            
          continue;                                 
        }       
      //connect to the addr using the socket        
      if(0 > connect(socketFD,                     
                      serverAddr->ai_addr,          
                      serverAddr->ai_addrlen))      
        {       
          //This addr didn't work                   
          close(socketFD);                          
          socketFD = -1;                            
          continue;                                 
        }       
      //Everything worked, lets get going.          
      break;    
    }           
                
  //Check that the connection worked                
  if(socketFD == -1)                                
    {           
      printf("Could not connect to %s %s\n",        
             serverName,                    
             portString);                           
      return -1;
    }                              



  //==============================================================                      
  //We have a working connection!                   
  //==============================================================                      
  //Set socket to non-blocking so we are sure a race condition won't screw us           
  if(!SetSocketBlocking(socketFD,0x0))            
    {           
      printf("Error in socket non-blocking\n");     
      close(socketFD);
      return -6;
    }           

  // tg quick test
  // if(!SetSocketBlocking(socketFD,0x1))            
  //  {           
  //    printf("Error in socket non-blocking\n");     
  //    close(socketFD);
  //    return -6;
  //  }           
  // int readSize = read(socketFD,buffer,bufferSize);
  // printf("tg test, readSize %d, errno %d\n", readSize, errno);
  // tg end quick test 


  //==============================================================                      
  //Setup signal handler                            
  //==============================================================                      
  sigset_t signalMask,emptySignalMask;              
  struct sigaction sSigAction;                      
  memset (&sSigAction, 0, sizeof(sSigAction));      
                
                
  //Setup a signal handler & sigmask to capture CTRL-C                                  
  sSigAction.sa_handler = signalHandler;            
  if ( sigaction(SIGINT, &sSigAction, NULL)  < 0 )  
    {           
      printf("Error in sigaction (%d): %s\n",errno,strerror(errno));                    
      return -2;
    }           
  if ( sigaction(SIGALRM, &sSigAction, NULL)  < 0 )  
    {           
      printf("Error in sigaction (%d): %s\n",errno,strerror(errno));                    
      return -2;
    }           
  //Set up the signal mask for during the 
  sigfillset(&signalMask);
  emptySignalMask = signalMask;
  sigdelset(&signalMask,SIGINT);
  sigdelset(&signalMask,SIGALRM);
                
  //install the new signal mask                     
  if (sigprocmask(SIG_SETMASK, &emptySignalMask,NULL) < 0)                        
    {           
      printf("Error in sigprocmask (%d): %s\n",errno,strerror(errno));                  
      return -3;
    }           
                                                            
  //==============================================================                      
  //Setup select
  //==============================================================              
  fd_set mask;
  FD_ZERO(&mask);
  FD_SET(socketFD,&mask);
  int maxFDp1 = socketFD+1;

  //==============================================================
  //Set core affinity
  //==============================================================
  //  get_nprocs();
  cpu_set_t maskCPU;
  CPU_ZERO(&maskCPU);
  CPU_SET(10,&maskCPU); //11 is where the irqs are
  if( sched_setaffinity(0,
			sizeof(maskCPU),
			&maskCPU
			) < 0)
    {
      perror("Scheduler set affinity");
      return -1;
    }

  //==============================================================
  //Main loop timer
  //==============================================================
  struct timespec timeout;  
  timeout.tv_sec = 2.0;  //Set select timout to 2 seconds
  timeout.tv_nsec = 0;

  printf("Starting run\n");
  loopControl = 0x1;
  int loopCounter = 0;

  alarm(10); // Schedule another alarm for 10 seconds from now

  struct timeval tthis, tlast; // time structures declare, initialize
  long int dt_s = 0, dt_us = 0;
  bool firstblock = 1;
  gettimeofday( &tthis, NULL);
  gettimeofday( &tlast, NULL);

  while(loopControl)
    {
      loopCounter++;
      //copy mask to new variable because select modifies it.
      fd_set readMask = mask;

      //Wait on select for data from a client, or ctrl-C
      //pselect is used to properly handle a race conditions between 
      //a signal handler and select
      int selectReturn = pselect(maxFDp1,
				 &readMask,NULL,NULL,
				 &timeout,&signalMask);

      //==============================================================
      // Handle select errors
      //==============================================================
      if(selectReturn < 0)
	{

	  //Check for signal
	  if(errno == EINTR)
	    {
	      //printf("Select interrupted by signal\n");	      
	    }
	  //check for timeout
	  else if (errno == EINVAL)
	    {
	      printf("Bad arguments to pselect\n");
	      loopControl = 0x0;
	    }
	  //Check for a bad file descriptor in the select mask
	  else if (errno == EBADF)
	    {
	      printf("We have a bad FD in the select mask.\n");
	      printf("This is fatal for now(fix me)");
	      loopControl = 0x0;
	    }
	  //Oh noes!
	  else
	    {
	      printf("PSelect error (%d): %s\n",errno,strerror(errno));
	      loopControl = 0x0;
	    }
	}
      //==============================================================
      // Handle select's active FDs
      //==============================================================
      else if( selectReturn > 0)
	{
	  if(FD_ISSET(socketFD,&readMask))
	    {

	      // get start time
              if (firstblock) {
		gettimeofday( &tlast, NULL);
		printf(" first block %d \n",firstblock);
		firstblock = 0;
	      }
              
	      int readSize = read(socketFD,buffer,bufferSize);
	      //printf("read: request %d, got %d \n", bufferSize, readSize);

	      // get end time
	      if (!firstblock){
		gettimeofday( &tthis, NULL);
		dt_s = tthis.tv_sec - tlast.tv_sec;
		//dt2_s -= trigger_info.time_tcp_finish_data_read_s;
		dt_us = tthis.tv_usec - tlast.tv_usec;
                if (dt_s > 0.0){
		  dt_us += 1.e6*dt_s;
                  dt_s = 0;
		}
		//dt2_us -= trigger_info.time_tcp_finish_data_read_us;
              }

              //if ( loopCounter%10000 == 0){
	      //uint64_t * tmp64 = (uint64_t*) buffer;
	      //printf("read size from socket read 0x%08x 0x%08x  0x%08x  0x%08x  0x%08x \n", readSize, 
	      //     (int)be64toh(tmp64[0]), (int)be64toh(tmp64[1]), (int)be64toh(tmp64[2]), (int)be64toh(tmp64[3]) );
	      //}	
	      if(readSize < 0)
		{
		  printf("Error in reading data when there should be data (disconnect?)\n");
		  loopControl = 0x0;
		}
	      //Write out data if there is a raw file specified
	      else if(readSize >= 0)
		{
		  if(outFileFD > 0)
		    {
		      int wordReadSize = readSize/sizeof(uint64_t);
		      int iWord;
		      uint64_t * buf64 = (uint64_t*) buffer;
		      for(iWord = 0; iWord < wordReadSize;iWord++)
			{
			  sum += buf64[iWord] = be64toh(buf64[iWord]);
			}
		      write(outFileFD,buffer,readSize);
		    }	       
		  else
		    {
		      int iWord;
		      for(iWord = 0; iWord < readSize;iWord++)
			{
			  sum += buffer[iWord];
			}
		    }
		  dataCount += readSize;
		}
	    }
	}
      else
	{
	  printf(" dt = %li s %li us\n", dt_s, dt_us);
          firstblock = 1;
	  printf("Waiting...\n");
	}

      //If our alarm said to print this. 
      if(printData)
	{
	  struct timespec currentTimeSpec;
	  if(clock_gettime(CLOCK_MONOTONIC,
			   &currentTimeSpec) != 0)
	    {
	      perror("Get Current time");
	    }
	  else
	    {
	      double timeDifference = (currentTimeSpec.tv_sec - lastTimeSpec.tv_sec);
	      timeDifference += 1E-9 * (currentTimeSpec.tv_nsec - lastTimeSpec.tv_nsec);
	      double dataRate = (dataCount/timeDifference) * (1.0/(1024.0*1024.0));
	      printf("%d: Data rate %f MBps (%"PRIu64" bytes : running sum=%"PRIu64")\n",(int)time(NULL),
		     dataRate,dataCount,sum);
	      printData = 0x0;
	      dataCount = 0;
	      clock_gettime(CLOCK_MONOTONIC,
			    &lastTimeSpec);
	    }
	}      
    }
  //Close output file
  close(outFileFD);
  
  //Set socket back to blocking
  SetSocketBlocking(socketFD,0x1);
  //Close socket
  close(socketFD);

  return 0;
}
  



