/********************************************************************\

  Name:         rmana.cpp
  Created by:   Vlarimir Tishchenko based on 
                Stefan Ritt's mana.c

  Contents:     The system part of the MIDAS analyzer. Has to be
                linked with analyze.c to form a complete analyzer

\********************************************************************/

#include <assert.h>
#include "midas.h"
#include "msystem.h"
#include "hardware.h"
#include "rmana.h"

#include "mdsupport.h"

/*------------------------------------------------------------------*/

/* cernlib includes */
#ifdef OS_WINNT
#define VISUAL_CPLUSPLUS
#endif

#ifdef OS_LINUX
#define f2cFortran
#endif

ANA_OUTPUT_INFO out_info;

/*------------------------------------------------------------------*/

#ifdef USE_ROOT

#undef abs

#include <assert.h>
#include <TApplication.h>
#include <TKey.h>
#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TSocket.h>
#include <TServerSocket.h>
#include <TMessage.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TFolder.h>
#include <TRint.h>
//#include <TCutG.h>
#include <TGraph.h>
#include <TGraphErrors.h>

#ifdef OS_LINUX
#include <TThread.h>
#endif

/* Our own ROOT global objects */

TApplication *manaApp;
TFolder      *gManaHistosFolder = NULL;      // Container for all histograms
TFile        *gManaOutputFile = NULL;        // MIDAS output file

#endif                          /* USE_ROOT */

/*------------------------------------------------------------------*/

/* items defined in analyzer.c */
extern char *analyzer_name;
extern INT analyzer_loop_period;
extern INT analyzer_init(void);
extern INT analyzer_exit(void);
extern INT analyzer_loop(void);
extern INT ana_begin_of_run(INT run_number, char *error);
extern INT ana_end_of_run(INT run_number, char *error);
extern INT ana_pause_run(INT run_number, char *error);
extern INT ana_resume_run(INT run_number, char *error);
extern INT odb_size;

/*---- globals -----------------------------------------------------*/

/* ODB handle */
HNDLE hDB;

/* run number */
DWORD current_run_number;

/* analyze_request defined in analyze.c or anasys.c */
extern ANALYZE_REQUEST analyze_request[];

/* maximum extended event size */
#define EXT_EVENT_SIZE (2*(MAX_EVENT_SIZE+sizeof(EVENT_HEADER)))

/* command line parameters */
static struct {
   INT online;
   char host_name[HOST_NAME_LENGTH];
   char exp_name[NAME_LENGTH];
   char input_file_name[10][256];
   char output_file_name[256];
   INT run_number[2];
   DWORD n[4];
   BOOL filter;
   char config_file_name[10][256];
   char param[10][256];
   char protect[10][256];
   BOOL debug;
   BOOL verbose;
   BOOL quiet;
   BOOL no_load;
   BOOL daemon;
   INT n_task;
   INT root_port;
   BOOL start_rint;
} clp;

static struct {
   char flag_char;
   char description[1000];
   void *data;
   INT type;
   INT n;
   INT index;
} clp_descrip[] = {

   {
   'c', "<filename1>   Configuration file name(s). May contain a '%05d' to be\n\
     <filename2>   replaced by the run number. Up to ten files can be\n\
         ...       specified in one \"-c\" statement", clp.config_file_name, TID_STRING, 10}, {
   'd', "              Debug flag when started the analyzer fron a debugger.\n\
                   Prevents the system to kill the analyzer when the\n\
                   debugger stops at a breakpoint", &clp.debug, TID_BOOL, 0}, {
   'D', "              Start analyzer as a daemon in the background (UNIX only).",
          &clp.daemon, TID_BOOL, 0}, {
   'e', "<experiment>  MIDAS experiment to connect to", clp.exp_name, TID_STRING, 1}, {
   'f', "              Filter mode. Write original events to output file\n\
                   only if analyzer accepts them (doesn't return ANA_SKIP).\n", &clp.filter, TID_BOOL, 0}, {
   'h', "<hostname>    MIDAS host to connect to when running the analyzer online",
          clp.host_name, TID_STRING, 1}, {
   'i', "<filename1>   Input file name. May contain a '%05d' to be replaced by\n\
     <filename2>   the run number. Up to ten input files can be specified\n\
         ...       in one \"-i\" statement", clp.input_file_name, TID_STRING, 10}, {
   'l', "              If set, don't load histos from last histo file when\n\
                   running online.", &clp.no_load, TID_BOOL, 0}, {
   'n', "<count>       Analyze only \"count\" events.\n\
     <first> <last>\n\
                   Analyze only events from \"first\" to \"last\".\n\
     <first> <last> <n>\n\
                   Analyze every n-th event from \"first\" to \"last\".", clp.n, TID_INT, 4}, {
   'o', "<filename>    Output file name. Extension may be .mid (MIDAS binary),\n\
                   .asc (ASCII) or .rz (HBOOK). If the name contains a '%05d',\n\
                   one output file is generated for each run. Use \"OFLN\" as\n\
                   output file name to creaate a HBOOK shared memory instead\n\
                   of a file.", clp.output_file_name, TID_STRING, 1}, {
   'p', "<param=value> Set individual parameters to a specific value.\n\
                   Overrides any setting in configuration files", clp.param, TID_STRING, 10}, {
   'P', "<ODB tree>    Protect an ODB subtree from being overwritten\n\
                   with the online data when ODB gets loaded from .mid file", clp.protect, TID_STRING, 10}, {
   'q', "              Quiet flag. If set, don't display run progress in\n\
                   offline mode.", &clp.quiet, TID_BOOL, 0}, {
   'r', "<range>       Range of run numbers to analyzer like \"-r 120 125\"\n\
                   to analyze runs 120 to 125 (inclusive). The \"-r\"\n\
                   flag must be used with a '%05d' in the input file name.", clp.run_number, TID_INT, 2},
#ifdef USE_ROOT
   {
   's', "<port>        Start ROOT histo server under <port>. If port==0, don't start server.", &clp.root_port, TID_INT, 1}, {
   'R', "              Start ROOT interpreter after analysis has finished.",
          &clp.start_rint, TID_BOOL, 0},
#endif
   {
   'v', "              Verbose output.", &clp.verbose, TID_BOOL, 0}, {
   0}
};

FILE *out_file;
#ifdef HAVE_ZLIB
BOOL out_gzip;
#endif
INT out_format;
BOOL out_append;

void update_stats();
void odb_load(EVENT_HEADER * pevent);

/*---- ODB records -------------------------------------------------*/

#define ANALYZER_REQUEST_STR "\
Event ID = INT : 0\n\
Trigger mask = INT : -1\n\
Sampling type = INT : 1\n\
Buffer = STRING : [32] SYSTEM\n\
Enabled = BOOL : 1\n\
Client name = STRING : [32] \n\
Host = STRING : [32] \n\
"

#define ANALYZER_STATS_STR "\
Events received = DOUBLE : 0\n\
Events per sec. = DOUBLE : 0\n\
Events written = DOUBLE : 0\n\
"

/*-- interprete command line parameters ----------------------------*/

INT getparam(int argc, char **argv)
{
   INT index, i, j, size;

   /* parse command line parameters */
   for (index = 1; index < argc;) {
      /* search flag in parameter description */
      if (argv[index][0] == '-') {
         for (j = 0; clp_descrip[j].flag_char; j++)
            if (argv[index][1] == clp_descrip[j].flag_char)
               break;

         if (!clp_descrip[j].flag_char)
            goto usage;

         if (clp_descrip[j].n > 0 && index >= argc - 1)
            goto usage;
         index++;

         if (clp_descrip[j].type == TID_BOOL) {
            *((BOOL *) clp_descrip[j].data) = TRUE;
            continue;
         }

         do {
            if (clp_descrip[j].type == TID_STRING)
               strcpy((char *) clp_descrip[j].data + clp_descrip[j].index * 256,
                      argv[index]);
            else
               db_sscanf(argv[index], clp_descrip[j].data, &size, clp_descrip[j].index,
                         clp_descrip[j].type);

            if (clp_descrip[j].n > 1)
               clp_descrip[j].index++;

            if (clp_descrip[j].index > clp_descrip[j].n) {
               printf("Note more than %d options possible for flag -%c\n",
                      clp_descrip[j].n, clp_descrip[j].flag_char);
               return 0;
            }

            index++;

         } while (index < argc && argv[index][0] != '-');

      } else
         goto usage;
   }

   return SUCCESS;

 usage:

   printf("usage: analyzer [options]\n\n");
   printf("valid options are:\n");
   for (i = 0; clp_descrip[i].flag_char; i++)
      printf("  -%c %s\n", clp_descrip[i].flag_char, clp_descrip[i].description);

   return 0;
}

/*-- add </logger/data dir> before filename ------------------------*/

void add_data_dir(char *result, char *file)
{
   HNDLE hDB, hkey;
   char str[256];
   int size;

   cm_get_experiment_database(&hDB, NULL);
   db_find_key(hDB, 0, "/Logger/Data dir", &hkey);

   if (hkey) {
      size = sizeof(str);
      db_get_data(hDB, hkey, str, &size, TID_STRING);
      if (str[strlen(str) - 1] != DIR_SEPARATOR)
         strcat(str, DIR_SEPARATOR_STR);
      strcat(str, file);
      strcpy(result, str);
   } else
      strcpy(result, file);
}

/*-- db_get_event_definition ---------------------------------------*/

typedef struct {
   short int event_id;
   int type;
   WORD format;
   HNDLE hDefKey;
   BOOL disabled;
} EVENT_DEF;

