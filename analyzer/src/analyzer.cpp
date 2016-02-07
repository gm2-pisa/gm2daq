/********************************************************************\

  Name:         analyzer.c
  Created by:   Stefan Ritt

  Contents:     System part of Analyzer code for sample experiment

  $Id: analyzer.c 4732 2010-06-02 09:41:08Z ritt $

\********************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <time.h>

/* midas includes */
#include "midas.h"
#include "rmana.h"

/* cernlib includes */
#ifdef OS_WINNT
#define VISUAL_CPLUSPLUS
#endif
#ifdef __linux__
#define f2cFortran
#endif

/*-- Globals -------------------------------------------------------*/

/* The analyzer name (client name) as seen by other MIDAS clients   */
/* char *analyzer_name = "OfflineAnalyzer"; */
char *analyzer_name = "Analyzer";

/* analyzer_loop is called with this interval in ms (0 to disable)  */
INT analyzer_loop_period = 0;

/* default ODB size */
INT odb_size = DEFAULT_ODB_SIZE;

/* ODB structures */
RUNINFO runinfo;
/*-- Module declarations -------------------------------------------*/

#include "MODULES.h"

/*-- Bank definitions ----------------------------------------------*/


/*-- Event request list --------------------------------------------*/

ANALYZE_REQUEST analyze_request[] = {
   {
     "Online",                    /* equipment name */
     {
       1,                         /* event ID */
       TRIGGER_ALL,               /* trigger mask */
       //GET_ALL,                   /* get events without blocking producer */
       GET_NONBLOCKING,                   /* get events without blocking producer */
       "SYSTEM",                  /* event buffer */
       TRUE,                      /* enabled */
       "", 
       "",
     },
     NULL,                       /* analyzer routine */
     offline_modules,            /* module list */
     //ana_trigger_bank_list,      /* bank list */
     NULL,
   },
   
   {""},
};

/*-- Analyzer Init -------------------------------------------------*/

INT analyzer_init()
{
   HNDLE hDB, hKey;
   char str[80];

   RUNINFO_STR(runinfo_str);

   /* open ODB structures */
   cm_get_experiment_database(&hDB, NULL);
   db_create_record(hDB, 0, "/Runinfo", strcomb((const char **)runinfo_str));
   db_find_key(hDB, 0, "/Runinfo", &hKey);
   if (db_open_record(hDB, hKey, &runinfo, sizeof(runinfo), MODE_READ, NULL, NULL) !=
       DB_SUCCESS) {
      cm_msg(MERROR, "analyzer_init", "Cannot open \"/Runinfo\" tree in ODB");
      return 0;
   }


   return SUCCESS;
}

/*-- Analyzer Exit -------------------------------------------------*/

INT analyzer_exit()
{
   return CM_SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT ana_begin_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT ana_end_of_run(INT run_number, char *error)
{
   FILE *f;
   time_t now;
   char str[256];
   int size;
   double n;
   HNDLE hDB;
   BOOL flag;

   cm_get_experiment_database(&hDB, NULL);

   /* update run log if run was written and running online */

   return CM_SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT ana_pause_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/

INT ana_resume_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Analyzer Loop -------------------------------------------------*/

INT analyzer_loop()
{
   return CM_SUCCESS;
}

/*------------------------------------------------------------------*/