EVENT_DEF *db_get_event_definition(short int event_id)
{
   INT i, index, status, size, type;
   char str[80];
   HNDLE hKey, hKeyRoot;
   WORD id;
   static EVENT_DEF *event_def = NULL;
   static int n_cache = 0;

   /* search free cache entry */
   for (index = 0; index < n_cache; index++)
      if (event_def[index].event_id == event_id)
         return &event_def[index];

   /* If we get here, we have an undefined ID;
      allocate memory for it, zero it, then cache the ODB data */
   n_cache = index + 1;

   event_def = (EVENT_DEF *) realloc(event_def, (n_cache) * sizeof(EVENT_DEF));
   assert(event_def);

   memset(&event_def[index], 0, sizeof(EVENT_DEF));

   /* check for system events */
   if (event_id < 0) {
      event_def[index].event_id = event_id;
      event_def[index].format = FORMAT_ASCII;
      event_def[index].hDefKey = 0;
      event_def[index].disabled = FALSE;
      return &event_def[index];
   }

   status = db_find_key(hDB, 0, "/equipment", &hKeyRoot);
   if (status != DB_SUCCESS) {
      cm_msg(MERROR, "db_get_event_definition", "cannot find /equipment entry in ODB");
      return NULL;
   }

   for (i = 0;; i++) {
      /* search for equipment with specific name */
      status = db_enum_key(hDB, hKeyRoot, i, &hKey);
      if (status == DB_NO_MORE_SUBKEYS) {
         sprintf(str, "Cannot find event id %d under /equipment", event_id);
         cm_msg(MERROR, "db_get_event_definition", str);
         return NULL;
      }

      size = sizeof(id);
      status = db_get_value(hDB, hKey, "Common/Event ID", &id, &size, TID_WORD, TRUE);
      if (status != DB_SUCCESS)
         continue;

      size = sizeof(type);
      status = db_get_value(hDB, hKey, "Common/Type", &type, &size, TID_INT, TRUE);
      if (status != DB_SUCCESS)
         continue;

      if (id == event_id) {
         /* set cache entry */
         event_def[index].event_id = id;
         event_def[index].type = type;

         size = sizeof(str);
         str[0] = 0;
         db_get_value(hDB, hKey, "Common/Format", str, &size, TID_STRING, TRUE);

         if (equal_ustring(str, "Fixed"))
            event_def[index].format = FORMAT_FIXED;
         else if (equal_ustring(str, "ASCII"))
            event_def[index].format = FORMAT_ASCII;
         else if (equal_ustring(str, "MIDAS"))
            event_def[index].format = FORMAT_MIDAS;
         else if (equal_ustring(str, "DUMP"))
            event_def[index].format = FORMAT_DUMP;
         else {
            cm_msg(MERROR, "db_get_event_definition", "unknown data format");
            event_def[index].event_id = 0;
            return NULL;
         }

         db_find_key(hDB, hKey, "Variables", &event_def[index].hDefKey);
         return &event_def[index];
      }
   }
}

/*-- load parameters specified on command line ---------------------*/

INT load_parameters(INT run_number)
{
   INT i, size, index, status;
   HNDLE hkey;
   char file_name[256], str[80], value_string[80], param_string[80];
   char data[32];
   KEY key;

   /* loop over configutation file names */
   for (i = 0; clp.config_file_name[i][0] && i < 10; i++) {
      if (strchr(clp.config_file_name[i], '%') != NULL)
         sprintf(file_name, clp.config_file_name[i], run_number);
      else
         strcpy(file_name, clp.config_file_name[i]);

      /* load file under "/" */
      if (db_load(hDB, 0, file_name, FALSE) == DB_SUCCESS)
         printf("Configuration file \"%s\" loaded\n", file_name);
   }

   /* loop over parameters */
   for (i = 0; clp.param[i][0] && i < 10; i++) {
      if (strchr(clp.param[i], '=') == NULL) {
         printf("Error: parameter %s contains no value\n", clp.param[i]);
      } else {
         strcpy(value_string, strchr(clp.param[i], '=') + 1);
         strcpy(param_string, clp.param[i]);
         *strchr(param_string, '=') = 0;

         index = 0;
         if (strchr(param_string, '[') != NULL) {
            index = atoi(strchr(param_string, '[') + 1);
            *strchr(param_string, '[') = 0;
         }

         if (param_string[0] == '/')
            strcpy(str, param_string);
         else
            sprintf(str, "/%s/Parameters/%s", analyzer_name, param_string);
         db_find_key(hDB, 0, str, &hkey);
         if (hkey == 0) {
            printf("Error: cannot find parameter %s in ODB\n", str);
         } else {
            db_get_key(hDB, hkey, &key);
            db_sscanf(value_string, data, &size, 0, key.type);

            status = db_set_data_index(hDB, hkey, data, size, index, key.type);
            if (status == DB_SUCCESS)
               printf("Parameter %s changed to %s\n", str, value_string);
            else
               printf("Cannot change parameter %s\n", str);
         }
      }
   }

   /* let parameter changes propagate to modules */
   cm_yield(0);

   return SUCCESS;
}


void banks_changed(INT hDB, INT hKey, void *info)
{
   char str[80];
   HNDLE hkey;

   /* close previously opened hot link */
   sprintf(str, "/%s/Bank switches", analyzer_name);
   db_find_key(hDB, 0, str, &hkey);
   db_close_record(hDB, hkey);

}


/*-- book TTree from ODB bank structures ---------------------------*/

#ifdef USE_ROOT

/*-- root histogram routines ---------------------------------------*/

// Save all objects from given directory into given file
INT SaveRootHistograms(TFolder * folder, const char *filename)
{
   TDirectory *savedir = gDirectory;
   TFile *outf = new TFile(filename, "RECREATE", "Midas Analyzer Histograms");
   if (outf == 0) {
      cm_msg(MERROR, "SaveRootHistograms", "Cannot create output file %s", filename);
      return 0;
   }

   outf->cd();
   folder->Write();
   outf->Close();
   delete outf;
   // restore current directory
   savedir->cd();
   return SUCCESS;
}

/*------------------------------------------------------------------*/

// copy object from a last folder, to an online folder,
// and call itself to handle subfolders
void copy_from_last(TFolder * lastFolder, TFolder * onlineFolder)
{

   TIter next(lastFolder->GetListOfFolders());
   while (TObject * obj = next()) {
      const char *name = obj->GetName();

      if (obj->InheritsFrom("TFolder")) {

         TFolder *onlineSubfolder = (TFolder *) onlineFolder->FindObject(name);
         if (onlineSubfolder)
            copy_from_last((TFolder *) obj, onlineSubfolder);

      } else if (obj->InheritsFrom("TH1")) {

         // still don't know how to do TH1s

      } 
   }
   return;
}

/*------------------------------------------------------------------*/

// Load all objects from given file into given directory
INT LoadRootHistograms(TFolder * folder, const char *filename)
{
   TFile *inf = TFile::Open(filename, "READ");
   if (inf == NULL)
      printf("Error: File \"%s\" not found\n", filename);
   else {

      TFolder *lastHistos = (TFolder *) inf->Get("histos");
      if (lastHistos) {
         // copy histos to online folder
         copy_from_last(lastHistos, folder);
         inf->Close();
      }
   }
   return SUCCESS;
}

/*------------------------------------------------------------------*/

// Clear all TH1 objects in the given directory,
// and it's subdirectories
INT ClearRootHistograms(TFolder * folder)
{
   TIter next(folder->GetListOfFolders());
   while (TObject * obj = next())
     if (obj->InheritsFrom("TH1") )
       ((TH1 *) obj)->Reset();
     else if ( obj->InheritsFrom("TGraph") )
       ((TGraph *) obj)->Set(0);
     else if ( obj->InheritsFrom("TGraphErrors") )
       ((TGraphErrors *) obj)->Set(0);
     else if (obj->InheritsFrom("TFolder"))
       ClearRootHistograms((TFolder *) obj);
   return SUCCESS;
}

/*------------------------------------------------------------------*/

INT CloseRootOutputFile()
{
   int i;

   // ensure that we do have an open file
   assert(gManaOutputFile != NULL);

   // save the histograms
   gManaOutputFile->cd();
   gManaHistosFolder->Write();

   // close the output file
   gManaOutputFile->Write();
   gManaOutputFile->Close();
   delete gManaOutputFile;
   gManaOutputFile = NULL;

   // go to ROOT root directory
   gROOT->cd();

   return SUCCESS;
}

#endif                          /* USE_ROOT */

/*-- analyzer init routine -----------------------------------------*/

INT mana_init()
{
   ANA_MODULE **module;
   INT i, j, status, size;
   HNDLE hkey;
   char str[256], block_name[32];
   BANK_LIST *bank_list;
   double dummy;

   sprintf(str, "/%s/Output", analyzer_name);
   db_find_key(hDB, 0, str, &hkey);

   if (clp.online) {
      status =
          db_open_record(hDB, hkey, &out_info, sizeof(out_info), MODE_READ, NULL, NULL);
      if (status != DB_SUCCESS) {
         cm_msg(MERROR, "bor", "Cannot read output info record");
         return 0;
      }
   }

   /* create ODB structures for banks */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      bank_list = analyze_request[i].bank_list;

      if (bank_list == NULL)
         continue;

      for (; bank_list->name[0]; bank_list++) {
         strncpy(block_name, bank_list->name, 4);
         block_name[4] = 0;

         if (bank_list->type == TID_STRUCT) {
            sprintf(str, "/Equipment/%s/Variables/%s", analyze_request[i].event_name,
                    block_name);
            db_check_record(hDB, 0, str, strcomb((const char **)bank_list->init_str), TRUE);
            db_find_key(hDB, 0, str, &hkey);
            bank_list->def_key = hkey;
         } else {
            sprintf(str, "/Equipment/%s/Variables/%s", analyze_request[i].event_name,
                    block_name);
            status = db_find_key(hDB, 0, str, &hkey);
            if (status != DB_SUCCESS) {
               dummy = 0;
               db_set_value(hDB, 0, str, &dummy, rpc_tid_size(bank_list->type), 1,
                            bank_list->type);
            }
            bank_list->def_key = hkey;
         }
      }
   }

   /* create ODB structures for fixed events */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      if (analyze_request[i].init_string) {
         sprintf(str, "/Equipment/%s/Variables", analyze_request[i].event_name);
         db_check_record(hDB, 0, str, strcomb((const char **)analyze_request[i].init_string), TRUE);
      }
   }

   /* call main analyzer init routine */
   status = analyzer_init();
   if (status != SUCCESS)
      return status;

   /* initialize modules */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      module = analyze_request[i].ana_module;
      for (j = 0; module != NULL && module[j] != NULL; j++) {

         /* copy module enabled flag to ana_module */
         sprintf(str, "/%s/Module switches/%s", analyzer_name, module[j]->name);
         module[j]->enabled = TRUE;
         size = sizeof(BOOL);
         db_get_value(hDB, 0, str, &module[j]->enabled, &size, TID_BOOL, TRUE);

         if (module[j]->init != NULL && module[j]->enabled) {

#ifdef USE_ROOT
            /* create histo subfolder for module */
            sprintf(str, "Histos for module %s", module[j]->name);
            module[j]->histo_folder = (TFolder *) gROOT->FindObjectAny(module[j]->name);
            if (!module[j]->histo_folder)
	      {
		
		module[j]->histo_folder =
		  gManaHistosFolder->AddFolder(module[j]->name, str);
		((TFolder*)module[j]->histo_folder)->SetOwner(kTRUE);
	      }
            else if (strcmp(((TObject *) module[j]->histo_folder)->ClassName(), "TFolder")
                     != 0) {
               cm_msg(MERROR, "mana_init",
                      "Fatal error: ROOT Object \"%s\" of class \"%s\" exists but it is not a TFolder, exiting!",
                      module[j]->name,
                      ((TObject *) module[j]->histo_folder)->ClassName());
               exit(1);
            }
	    gROOT->cd();
#endif
            module[j]->init();
#ifdef USE_ROOT
	    // *** Move all objects from the ROOT global memory into a folder ***
	    TFolder *folder = (TFolder*) module[j]->histo_folder;
	    TFolder *root = (TFolder*)gROOT->GetRootFolder()->GetListOfFolders()->FindObject("ROOT Memory");
#if 0
	    root->ls();
#endif
	    TIter next(root->GetListOfFolders());
	    while ( TObject *obj = (TObject*) next() )
	      {
		//printf("Object: [%s]\n",obj->GetName());
		folder->Add(obj);
		root->Remove(obj);
	      }
#endif
         }
      }
   }

   return SUCCESS;
}

/*-- exit routine --------------------------------------------------*/

INT mana_exit()
{
   ANA_MODULE **module;
   INT i, j;

   /* call exit routines from modules */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      module = analyze_request[i].ana_module;
      for (j = 0; module != NULL && module[j] != NULL; j++)
         if (module[j]->exit != NULL && module[j]->enabled) {
            module[j]->exit();
         }
   }

   /* call main analyzer exit routine */
   return analyzer_exit();
}

/*-- BOR routine ---------------------------------------------------*/

INT bor(INT run_number, char *error)
{
   ANA_MODULE **module;
   INT i, j, size;
   char str[256], file_name[256], *ext_str;
   BANK_LIST *bank_list;

   /* load parameters */
   load_parameters(run_number);

   for (i = 0; analyze_request[i].event_name[0]; i++) {
      /* copy output flag from ODB to bank_list */
      bank_list = analyze_request[i].bank_list;

      if (bank_list != NULL)
         for (; bank_list->name[0]; bank_list++) {
            sprintf(str, "/%s/Bank switches/%s", analyzer_name, bank_list->name);
            bank_list->output_flag = FALSE;
            size = sizeof(DWORD);
            db_get_value(hDB, 0, str, &bank_list->output_flag, &size, TID_DWORD, TRUE);
         }

      /* copy module enabled flag to ana_module */
      module = analyze_request[i].ana_module;
      for (j = 0; module != NULL && module[j] != NULL; j++) {
         sprintf(str, "/%s/Module switches/%s", analyzer_name, module[j]->name);
         module[j]->enabled = TRUE;
         size = sizeof(BOOL);
         db_get_value(hDB, 0, str, &module[j]->enabled, &size, TID_BOOL, TRUE);
      }
   }

   /* clear histos, N-tuples and tests */
   if (clp.online && out_info.clear_histos) {

#ifdef USE_ROOT
      /* clear histos */
      if (clp.online && out_info.clear_histos)
         ClearRootHistograms(gManaHistosFolder);
#endif                          /* USE_ROOT */

   }

   /* open output file if not already open (append mode) and in offline mode */
   if (!clp.online && out_file == NULL
       && !equal_ustring(clp.output_file_name, "OFLN")) {
      if (out_info.filename[0]) {
         strcpy(str, out_info.filename);
         if (strchr(str, '%') != NULL)
            sprintf(file_name, str, run_number);
         else
            strcpy(file_name, str);

         /* check output file extension */
#ifdef HAVE_ZLIB
         out_gzip = FALSE;
#endif
         if (strchr(file_name, '.')) {
            ext_str = file_name + strlen(file_name) - 1;
            while (*ext_str != '.')
               ext_str--;

            if (strncmp(ext_str, ".gz", 3) == 0) {
#ifdef HAVE_ZLIB
               out_gzip = TRUE;
               ext_str--;
               while (*ext_str != '.' && ext_str > file_name)
                  ext_str--;
#else
               strcpy(error,
                      ".gz extension not possible because zlib support is not compiled in.\n");
               cm_msg(MERROR, "bor", error);
               return 0;
#endif
            }

            if (strncmp(ext_str, ".asc", 4) == 0)
               out_format = FORMAT_ASCII;
            else if (strncmp(ext_str, ".mid", 4) == 0)
               out_format = FORMAT_MIDAS;
            else if (strncmp(ext_str, ".rz", 3) == 0)
               out_format = FORMAT_HBOOK;
            else if (strncmp(ext_str, ".root", 5) == 0)
               out_format = FORMAT_ROOT;
            else {
               strcpy(error,
                      "Unknown output data format. Please use file extension .asc, .mid, .rz or .root.\n");
               cm_msg(MERROR, "bor", error);
               return 0;
            }
         } else
            out_format = FORMAT_ASCII;


         /* open output file */
         if (out_format == FORMAT_HBOOK) {
            cm_msg(MERROR, "bor", "HBOOK support is not compiled in");
         }

         else if (out_format == FORMAT_ROOT) {
#ifdef USE_ROOT
            // ensure the output file is closed
            assert(gManaOutputFile == NULL);

            gManaOutputFile =
                new TFile(file_name, "RECREATE", "Midas Analyzer output file");
            if (gManaOutputFile == NULL) {
               sprintf(error, "Cannot open output file %s", out_info.filename);
               cm_msg(MERROR, "bor", error);
               out_file = NULL;
               return 0;
            }
            // make all ROOT objects created by user module bor() functions
            // go into the output file
	    printf("Created output file [%s]\n",file_name);
            gManaOutputFile->cd();

            out_file = (FILE *) 1;
#else
            cm_msg(MERROR, "bor", "ROOT support is not compiled in");
#endif                          /* USE_ROOT */
         }

         else {
#ifdef HAVE_ZLIB
            if (out_gzip)
               out_file = (FILE *) gzopen(file_name, "wb");
            else
#endif
            if (out_format == FORMAT_ASCII)
               out_file = fopen(file_name, "wt");
            else
               out_file = fopen(file_name, "wb");
            if (out_file == NULL) {
               sprintf(error, "Cannot open output file %s", file_name);
               cm_msg(MERROR, "bor", error);
               return 0;
            }
         }
      } else
         out_file = NULL;

   }

   /* if (out_file == NULL) */
   /* save run number */
   current_run_number = run_number;

   /* call bor for modules */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      module = analyze_request[i].ana_module;
      for (j = 0; module != NULL && module[j] != NULL; j++)
         if (module[j]->bor != NULL && module[j]->enabled) {
            module[j]->bor(run_number);
         }
   }

   /* call main analyzer BOR routine */
   return ana_begin_of_run(run_number, error);
}

/*-- EOR routine ---------------------------------------------------*/

INT eor(INT run_number, char *error)
{
   ANA_MODULE **module;
   BANK_LIST *bank_list;
   INT i, j, status;
   char str[256], file_name[256];

   /* call EOR routines modules */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      module = analyze_request[i].ana_module;
      for (j = 0; module != NULL && module[j] != NULL; j++)
         if (module[j]->eor != NULL && module[j]->enabled) {
            module[j]->eor(run_number);
         }
   }

   /* call main analyzer BOR routine */
   status = ana_end_of_run(run_number, error);

   /* save histos if requested */
   if (out_info.histo_dump && clp.online) {
      strcpy(str, out_info.histo_dump_filename);
      if (strchr(str, '%') != NULL)
         sprintf(file_name, str, run_number);
      else
         strcpy(file_name, str);

      add_data_dir(str, file_name);

#ifdef USE_ROOT
      SaveRootHistograms(gManaHistosFolder, str);
#endif                          /* USE_ROOT */
   }

   /* close output file */
   if (out_file && !out_append) {
      if (out_format == FORMAT_HBOOK) {
         cm_msg(MERROR, "eor", "HBOOK support is not compiled in");
      } else if (out_format == FORMAT_ROOT) {
#ifdef USE_ROOT
         CloseRootOutputFile();
#else
         cm_msg(MERROR, "eor", "ROOT support is not compiled in");
#endif                          /* USE_ROOT */
      } else {
#ifdef HAVE_ZLIB
         if (out_gzip)
            gzclose(out_file);
         else
#endif
            fclose(out_file);
      }

      out_file = NULL;

      /* free CWNT buffer */
      for (i = 0; analyze_request[i].event_name[0]; i++) {
         bank_list = analyze_request[i].bank_list;

         if (bank_list == NULL) {
            if (analyze_request[i].addr) {
               free(analyze_request[i].addr);
               analyze_request[i].addr = NULL;
            }
         } else {
            for (; bank_list->name[0]; bank_list++)
               if (bank_list->addr) {
                  free(bank_list->addr);
                  bank_list->addr = NULL;
               }
         }
      }
   }

   return status;
}

/*-- transition callbacks ------------------------------------------*/

/*-- start ---------------------------------------------------------*/

INT tr_start(INT rn, char *error)
{
   INT status, i;

   /* reset counters */
   for (i = 0; analyze_request[i].event_name[0]; i++) {
      analyze_request[i].ar_stats.events_received = 0;
      analyze_request[i].ar_stats.events_per_sec = 0;
      analyze_request[i].ar_stats.events_written = 0;
      analyze_request[i].events_received = 0;
      analyze_request[i].events_written = 0;
   }

   status = bor(rn, error);
   if (status != SUCCESS)
      return status;

   return SUCCESS;
}

/*-- stop ----------------------------------------------------------*/

INT tr_stop(INT rn, char *error)
{
   INT i, status, n_bytes;

   /* wait until all events in buffers are analyzed */

   if (rpc_is_remote())
      while (bm_poll_event(TRUE));
   else
      for (i = 0; analyze_request[i].event_name[0]; i++) {
         do {
            bm_get_buffer_level(analyze_request[i].buffer_handle, &n_bytes);
            if (n_bytes > 0)
               cm_yield(100);
         } while (n_bytes > 0);
      }

   /* update statistics */
   update_stats();

   status = eor(rn, error);
   if (status != SUCCESS)
      return status;

   return CM_SUCCESS;
}

/*-- pause ---------------------------------------------------------*/

INT tr_pause(INT rn, char *error)
{
   INT status;

   status = ana_pause_run(rn, error);
   if (status != CM_SUCCESS)
      return status;

   return CM_SUCCESS;
}

/*-- resume --------------------------------------------------------*/

INT tr_resume(INT rn, char *error)
{
   INT status;

   status = ana_resume_run(rn, error);
   if (status != CM_SUCCESS)
      return status;

   return CM_SUCCESS;
}

/*---- ASCII output ------------------------------------------------*/

#define STR_INC(p,base) { p+=strlen(p); \
                          if (p > base+sizeof(base)) \
                            cm_msg(MERROR, "STR_INC", "ASCII buffer too small"); }


INT write_event_ascii(FILE * file, EVENT_HEADER * pevent, ANALYZE_REQUEST * par)
{
   INT status, size, i, j, count;
   BOOL exclude;
   BANK_HEADER *pbh;
   BANK_LIST *pbl;
   EVENT_DEF *event_def;
   BANK *pbk;
   BANK32 *pbk32;
   void *pdata;
   char *pbuf, name[5], type_name[10];
   LRS1882_DATA *lrs1882;
   LRS1877_DATA *lrs1877;
   LRS1877_HEADER *lrs1877_header;
   HNDLE hKey;
   KEY key;
   char buffer[100000];
   DWORD bkname;
   WORD bktype;

   event_def = db_get_event_definition(pevent->event_id);
   if (event_def == NULL)
      return SS_SUCCESS;

   /* write event header */
   pbuf = buffer;
   name[4] = 0;

   if (pevent->event_id == EVENTID_BOR)
      sprintf(pbuf, "%%ID BOR NR %d\n", (int) pevent->serial_number);
   else if (pevent->event_id == EVENTID_EOR)
      sprintf(pbuf, "%%ID EOR NR %d\n", (int) pevent->serial_number);
   else
      sprintf(pbuf, "%%ID %d TR %d NR %d\n", pevent->event_id, pevent->trigger_mask,
              (int) pevent->serial_number);
   STR_INC(pbuf, buffer);

  /*---- MIDAS format ----------------------------------------------*/
   if (event_def->format == FORMAT_MIDAS) {
      pbh = (BANK_HEADER *) (pevent + 1);
      pbk = NULL;
      pbk32 = NULL;
      do {
         /* scan all banks */
         if (bk_is32(pbh)) {
            size = bk_iterate32(pbh, &pbk32, &pdata);
            if (pbk32 == NULL)
               break;
            bkname = *((DWORD *) pbk32->name);
            bktype = (WORD) pbk32->type;
         } else {
            size = bk_iterate(pbh, &pbk, &pdata);
            if (pbk == NULL)
               break;
            bkname = *((DWORD *) pbk->name);
            bktype = (WORD) pbk->type;
         }

         /* look if bank is in exclude list */
         exclude = FALSE;
         pbl = NULL;
         if (par->bank_list != NULL)
            for (i = 0; par->bank_list[i].name[0]; i++)
               if (*((DWORD *) par->bank_list[i].name) == bkname) {
                  pbl = &par->bank_list[i];
                  exclude = (pbl->output_flag == 0);
                  break;
               }

         if (!exclude) {
            if (rpc_tid_size(bktype & 0xFF))
               size /= rpc_tid_size(bktype & 0xFF);

            lrs1882 = (LRS1882_DATA *) pdata;
            lrs1877 = (LRS1877_DATA *) pdata;

            /* write bank header */
            *((DWORD *) name) = bkname;

            if ((bktype & 0xFF00) == 0)
               strcpy(type_name, rpc_tid_name(bktype & 0xFF));
            else if ((bktype & 0xFF00) == TID_LRS1882)
               strcpy(type_name, "LRS1882");
            else if ((bktype & 0xFF00) == TID_LRS1877)
               strcpy(type_name, "LRS1877");
            else if ((bktype & 0xFF00) == TID_PCOS3)
               strcpy(type_name, "PCOS3");
            else
               strcpy(type_name, "unknown");

            sprintf(pbuf, "BK %s TP %s SZ %d\n", name, type_name, size);
            STR_INC(pbuf, buffer);

            if (bktype == TID_STRUCT) {
               if (pbl == NULL)
                  cm_msg(MERROR, "write_event_ascii", "received unknown bank %s", name);
               else
                  /* write structured bank */
                  for (i = 0;; i++) {
                     status = db_enum_key(hDB, pbl->def_key, i, &hKey);
                     if (status == DB_NO_MORE_SUBKEYS)
                        break;

                     db_get_key(hDB, hKey, &key);
                     sprintf(pbuf, "%s:\n", key.name);
                     STR_INC(pbuf, buffer);

                     /* adjust for alignment */
                     pdata =
                         (void *) VALIGN(pdata,
                                         MIN(ss_get_struct_align(), key.item_size));

                     for (j = 0; j < key.num_values; j++) {
                        db_sprintf(pbuf, pdata, key.item_size, j, key.type);
                        strcat(pbuf, "\n");
                        STR_INC(pbuf, buffer);
                     }

                     /* shift data pointer to next item */
                     pdata = (char *) pdata + key.item_size * key.num_values;
                  }
            } else {
               /* write variable length bank  */
               if ((bktype & 0xFF00) == TID_LRS1877) {
                  for (i = 0; i < size;) {
                     lrs1877_header = (LRS1877_HEADER *) & lrs1877[i];

                     /* print header */
                     sprintf(pbuf, "GA %d BF %d CN %d",
                             lrs1877_header->geo_addr, lrs1877_header->buffer,
                             lrs1877_header->count);
                     strcat(pbuf, "\n");
                     STR_INC(pbuf, buffer);

                     count = lrs1877_header->count;
                     if (count == 0)
                        break;
                     for (j = 1; j < count; j++) {
                        /* print data */
                        sprintf(pbuf, "GA %d CH %02d ED %d DA %1.1lf",
                                lrs1877[i].geo_addr, lrs1877[i + j].channel,
                                lrs1877[i + j].edge, lrs1877[i + j].data * 0.5);
                        strcat(pbuf, "\n");
                        STR_INC(pbuf, buffer);
                     }

                     i += count;
                  }
               } else
                  for (i = 0; i < size; i++) {
                     if ((bktype & 0xFF00) == 0)
                        db_sprintf(pbuf, pdata, size, i, bktype & 0xFF);

                     else if ((bktype & 0xFF00) == TID_LRS1882)
                        sprintf(pbuf, "GA %d CH %02d DA %d",
                                lrs1882[i].geo_addr, lrs1882[i].channel, lrs1882[i].data);

                     else if ((bktype & 0xFF00) == TID_PCOS3)
                        sprintf(pbuf, "TBD");
                     else
                        db_sprintf(pbuf, pdata, size, i, bktype & 0xFF);

                     strcat(pbuf, "\n");
                     STR_INC(pbuf, buffer);
                  }
            }
         }

      } while (1);
   }

  /*---- FIXED format ----------------------------------------------*/
   if (event_def->format == FORMAT_FIXED) {
      if (event_def->hDefKey == 0)
         cm_msg(MERROR, "write_event_ascii", "cannot find event definition");
      else {
         pdata = (char *) (pevent + 1);
         for (i = 0;; i++) {
            status = db_enum_key(hDB, event_def->hDefKey, i, &hKey);
            if (status == DB_NO_MORE_SUBKEYS)
               break;

            db_get_key(hDB, hKey, &key);
            sprintf(pbuf, "%s\n", key.name);
            STR_INC(pbuf, buffer);

            /* adjust for alignment */
            pdata = (void *) VALIGN(pdata, MIN(ss_get_struct_align(), key.item_size));

            for (j = 0; j < key.num_values; j++) {
               db_sprintf(pbuf, pdata, key.item_size, j, key.type);
               strcat(pbuf, "\n");
               STR_INC(pbuf, buffer);
            }

            /* shift data pointer to next item */
            pdata = (char *) pdata + key.item_size * key.num_values;
         }
      }
   }

   /* insert empty line after each event */
   strcat(pbuf, "\n");
   size = strlen(buffer);

   /* write record to device */
#ifdef HAVE_ZLIB
   if (out_gzip)
      status = gzwrite(file, buffer, size) == size ? SS_SUCCESS : SS_FILE_ERROR;
   else
#endif
      status =
          fwrite(buffer, 1, size, file) == (size_t) size ? SS_SUCCESS : SS_FILE_ERROR;

   return status;
}

/*---- MIDAS output ------------------------------------------------*/

INT write_event_midas(FILE * file, EVENT_HEADER * pevent, ANALYZE_REQUEST * par)
{
   INT status, size = 0, i;
   BOOL exclude;
   BANK_HEADER *pbh;
   BANK_LIST *pbl;
   EVENT_DEF *event_def;
   BANK *pbk;
   BANK32 *pbk32;
   char *pdata, *pdata_copy;
   char *pbuf;
   EVENT_HEADER *pevent_copy;
   DWORD bkname, bksize;
   WORD bktype;
   static char *buffer = NULL;

   if (buffer == NULL)
      buffer = (char *) malloc(MAX_EVENT_SIZE);

   pevent_copy = (EVENT_HEADER *) ALIGN8((POINTER_T) buffer);

   if (clp.filter) {
      /* use original event */
      size = pevent->data_size + sizeof(EVENT_HEADER);
      memcpy(pevent_copy, pevent, size);
   } else {
      /* copy only banks which are turned on via /analyzer/bank switches */

    /*---- MIDAS format ----------------------------------------------*/

      event_def = db_get_event_definition(pevent->event_id);
      if (event_def == NULL)
         return SUCCESS;

      if (event_def->format == FORMAT_MIDAS) {
         /* copy event header */
         pbuf = (char *) pevent_copy;
         memcpy(pbuf, pevent, sizeof(EVENT_HEADER));
         pbuf += sizeof(EVENT_HEADER);

         pbh = (BANK_HEADER *) (pevent + 1);

         if (bk_is32(pbh))
            bk_init32(pbuf);
         else
            bk_init(pbuf);

         pbk = NULL;
         pbk32 = NULL;
         pdata_copy = pbuf;
         do {
            /* scan all banks */
            if (bk_is32(pbh)) {
               size = bk_iterate32(pbh, &pbk32, &pdata);
               if (pbk32 == NULL)
                  break;
               bkname = *((DWORD *) pbk32->name);
               bktype = (WORD) pbk32->type;
               bksize = pbk32->data_size;
            } else {
               size = bk_iterate(pbh, &pbk, &pdata);
               if (pbk == NULL)
                  break;
               bkname = *((DWORD *) pbk->name);
               bktype = (WORD) pbk->type;
               bksize = pbk->data_size;
            }

            /* look if bank is in exclude list */
            exclude = FALSE;
            pbl = NULL;
            if (par->bank_list != NULL)
               for (i = 0; par->bank_list[i].name[0]; i++)
                  if (*((DWORD *) par->bank_list[i].name) == bkname) {
                     pbl = &par->bank_list[i];
                     exclude = (pbl->output_flag == 0);
                     break;
                  }

            if (!exclude) {
               /* copy bank */
               bk_create(pbuf, (char *) (&bkname), bktype, &pdata_copy);
               memcpy(pdata_copy, pdata, bksize);
               pdata_copy += bksize;
               bk_close(pbuf, pdata_copy);
            }

         } while (1);

         /* set event size in header */
         size = ALIGN8((POINTER_T) pdata_copy - (POINTER_T) pbuf);
         pevent_copy->data_size = size;
         size += sizeof(EVENT_HEADER);
      }

    /*---- FIXED format ----------------------------------------------*/
      if (event_def->format == FORMAT_FIXED) {
         size = pevent->data_size + sizeof(EVENT_HEADER);
         memcpy(pevent_copy, pevent, size);
      }

      if (pevent_copy->data_size == 0)
         return SUCCESS;
   }

   /* write record to device */
#ifdef HAVE_ZLIB
   if (out_gzip)
      status = gzwrite(file, pevent_copy, size) == size ? SUCCESS : SS_FILE_ERROR;
   else
#endif
      status =
          fwrite(pevent_copy, 1, size, file) == (size_t) size ? SUCCESS : SS_FILE_ERROR;

   return status;
}


/*------------------------------------------------------------------*/

static struct {
   short int event_id;
   DWORD last_time;
} last_time_event[50];

ANALYZE_REQUEST *_current_par;

void correct_num_events(INT i)
{
   if (_current_par)
      _current_par->events_received += i - 1;
}

INT process_event(ANALYZE_REQUEST * par, EVENT_HEADER * pevent)
{
   INT i, status = SUCCESS, ch;
   ANA_MODULE **module;
   DWORD actual_time;
   EVENT_DEF *event_def;
   static DWORD last_time_kb = 0;
   static char *orig_event = NULL;

   /* verbose output */
   if (clp.verbose)
      printf("event %d, number %d, total size %d\n",
             (int) pevent->event_id,
             (int) pevent->serial_number,
             (int) (pevent->data_size + sizeof(EVENT_HEADER)));

   /* save analyze_request for event number correction */
   _current_par = par;

   /* check keyboard once every second */
   actual_time = ss_millitime();
   if (!clp.online && actual_time - last_time_kb > 1000 && !clp.quiet ) {
      last_time_kb = actual_time;

      while (ss_kbhit()) {
         ch = ss_getchar(0);
         if (ch == -1)
            ch = getchar();

         if ((char) ch == '!')
            return RPC_SHUTDOWN;
      }
   }

   if (par == NULL) {
      /* load ODB with BOR event */
      if (pevent->event_id == EVENTID_BOR) {
         /* get run number from BOR event */
         current_run_number = pevent->serial_number;

         cm_msg(MINFO, "process_event", "Set run number %d in ODB", current_run_number);
         assert(current_run_number > 0);

         /* set run number in ODB */
         status = db_set_value(hDB, 0, "/Runinfo/Run number", &current_run_number,
                               sizeof(current_run_number), 1, TID_INT);
         assert(status == SUCCESS);

         /* load ODB from event */
         odb_load(pevent);

      }
   } else
      /* increment event counter */
      par->events_received++;


   /* don't analyze special (BOR,MESSAGE,...) events */
   if (par == NULL)
      return SUCCESS;

   /* swap event if necessary */
   event_def = db_get_event_definition(pevent->event_id);
   if (event_def == NULL)
      return 0;

   if (event_def->format == FORMAT_MIDAS)
      bk_swap((BANK_HEADER *) (pevent + 1), FALSE);

   /* keep copy of original event */
   if (clp.filter) {
      if (orig_event == NULL)
         orig_event = (char *) malloc(MAX_EVENT_SIZE + sizeof(EVENT_HEADER));
      memcpy(orig_event, pevent, pevent->data_size + sizeof(EVENT_HEADER));
   }

  /*---- analyze event ----*/

   /* call non-modular analyzer if defined */
   if (par->analyzer) {
      status = par->analyzer(pevent, (void *) (pevent + 1));

      /* don't continue if event was rejected */
      if (status == ANA_SKIP)
         return 0;
   }

   /* loop over analyzer modules */
   module = par->ana_module;
   for (i = 0; module != NULL && module[i] != NULL; i++) {
      if (module[i]->enabled) {
	
	if ( module[i]->analyzer ) 
	  {
	    printf("In module %s\n",module[i]->name);
	    status = module[i]->analyzer(pevent, (void *) (pevent + 1));
	  }

         /* don't continue if event was rejected */
         if (status == ANA_SKIP)
            return 0;
      }
   }

   if (event_def->format == FORMAT_MIDAS) {
      /* check if event got too large */
      i = bk_size(pevent + 1);
      if (i > MAX_EVENT_SIZE)
         cm_msg(MERROR, "process_event", "Event got too large (%d Bytes) in analyzer", i);

      /* correct for increased event size */
      pevent->data_size = i;
   }

   /* in filter mode, use original event */
   if (clp.filter)
      pevent = (EVENT_HEADER *) orig_event;

   /* write resulting event */
   if (out_file) {
      if (out_format == FORMAT_ASCII)
         status = write_event_ascii(out_file, pevent, par);
      if (out_format == FORMAT_MIDAS)
         status = write_event_midas(out_file, pevent, par);

      if (status != SUCCESS) {
         cm_msg(MERROR, "process_event", "Error writing to file (Disk full?)");
         return -1;
      }

      par->events_written++;
   }


   return SUCCESS;
}

/*------------------------------------------------------------------*/

void receive_event(HNDLE buffer_handle, HNDLE request_id, EVENT_HEADER * pheader,
                   void *pevent)
/* receive online event */
{
   INT i;
   ANALYZE_REQUEST *par;
   static DWORD buffer_size = 0;
   static char *buffer = NULL;
   char *pb;

   if (buffer == NULL) {
      buffer = (char *) malloc(MAX_EVENT_SIZE + sizeof(EVENT_HEADER));

      if (buffer == NULL) {
         cm_msg(MERROR, "receive_event", "Not enough memory to buffer event of size %d",
                buffer_size);
         return;
      }
   }

   /* align buffer */
   pb = (char *) ALIGN8((POINTER_T) buffer);

   /* copy event to local buffer */
   memcpy(pb, pheader, pheader->data_size + sizeof(EVENT_HEADER));

   par = analyze_request;

   for (i = 0; par->event_name[0]; par++)
      if (par->buffer_handle == buffer_handle && par->request_id == request_id) {
         process_event(par, (EVENT_HEADER *) pb);
      }
}

/*------------------------------------------------------------------*/

void update_request(HNDLE hDB, HNDLE hKey, void *info)
{
   AR_INFO *ar_info;
   INT i;

   if (!clp.online)
      return;

   /* check which request's key has changed */
   for (i = 0; analyze_request[i].event_name[0]; i++)
      if (analyze_request[i].hkey_common == hKey) {
         ar_info = &analyze_request[i].ar_info;

         /* remove previous request */
         if (analyze_request[i].request_id != -1)
            bm_delete_request(analyze_request[i].request_id);

         /* if enabled, add new request */
         if (ar_info->enabled)
            bm_request_event(analyze_request[i].buffer_handle, (short) ar_info->event_id,
                             (short) ar_info->trigger_mask, ar_info->sampling_type,
                             &analyze_request[i].request_id, receive_event);
         else
            analyze_request[i].request_id = -1;
      }

}

/*------------------------------------------------------------------*/

void register_requests(void)
{
   INT index, status;
   char str[256];
   AR_INFO *ar_info;
   AR_STATS *ar_stats;
   HNDLE hKey;

   /* scan ANALYZE_REQUEST table from ANALYZE.C */
   for (index = 0; analyze_request[index].event_name[0]; index++) {
      ar_info = &analyze_request[index].ar_info;
      ar_stats = &analyze_request[index].ar_stats;

      /* create common subtree from analyze_request table in analyze.c */
      sprintf(str, "/%s/%s/Common", analyzer_name, analyze_request[index].event_name);
      db_check_record(hDB, 0, str, ANALYZER_REQUEST_STR, TRUE);
      db_find_key(hDB, 0, str, &hKey);
      analyze_request[index].hkey_common = hKey;

      strcpy(ar_info->client_name, analyzer_name);
      gethostname(ar_info->host, sizeof(ar_info->host));
      db_set_record(hDB, hKey, ar_info, sizeof(AR_INFO), 0);

      /* open hot link to analyzer request info */
      db_open_record(hDB, hKey, ar_info, sizeof(AR_INFO), MODE_READ, update_request,
                     NULL);

      /* create statistics tree */
      sprintf(str, "/%s/%s/Statistics", analyzer_name, analyze_request[index].event_name);
      db_check_record(hDB, 0, str, ANALYZER_STATS_STR, TRUE);
      db_find_key(hDB, 0, str, &hKey);
      assert(hKey);

      ar_stats->events_received = 0;
      ar_stats->events_per_sec = 0;
      ar_stats->events_written = 0;

      /* open hot link to statistics tree */
      status =
          db_open_record(hDB, hKey, ar_stats, sizeof(AR_STATS), MODE_WRITE, NULL, NULL);
      if (status != DB_SUCCESS)
         printf("Cannot open statistics record, probably other analyzer is using it\n");

      if (clp.online) {
         /*---- open event buffer ---------------------------------------*/
         bm_open_buffer(ar_info->buffer, 2*MAX_EVENT_SIZE,
                        &analyze_request[index].buffer_handle);

         /* set the default buffer cache size */
         bm_set_cache_size(analyze_request[index].buffer_handle, 100000, 0);

         /*---- request event -------------------------------------------*/
         if (ar_info->enabled)
            bm_request_event(analyze_request[index].buffer_handle,
                             (short) ar_info->event_id, (short) ar_info->trigger_mask,
                             ar_info->sampling_type, &analyze_request[index].request_id,
                             receive_event);
         else
            analyze_request[index].request_id = -1;
      }
   }
}

/*------------------------------------------------------------------*/

void update_stats()
{
   int i;
   AR_STATS *ar_stats;
   static DWORD last_time = 0;
   DWORD actual_time;

   actual_time = ss_millitime();

   if (last_time == 0)
      last_time = actual_time;

   if (actual_time - last_time == 0)
      return;

   for (i = 0; analyze_request[i].event_name[0]; i++) {
      ar_stats = &analyze_request[i].ar_stats;
      ar_stats->events_received += analyze_request[i].events_received;
      ar_stats->events_written += analyze_request[i].events_written;
      ar_stats->events_per_sec =
          (analyze_request[i].events_received / ((actual_time - last_time) / 1000.0));
      analyze_request[i].events_received = 0;
      analyze_request[i].events_written = 0;
   }

   /* propagate new statistics to ODB */
   db_send_changed_records();

   last_time = actual_time;
}

/*-- Book histos --------------------------------------------------*/

#ifdef USE_ROOT

/* h1_book and h2_book are now templates in midas.h */

//==============================================================================



#endif

/*-- Clear histos --------------------------------------------------*/

void load_last_histos()
{
   char str[256];

   /* load previous online histos */
   if (!clp.no_load) {
      strcpy(str, out_info.last_histo_filename);

      if (strchr(str, DIR_SEPARATOR) == NULL)
         add_data_dir(str, out_info.last_histo_filename);

#ifdef USE_ROOT
      printf("Loading previous online histos from %s\n", str);
      LoadRootHistograms(gManaHistosFolder, str);
#endif
   }
}

/*------------------------------------------------------------------*/

void save_last_histos()
{
   char str[256];

   /* save online histos */
   strcpy(str, out_info.last_histo_filename);
   if (strchr(str, DIR_SEPARATOR) == NULL)
      add_data_dir(str, out_info.last_histo_filename);

   printf("Saving current online histos to %s\n", str);

#ifdef USE_ROOT
   SaveRootHistograms(gManaHistosFolder, str);
#endif

}

/*------------------------------------------------------------------*/

INT loop_online()
{
   INT status = SUCCESS;
   DWORD last_time_loop, last_time_update, actual_time;
   int ch;

   printf("Running analyzer online. Stop with \"!\"\n");

   /* main loop */
   last_time_update = 0;
   last_time_loop = 0;

   do {
      /* calculate events per second */
      actual_time = ss_millitime();

      if (actual_time - last_time_update > 1000) {
         /* update statistics */
         update_stats();
         last_time_update = actual_time;

         /* check keyboard */
         ch = 0;
         while (ss_kbhit()) {
            ch = ss_getchar(0);
            if (ch == -1)
               ch = getchar();

            if ((char) ch == '!')
               break;
         }

         if ((char) ch == '!')
            break;
      }

      if (analyzer_loop_period == 0)
         status = cm_yield(1000);
      else {
         if (actual_time - last_time_loop > (DWORD) analyzer_loop_period) {
            last_time_loop = actual_time;
            analyzer_loop();
         }

         status = cm_yield(analyzer_loop_period);
      }

   } while (status != RPC_SHUTDOWN && status != SS_ABORT);

   /* update statistics */
   update_stats();

   return status;
}

/*------------------------------------------------------------------*/

INT init_module_parameters(BOOL bclose)
{
   INT i, j, status, size;
   ANA_MODULE **module;
   char str[80];
   HNDLE hkey;

   for (i = 0; analyze_request[i].event_name[0]; i++) {
      module = analyze_request[i].ana_module;
      for (j = 0; module != NULL && module[j] != NULL; j++) {
         if (module[j]->parameters != NULL) {
            sprintf(str, "/%s/Parameters/%s", analyzer_name, module[j]->name);

            if (bclose) {
               db_find_key(hDB, 0, str, &hkey);
               db_close_record(hDB, hkey);
            } else {
               status = db_find_key(hDB, 0, str, &hkey);
               if (status == DB_SUCCESS) {
                  db_get_record_size(hDB, hkey, 0, &size);
                  if (size != module[j]->param_size)
                     status = 0;
               }
               if (status != DB_SUCCESS && module[j]->init_str) {
                  if (db_check_record(hDB, 0, str, strcomb((const char **)module[j]->init_str), TRUE) !=
                      DB_SUCCESS) {
                     cm_msg(MERROR, "init_module_parameters",
                            "Cannot create/check \"%s\" parameters in ODB", str);
                     return 0;
                  }
               }

               db_find_key(hDB, 0, str, &hkey);
               assert(hkey);

               if (db_open_record(hDB, hkey, module[j]->parameters, module[j]->param_size,
                                  MODE_READ, NULL, NULL) != DB_SUCCESS) {
                  cm_msg(MERROR, "init_module_parameters",
                         "Cannot open \"%s\" parameters in ODB", str);
                  return 0;
               }
            }
         }
      }
   }

   return SUCCESS;
}

/*------------------------------------------------------------------*/
void odb_load(EVENT_HEADER * pevent)
{
   BOOL flag;
   int size, i, status;
   char str[256];
   HNDLE hKey, hKeyRoot, hKeyEq;

   flag = TRUE;
   size = sizeof(flag);
   sprintf(str, "/%s/ODB Load", analyzer_name);
   db_get_value(hDB, 0, str, &flag, &size, TID_BOOL, TRUE);

   if (flag) {
      for (i = 0; i < 10; i++)
         if (clp.protect[i][0] && !clp.quiet)
            printf("Protect ODB tree \"%s\"\n", clp.protect[i]);

      if (!clp.quiet)
         printf("Load ODB from run %d...", (int) current_run_number);

      if (flag == 1) {
         /* lock all ODB values except run parameters */
         db_set_mode(hDB, 0, MODE_READ, TRUE);

         db_find_key(hDB, 0, "/Experiment/Run Parameters", &hKey);
         if (hKey)
            db_set_mode(hDB, hKey, MODE_READ | MODE_WRITE | MODE_DELETE, TRUE);

         /* and analyzer parameters */
         sprintf(str, "/%s/Parameters", analyzer_name);
         db_find_key(hDB, 0, str, &hKey);
         if (hKey)
            db_set_mode(hDB, hKey, MODE_READ | MODE_WRITE | MODE_DELETE, TRUE);

         /* and equipment (except /variables) */
         db_find_key(hDB, 0, "/Equipment", &hKeyRoot);
         if (hKeyRoot) {
            db_set_mode(hDB, hKeyRoot, MODE_READ | MODE_WRITE | MODE_DELETE, FALSE);

            for (i = 0;; i++) {
               status = db_enum_key(hDB, hKeyRoot, i, &hKeyEq);
               if (status == DB_NO_MORE_SUBKEYS)
                  break;

               db_set_mode(hDB, hKeyEq, MODE_READ | MODE_WRITE | MODE_DELETE, TRUE);

               db_find_key(hDB, hKeyEq, "Variables", &hKey);
               if (hKey)
                  db_set_mode(hDB, hKey, MODE_READ, TRUE);
            }
         }

         /* lock protected trees */
         for (i = 0; i < 10; i++)
            if (clp.protect[i][0]) {
               db_find_key(hDB, 0, clp.protect[i], &hKey);
               if (hKey)
                  db_set_mode(hDB, hKey, MODE_READ, TRUE);
            }
      }

      /* close open records to parameters */
      init_module_parameters(TRUE);

      if (strncmp((char *) (pevent + 1), "<?xml version=\"1.0\"", 19) == 0)
         db_paste_xml(hDB, 0, (char *) (pevent + 1));
      else
         db_paste(hDB, 0, (char *) (pevent + 1));

      if (flag == 1)
         db_set_mode(hDB, 0, MODE_READ | MODE_WRITE | MODE_DELETE, TRUE);

      /* reinit structured opened by user analyzer */
      analyzer_init();

      /* reload parameter files after BOR event */
      if (!clp.quiet)
         printf("OK\n");
      load_parameters(current_run_number);

      /* open module parameters again */
      init_module_parameters(FALSE);
   }
}

/*------------------------------------------------------------------*/

#define MA_DEVICE_DISK        1
#define MA_DEVICE_TAPE        2
#define MA_DEVICE_FTP         3

#define MA_FORMAT_MIDAS       (1<<0)
#define MA_FORMAT_GZIP        (1<<3)

typedef struct {
   char file_name[256];
   int format;
   int device;
   int fd;
#ifdef HAVE_ZLIB
   gzFile gzfile;
#else
   FILE *file;
#endif
   char *buffer;
   int wp, rp;
   /*FTP_CON ftp_con; */
} MA_FILE;

/*------------------------------------------------------------------*/

MA_FILE *ma_open(char *file_name)
{
   char *ext_str;
   MA_FILE *file;

   /* allocate MA_FILE structure */
   file = (MA_FILE *) calloc(sizeof(MA_FILE), 1);
   if (file == NULL) {
      cm_msg(MERROR, "ma_open", "Cannot allocate MA file structure");
      return NULL;
   }

   /* save file name */
   strcpy(file->file_name, file_name);

   /* for now, just read from disk */
   file->device = MA_DEVICE_DISK;


   /* check input file extension */
   if (strchr(file_name, '.')) {
      ext_str = file_name + strlen(file_name) - 1;
      while (*ext_str != '.')
         ext_str--;
   } else
      ext_str = "";

   if (strncmp(ext_str, ".gz", 3) == 0) {
#ifdef HAVE_ZLIB
      ext_str--;
      while (*ext_str != '.' && ext_str > file_name)
         ext_str--;
#else
      cm_msg(MERROR, "ma_open",
             ".gz extension not possible because zlib support is not compiled in.\n");
      return NULL;
#endif
   }

   if (strncmp(file_name, "/dev/", 4) == 0)     /* assume MIDAS tape */
      file->format = MA_FORMAT_MIDAS;
   else if (strncmp(ext_str, ".mid", 4) == 0)
      file->format = MA_FORMAT_MIDAS;
   else {
      printf
          ("Unknown input data format \"%s\". Please use file extension .mid or mid.gz.\n",
           ext_str);
      return NULL;
   }

   if (file->device == MA_DEVICE_DISK) 
     {
#ifdef HAVE_ZLIB
       file->gzfile = gzopen(file_name, "rb");
       if (file->gzfile == NULL)
	 return NULL;
#else
       file->file = fopen(file_name, "rb");
       if (file->file == NULL)
	 return NULL;
#endif
       
     }
   
   return file;
}

/*------------------------------------------------------------------*/

int ma_close(MA_FILE * file)
{

#ifdef HAVE_ZLIB
      gzclose(file->gzfile);
#else
      fclose(file->file);
#endif

   free(file);
   return SUCCESS;
}

/*------------------------------------------------------------------*/

int ma_read_event(MA_FILE * file, EVENT_HEADER * pevent, int size)
{
   int n;

   if (file->device == MA_DEVICE_DISK) {
      if (file->format == MA_FORMAT_MIDAS) {
         if (size < (int) sizeof(EVENT_HEADER)) {
            cm_msg(MERROR, "ma_read_event", "Buffer size too small");
            return -1;
         }

         /* read event header */
#ifdef HAVE_ZLIB
         n = gzread(file->gzfile, pevent, sizeof(EVENT_HEADER));
#else
         n = sizeof(EVENT_HEADER)*fread(pevent, sizeof(EVENT_HEADER), 1, file->file);
#endif

         if (n < (int) sizeof(EVENT_HEADER)) {
            if (n > 0)
               printf("Unexpected end of file %s, last event skipped\n", file->file_name);
            return -1;
         }

         /* swap event header if in wrong format */
#ifdef SWAP_EVENTS
         WORD_SWAP(&pevent->event_id);
         WORD_SWAP(&pevent->trigger_mask);
         DWORD_SWAP(&pevent->serial_number);
         DWORD_SWAP(&pevent->time_stamp);
         DWORD_SWAP(&pevent->data_size);
#endif

         /* read event */
         n = 0;
         if (pevent->data_size > 0) {
            if (size < (int) pevent->data_size + (int) sizeof(EVENT_HEADER)) {
               cm_msg(MERROR, "ma_read_event", "Buffer size too small");
               return -1;
            }
#ifdef HAVE_ZLIB
            n = gzread(file->gzfile, pevent + 1, pevent->data_size);
#else
            n = pevent->data_size*fread(pevent + 1, pevent->data_size, 1, file->file);
#endif
            if (n != (INT) pevent->data_size) {
               printf("Unexpected end of file %s, last event skipped\n", file->file_name);
               return -1;
            }
         }

         return n + sizeof(EVENT_HEADER);
      }

   } 

   return 0;
}

/*------------------------------------------------------------------*/

INT analyze_run(INT run_number, char *input_file_name, char *output_file_name)
{
   EVENT_HEADER *pevent, *pevent_unaligned;
   ANALYZE_REQUEST *par;
   INT i, n, size;
   DWORD num_events_in, num_events_out;
   char error[256], str[256];
   INT status = SUCCESS;
   MA_FILE *file;
   BOOL skip;
   DWORD start_time;

   /* set output file name and flags in ODB */
   sprintf(str, "/%s/Output/Filename", analyzer_name);
   db_set_value(hDB, 0, str, output_file_name, 256, 1, TID_STRING);

   assert(run_number > 0);

   /* set run number in ODB */
   status =
       db_set_value(hDB, 0, "/Runinfo/Run number", &run_number, sizeof(run_number), 1,
                    TID_INT);
   assert(status == SUCCESS);

   /* set file name in out_info */
   strcpy(out_info.filename, output_file_name);

   /* let changes propagate to modules */
   cm_yield(0);

   /* open input file, will be changed to ma_open_file later... */
   file = ma_open(input_file_name);
   if (file == NULL) {
      printf("Cannot open input file \"%s\"\n", input_file_name);
      return -1;
   }

   pevent_unaligned = (EVENT_HEADER *) malloc(EXT_EVENT_SIZE);
   if (pevent_unaligned == NULL) {
      printf("Not enough memeory\n");
      return -1;
   }
   pevent = (EVENT_HEADER *) ALIGN8((POINTER_T) pevent_unaligned);

   /* call analyzer bor routines */
   bor(run_number, error);

   num_events_in = num_events_out = 0;

   start_time = ss_millitime();

   /* event loop */
   do {
      /* read next event */
      n = ma_read_event(file, pevent, EXT_EVENT_SIZE);
      if (n <= 0)
         break;

      num_events_in++;

      /* copy system events (BOR, EOR, MESSAGE) to output file */
      if (pevent->event_id < 0) {
         status = process_event(NULL, pevent);
         if (status < 0 || status == RPC_SHUTDOWN)      /* disk full/stop analyzer */
            break;

         if (out_file && out_format == FORMAT_MIDAS) {
            size = pevent->data_size + sizeof(EVENT_HEADER);
#ifdef HAVE_ZLIB
            if (out_gzip)
               status = gzwrite(out_file, pevent, size) == size ? SUCCESS : SS_FILE_ERROR;
            else
#endif
               status =
                   fwrite(pevent, 1, size,
                          out_file) == (size_t) size ? SUCCESS : SS_FILE_ERROR;

            if (status != SUCCESS) {
               cm_msg(MERROR, "analyze_run", "Error writing to file (Disk full?)");
               return -1;
            }

            num_events_out++;
         }

         /* reinit start time after BOR event */
         if (pevent->event_id == EVENTID_BOR)
            start_time = ss_millitime();
      }

      /* check if event is in event limit */
      skip = FALSE;

      if (clp.n[0] > 0 || clp.n[1] > 0) {
	if (clp.n[1] == 0) {
	  /* treat n[0] as upper limit */
	  if (num_events_in > clp.n[0]) {
	    num_events_in--;
	    status = SUCCESS;
	    break;
	  }
	} else {
	  if (num_events_in > clp.n[1]) {
	    status = SUCCESS;
	    break;
	  }
	  if (num_events_in < clp.n[0])
	    skip = TRUE;
	  else if (clp.n[2] > 0 && num_events_in % clp.n[2] != 0)
	    skip = TRUE;
	}
      }
      
      if (!skip) {
         /* find request belonging to this event */
         par = analyze_request;
         status = SUCCESS;
         for (i = 0; par->event_name[0]; par++)
            if ((par->ar_info.event_id == EVENTID_ALL ||
                 par->ar_info.event_id == pevent->event_id) &&
                (par->ar_info.trigger_mask == TRIGGER_ALL ||
                 (par->ar_info.trigger_mask & pevent->trigger_mask))
                && par->ar_info.enabled) {
               /* analyze this event */
               status = process_event(par, pevent);
               if (status == SUCCESS)
                  num_events_out++;
               if (status < 0 || status == RPC_SHUTDOWN)        /* disk full/stop analyzer */
                  break;

               /* check for Ctrl-C */
               status = cm_yield(0);
            }
         if (status < 0 || status == RPC_SHUTDOWN)
            break;
      }

      /* update ODB statistics once every 100 events */
      if (num_events_in % 100 == 0) {
         update_stats();
         if (!clp.quiet) {
            if (out_file)
               printf("%s:%d  %s:%d  events\r", input_file_name, (int) num_events_in,
                      out_info.filename, (int) num_events_out);
            else
               printf("%s:%d  events\r", input_file_name, (int) num_events_in);

#ifndef OS_WINNT
            fflush(stdout);
#endif
         }
      }
   } while (1);

   /* signal EOR to slaves */
   start_time = ss_millitime() - start_time;

   update_stats();
   if (!clp.quiet) {
      if (out_file)
         printf("%s:%d  %s:%d  events, %1.2lfs\n", input_file_name, (int) num_events_in,
                out_info.filename, (int) num_events_out, start_time / 1000.0);
      else
         printf("%s:%d  events, %1.2lfs\n", input_file_name, (int) num_events_in,
                start_time / 1000.0);
   }

   /* call analyzer eor routines */
   eor(current_run_number, error);

   ma_close(file);

   free(pevent_unaligned);

   return status;
}

/*------------------------------------------------------------------*/

INT loop_runs_offline()
{
   INT i, status, run_number;
   char input_file_name[256], output_file_name[256], *prn;
   BANK_LIST *bank_list;

   if (!clp.quiet)
      printf("Running analyzer offline. Stop with \"!\"\n");

   run_number = 0;
   out_append = ((strchr(clp.input_file_name[0], '%') != NULL) &&
                 (strchr(clp.output_file_name, '%') == NULL))
       || clp.input_file_name[1][0];

   /* loop over range of files */
   if (clp.run_number[0] > 0) {
      if (strchr(clp.input_file_name[0], '%') == NULL) {
         printf
             ("Input file name must contain a wildcard like \"%%05d\" when using a range.\n");
         return 0;
      }

      if (clp.run_number[0] == 0) {
         printf("End of range not specified.\n");
         return 0;
      }

      for (run_number = clp.run_number[0]; run_number <= clp.run_number[1]; run_number++) {
         sprintf(input_file_name, clp.input_file_name[0], run_number);
         if (strchr(clp.output_file_name, '%') != NULL)
            sprintf(output_file_name, clp.output_file_name, run_number);
         else
            strcpy(output_file_name, clp.output_file_name);

         status = analyze_run(run_number, input_file_name, output_file_name);
         if (status == RPC_SHUTDOWN)
            break;
      }
   } else {
      /* loop over input file names */
      for (i = 0; clp.input_file_name[i][0] && i < 10; i++) {
         strcpy(input_file_name, clp.input_file_name[i]);

         /* get run number from input file */
         prn = input_file_name;
         while (strchr(prn, DIR_SEPARATOR) != NULL)
            prn = strchr(prn, DIR_SEPARATOR) + 1;

         if (strpbrk(prn, "0123456789"))
            run_number = atoi(strpbrk(prn, "0123456789"));

         if (strchr(clp.output_file_name, '%') != NULL) {
            if (run_number == 0) {
               printf("Cannot extract run number from input file name.\n");
               return 0;
            }
            sprintf(output_file_name, clp.output_file_name, run_number);
         } else
            strcpy(output_file_name, clp.output_file_name);

         status = analyze_run(run_number, input_file_name, output_file_name);
         if (status == RPC_SHUTDOWN)
            break;
      }
   }

   /* close output file in append mode */
   if (out_file && out_append) {
      if (out_format == FORMAT_HBOOK) {
         cm_msg(MERROR, "loop_runs_offline", "HBOOK support is not compiled in");
      } else if (out_format == FORMAT_ROOT) {
#ifdef USE_ROOT
         CloseRootOutputFile();
#else
         cm_msg(MERROR, "loop_runs_offline", "ROOT support is not compiled in");
#endif                          /* USE_ROOT */
      } else {
#ifdef HAVE_ZLIB
         if (out_gzip)
            gzclose(out_file);
         else
#endif
            fclose(out_file);
      }

      /* free bank buffer */
      for (i = 0; analyze_request[i].event_name[0]; i++) {
         bank_list = analyze_request[i].bank_list;

         if (bank_list == NULL)
            continue;

         for (; bank_list->name[0]; bank_list++)
            if (bank_list->addr) {
               free(bank_list->addr);
               bank_list->addr = NULL;
            }
      }
   }

   return CM_SUCCESS;
}

/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/

#ifdef USE_ROOT

/*==== ROOT socket histo server ====================================*/

#if defined ( OS_UNIX )
#define THREADRETURN
#define THREADTYPE void
#endif
#if defined( OS_WINNT )
#define THREADRETURN 0
#define THREADTYPE DWORD WINAPI
#endif

/*------------------------------------------------------------------*/

TFolder *ReadFolderPointer(TSocket * fSocket)
{
   //read pointer to current folder
   TMessage *m = 0;
   fSocket->Recv(m);
   POINTER_T p;
   *m >> p;
   return (TFolder *) p;
}

/*------------------------------------------------------------------*/

THREADTYPE root_server_thread(void *arg)
/*
  Serve histograms over TCP/IP socket link
*/
{
   char request[256];

   TSocket *sock = (TSocket *) arg;
   sock->Send("RMSERV 2.0");

   printf("Socket thread started\n");

   do {

      /* close connection if client has disconnected */
      if (sock->Recv(request, sizeof(request)) <= 0) {
         printf("Closed connection to %s\n", sock->GetInetAddress().GetHostName());
         sock->Close();
         delete sock;
         return THREADRETURN;

      } else {

	printf("socket request: [%s]\n",request);


         TMessage *message = new TMessage(kMESS_OBJECT);

         if (strcmp(request, "GetListOfFolders") == 0) {

            TFolder *folder = ReadFolderPointer(sock);
            if (folder == NULL) {
               message->Reset(kMESS_OBJECT);
               message->WriteObject(NULL);
               sock->Send(*message);
               delete message;
               continue;
            }
            //get folder names
            TObject *obj;
            TObjArray *names = new TObjArray(100);

            TCollection *folders = folder->GetListOfFolders();
            TIterator *iterFolders = folders->MakeIterator();
            while ((obj = iterFolders->Next()) != NULL)
               names->Add(new TObjString(obj->GetName()));

            //write folder names
            message->Reset(kMESS_OBJECT);
            message->WriteObject(names);
            sock->Send(*message);

            for (int i = 0; i < names->GetLast() + 1; i++)
               delete(TObjString *) names->At(i);

            delete names;

            delete message;

         } else if (strncmp(request, "FindObject", 10) == 0) {

	   printf("Processing FindObject request\n");
	   
	   //TFolder *folder = ReadFolderPointer(sock);
	   TFolder *folder = gManaHistosFolder;

	   //get object
	   TObject *obj;
	   if (strncmp(request + 10, "Any", 3) == 0)
	     obj = folder->FindObjectAny(request + 14);
	   else
	     obj = folder->FindObject(request + 11);
	   
	   
	   //write object
	   printf("Sending object to client...\n");
	   if (!obj)
	     sock->Send("Error");
	   else {
	     message->Reset(kMESS_OBJECT);
	     message->WriteObject(obj);
	     sock->Send(*message);
	   }
	   delete message;
	    printf("Done\n");
	    
         } else if (strncmp(request, "FindFullPathName", 16) == 0) {
	   
	   TFolder *folder = ReadFolderPointer(sock);
	   
	   //find path
	   const char *path = folder->FindFullPathName(request + 17);
	   
	   //write path
	   if (!path) {
	     sock->Send("Error");
	   } else {
	     TObjString *obj = new TObjString(path);
	     message->Reset(kMESS_OBJECT);
	     message->WriteObject(obj);
	     sock->Send(*message);
	     delete obj;
	   }
	   delete message;
	   
         } else if (strncmp(request, "Occurence", 9) == 0) {
	   
            TFolder *folder = ReadFolderPointer(sock);

            //read object
            TMessage *m = 0;
            sock->Recv(m);
            TObject *obj = ((TObject *) m->ReadObject(m->GetClass()));

            //get occurence
            Int_t retValue = folder->Occurence(obj);

            //write occurence
            message->Reset(kMESS_OBJECT);
            *message << retValue;
            sock->Send(*message);

            delete message;

         } else if (strncmp(request, "GetPointer", 10) == 0) {

            //find object
	    //TObject *obj = gROOT->FindObjectAny(request + 11);
	   TObject *obj = gManaHistosFolder->FindObjectAny(request + 11);

            //write pointer
            message->Reset(kMESS_ANY);
            POINTER_T p = (POINTER_T) obj;
            *message << p;
            sock->Send(*message);

	    printf("Pointer: [%d]\n",p);

            delete message;

         } else if (strncmp(request, "Command", 7) == 0) {
            char objName[100], method[100];
            sock->Recv(objName, sizeof(objName));
            sock->Recv(method, sizeof(method));
            TObject *object = gROOT->FindObjectAny(objName);
            if (object && object->InheritsFrom(TH1::Class())
                && strcmp(method, "Reset") == 0)
               static_cast < TH1 * >(object)->Reset();

         } else if (strncmp(request, "GetRunNumber", 12) == 0) {

	   ULong_t run_nr = current_run_number;

	   message->Reset(kMESS_ANY);
	   *message << run_nr;
	   sock->Send(*message);

	   delete message;
	   
	 } else
	   printf("SocketServer: Received unknown command \"%s\"\n", request);
      }
   } while (1);
   
   return THREADRETURN;
}

/*------------------------------------------------------------------*/

THREADTYPE root_socket_server(void *arg)
{
// Server loop listening for incoming network connections on specified port.
// Starts a searver_thread for each connection.
   int port;

   port = *(int *) arg;

   printf("Root server listening on port %d...\n", port);
   TServerSocket *lsock = new TServerSocket(port, kTRUE);

   do {
      TSocket *sock = lsock->Accept();

      printf("Established connection to %s\n", sock->GetInetAddress().GetHostName());

#if defined ( __linux__ )
      TThread *thread = new TThread("Server", root_server_thread, sock);
      thread->Run();
#endif
#if defined( _MSC_VER )
      LPDWORD lpThreadId = 0;
      CloseHandle(CreateThread(NULL, 1024, &root_server_thread, sock, 0, lpThreadId));
#endif
   } while (1);

   return THREADRETURN;
}

/*------------------------------------------------------------------*/

void start_root_socket_server(int port)
{
   static int pport = port;
#if defined ( __linux__ )
   TThread *thread = new TThread("server_loop", root_socket_server, &pport);
   thread->Run();
#endif
#if defined( _MSC_VER )
   LPDWORD lpThreadId = 0;
   CloseHandle(CreateThread(NULL, 1024, &root_socket_server, &pport, 0, lpThreadId));
#endif
}

/*------------------------------------------------------------------*/

void *root_event_loop(void *arg)
/*
  Thread wrapper around main event loop
*/
{
   if (clp.online)
      loop_online();
   else
      loop_runs_offline();

   gSystem->ExitLoop();

   return NULL;
}

#endif                          /* USE_ROOT */

/*------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
   INT status, size;
   char str[256];
   HNDLE hkey;

#ifdef USE_ROOT
   int argn = 1;
   char *argp = (char *) argv[0];

   manaApp = new TRint("ranalyzer", &argn, &argp, NULL, 0, true);

   /* default server port */
   clp.root_port = 9090;
#endif

   /* get default from environment */
   cm_get_environment(clp.host_name, sizeof(clp.host_name), clp.exp_name,
                      sizeof(clp.exp_name));


   /* read in command line parameters into clp structure */
   status = getparam(argc, argv);
   if (status != CM_SUCCESS)
      return 1;

   /* become a daemon */
   if (clp.daemon) {
      printf("Becoming a daemon...\n");
      clp.quiet = TRUE;
      ss_daemon_init(FALSE);
   }

   /* set online mode if no input filename is given */
   clp.online = (clp.input_file_name[0][0] == 0);

   /* Don't run offline if MIDAS_DIR is not defined */
   if ( ! clp.online ) 
     {
       if ( !getenv("MIDAS_DIR") )
	 {
	   printf("For offline running the directory MIDAS_DIR must be defined\n");
	   return 1;
	 }
     }

#ifdef USE_ROOT
   /* workaround for multi-threading with midas system calls */
   ss_force_single_thread();
#endif

   /* now connect to server */
   if (clp.online) {
      if (clp.host_name[0])
         printf("Connect to experiment %s on host %s...", clp.exp_name, clp.host_name);
      else
         printf("Connect to experiment %s...", clp.exp_name);
   }

   status =
       cm_connect_experiment1(clp.host_name, clp.exp_name, analyzer_name, NULL, odb_size,
                              DEFAULT_WATCHDOG_TIMEOUT);

   if (status == CM_UNDEF_EXP) {
      printf("\nError: Experiment \"%s\" not defined.\n", clp.exp_name);
      if (getenv("MIDAS_DIR")) {
         printf
             ("Note that \"MIDAS_DIR\" is defined, which results in a single experiment\n");
         printf
             ("called \"Default\". If you want to use the \"exptab\" file, undefine \"MIDAS_DIR\".\n");
      }
      return 1;
   } else if (status != CM_SUCCESS) {
      cm_get_error(status, str);
      printf("\nError: %s\n", str);
      return 1;
   }

   if (clp.online)
      printf("OK\n");

   /* set online/offline mode */
   cm_get_experiment_database(&hDB, NULL);
   db_set_value(hDB, 0, "/Runinfo/Online Mode", &clp.online, sizeof(clp.online), 1,
                TID_INT);

   if (clp.online) {
      /* check for duplicate name */
      status = cm_exist(analyzer_name, FALSE);
      if (status == CM_SUCCESS) {
         cm_disconnect_experiment();
         printf("An analyzer named \"%s\" is already running in this experiment.\n",
                analyzer_name);
         printf
             ("Please select another analyzer name in analyzer.c or stop other analyzer.\n");
         return 1;
      }

      /* register transitions if started online */
      if (cm_register_transition(TR_START, tr_start, 300) != CM_SUCCESS ||
          cm_register_transition(TR_STOP, tr_stop, 700) != CM_SUCCESS ||
          cm_register_transition(TR_PAUSE, tr_pause, 700) != CM_SUCCESS ||
          cm_register_transition(TR_RESUME, tr_resume, 300) != CM_SUCCESS) {
         printf("Failed to start local RPC server");
         return 1;
      }
   } else {
     status = cm_exist(analyzer_name, FALSE);
         if (status == CM_SUCCESS) {
            /* kill hanging previous analyzer */
            cm_cleanup(analyzer_name, FALSE);

            status = cm_exist(analyzer_name, FALSE);
            if (status == CM_SUCCESS) {
               /* analyzer may only run once if offline */
               status = cm_shutdown(analyzer_name, FALSE);
               if (status == CM_SHUTDOWN)
                  printf("Previous analyzer stopped\n");
            }
      }
   }


   /* turn on keepalive messages */
   cm_set_watchdog_params(TRUE, DEFAULT_RPC_TIMEOUT);

   /* decrease watchdog timeout in offline mode */
   if (!clp.online)
      cm_set_watchdog_params(TRUE, 2000);

   /* turn off watchdog if in debug mode */
   if (clp.debug)
      cm_set_watchdog_params(0, 0);

   /* initialize module parameters */
   if (init_module_parameters(FALSE) != CM_SUCCESS) {
      cm_disconnect_experiment();
      return 1;
   }

   /* create ODB structure for output */
   sprintf(str, "/%s/Output", analyzer_name);
   db_check_record(hDB, 0, str, ANA_OUTPUT_INFO_STR, TRUE);
   db_find_key(hDB, 0, str, &hkey);
   assert(hkey);
   size = sizeof(out_info);
   db_get_record(hDB, hkey, &out_info, &size, 0);

#ifdef USE_ROOT
   /* create the folder for analyzer histograms */
   gManaHistosFolder =
       gROOT->GetRootFolder()->AddFolder("histos", "MIDAS Analyzer Histograms");
   gROOT->GetListOfBrowsables()->Add(gManaHistosFolder, "histos");
   gManaHistosFolder->SetOwner( kTRUE );

   /* convert .rz names to .root names */
   if (strstr(out_info.last_histo_filename, ".rz"))
      strcpy(out_info.last_histo_filename, "last.root");

   if (strstr(out_info.histo_dump_filename, ".rz"))
      strcpy(out_info.histo_dump_filename, "his%05d.root");

   db_set_record(hDB, hkey, &out_info, sizeof(out_info), 0);

   /* start socket server */
   if (clp.root_port)
      start_root_socket_server(clp.root_port);

#endif                          /* USE_ROOT */


   /* analyzer init function */
   if (mana_init() != CM_SUCCESS) {
      cm_disconnect_experiment();
      return 1;
   }

   /* load histos from last.xxx */
#if 0
   if (clp.online)
      load_last_histos();
#endif

   /* reqister event requests */
   register_requests();

   /* initialize ss_getchar */
   if (!clp.quiet)
      ss_getchar(0);

   /*---- start main loop ----*/

   if (clp.online)
      loop_online();
   else
      loop_runs_offline();

   /* reset terminal */
   if (!clp.quiet)
      ss_getchar(TRUE);

   /* call exit function */
   mana_exit();

   /* save histos to last.xxx */
   if (clp.online)
      save_last_histos();

   /* disconnect from experiment */
   cm_disconnect_experiment();

#ifdef USE_ROOT
   if (clp.start_rint)
      manaApp->Run(true);
   printf("\r               \n");       /* overwrite superflous ROOT prompt */
#endif

   return 0;
}

