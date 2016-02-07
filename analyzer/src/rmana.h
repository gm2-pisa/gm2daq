#ifndef rmana_h
#define rmana_h

/* moved to rmana.h by VT */
typedef struct {
  //<<<<<<< HEAD
 char name[NAME_LENGTH];            /**< Module name                       */
 char author[NAME_LENGTH];          /**< Author                            */
  INT(*analyzer) (EVENT_HEADER *, void *);
                                           /**< Pointer to user analyzer routine  */
  //=======
  //char name[NAME_LENGTH];            /**< Module name                       */
  //char author[NAME_LENGTH];          /**< Author                            */
  //INT(*analyzer) (EVENT_HEADER *, void *);
  /**< Pointer to user analyzer routine*/
  
  INT(*bor) (INT run_number);       /**< Pointer to begin-of-run routine   */
  INT(*eor) (INT run_number);       /**< Pointer to end-of-run routine     */
  INT(*init) ();                    /**< Pointer to init routine           */
  INT(*exit) ();                    /**< Pointer to exit routine           */
 void *parameters;                  /**< Pointer to parameter structure    */
 INT param_size;                    /**< Size of parameter structure       */
 char **init_str;                   /**< Parameter init string             */
 BOOL enabled;                      /**< Enabled flag                      */
 void *histo_folder;
} ANA_MODULE;

//<<<<<<< HEAD
typedef struct {
 INT event_id;                      /**< Event ID associated with equipm.  */
 INT trigger_mask;                  /**< Trigger mask                      */
 INT sampling_type;                 /**< GET_ALL/GET_NONBLOCKING/GET_RECENT*/
char buffer[NAME_LENGTH];          /**< Event buffer to send events into  */
 BOOL enabled;                      /**< Enable flag                       */
 char client_name[NAME_LENGTH];     /**< Analyzer name                     */
char host[NAME_LENGTH];            /**< Host on which analyzer is running */
} AR_INFO;
//=======
    //>>>>>>> ee8bc45bb1e1f9dd59da944e53a97709244860c2

/*
typedef struct {
   INT event_id;                      /**< Event ID associated with equipm.  */
//   INT trigger_mask;                  /**< Trigger mask                      */
//   INT sampling_type;                 /**< GET_ALL/GET_NONBLOCKING/GET_RECENT*/
//   char buffer[NAME_LENGTH];          /**< Event buffer to send events into  */
//   BOOL enabled;                      /**< Enable flag                       */
//   char client_name[NAME_LENGTH];     /**< Analyzer name                     */
//   char host[NAME_LENGTH];            /**< Host on which analyzer is running */
//} AR_INFO;

/*typedef struct {
   double events_received;
  double events_per_sec;
   double events_written;
<<<<<<< HEAD
  } AR_STATS;
=======
} AR_STATS;
*/
//>>>>>>> ee8bc45bb1e1f9dd59da944e53a97709244860c2

typedef struct {
 char event_name[NAME_LENGTH];      /**< Event name                        */
AR_INFO ar_info;                   /**< From above                        */
  INT(*analyzer) (EVENT_HEADER *, void *);/**< Pointer to user analyzer routine  */
 ANA_MODULE **ana_module;           /**< List of analyzer modules          */
 BANK_LIST *bank_list;              /**< List of banks for event           */
  #if 0
 BOOL use_tests;                    /**< Use tests for this event          */
  #endif
 char **init_string;
 INT status;                        /**< One of FE_xxx                     */
 HNDLE buffer_handle;               /**< MIDAS buffer handle               */
 HNDLE request_id;                  /**< Event request handle              */
 HNDLE hkey_variables;              /**< Key to variables subtree in ODB   */
 HNDLE hkey_common;                 /**< Key to common subtree             */
 void *addr;                        /**< Buffer for CWNT filling           */
struct {
    DWORD run;
    DWORD serial;
    DWORD time;
    } number;                          /**< Buffer for event number for CWNT  */
 DWORD events_received;             /**< number of events sent             */
 DWORD events_written;              /**< number of events written          */
  AR_STATS ar_stats;
  } ANALYZE_REQUEST;

/* output file information, maps to /<analyzer>/Output */
/*
typedef struct {
 char filename[256];
 BOOL histo_dump;
 char histo_dump_filename[256];
 BOOL clear_histos;
 char last_histo_filename[256];
 char global_memory_name[8];
} ANA_OUTPUT_INFO;

#define ANA_OUTPUT_INFO_STR "\
Filename = STRING : [256] run%05d.asc\n\
Histo Dump = BOOL : 0\n\
Histo Dump Filename = STRING : [256] his%05d.rz\n\
Clear histos = BOOL : 1\n\
Last Histo Filename = STRING : [256] last.rz\n\
Global Memory Name = STRING : [8] ONLN\n\
"
*/


#if 0
/*---- Tests -------------------------------------------------------*/

typedef struct {
   char name[80];
   BOOL registered;
   DWORD count;
   DWORD previous_count;
   BOOL value;
} ANA_TEST;

#define SET_TEST(t, v) { if (!t.registered) test_register(&t); t.value = (v); }
#define TEST(t) (t.value)

#ifdef DEFINE_TESTS
#define DEF_TEST(t) ANA_TEST t = { #t, 0, 0, FALSE };
#else
#define DEF_TEST(t) extern ANA_TEST t;
#endif
#endif

/* make functions callable from a C++ program */
#ifdef __cplusplus
extern "C" {
#endif
#if 0
   void EXPRT test_register(ANA_TEST * t);
#endif
   void EXPRT add_data_dir(char *result, char *file);
   void EXPRT lock_histo(INT id);

#if 0
   void EXPRT open_subfolder(char *name);
   void EXPRT close_subfolder();
#endif
#ifdef __cplusplus
}
#endif

#ifdef USE_ROOT
#if 0
   /* root functions really are C++ functions */ extern TFolder *gManaHistosFolder;
//extern TObjArray *gHistoFolderStack;

   // book functions put a root object in a suitable folder
   // for histos, there are a lot of types, so we use templates.
   // for other objects we have one function per object
template < typename TH1X >
    TH1X EXPRT * h1_book(const char *name, const char *title,
                         int bins, double min, double max)
{
   TH1X *hist;

   /* check if histo already exists */
   if (!gHistoFolderStack->Last())
      hist = (TH1X *) gManaHistosFolder->FindObjectAny(name);
   else
      hist = (TH1X *) ((TFolder *) gHistoFolderStack->Last())->FindObjectAny(name);

   if (hist == NULL) {
      hist = new TH1X(name, title, bins, min, max);
      if (!gHistoFolderStack->Last())
         gManaHistosFolder->Add(hist);
      else
         ((TFolder *) gHistoFolderStack->Last())->Add(hist);
   }

   return hist;
}

template < typename TH1X >
    TH1X EXPRT * h1_book(const char *name, const char *title, int bins, double edges[])
{
   TH1X *hist;

   /* check if histo already exists */
   if (!gHistoFolderStack->Last())
      hist = (TH1X *) gManaHistosFolder->FindObjectAny(name);
   else
      hist = (TH1X *) ((TFolder *) gHistoFolderStack->Last())->FindObjectAny(name);

   if (hist == NULL) {
      hist = new TH1X(name, title, bins, edges);
      if (!gHistoFolderStack->Last())
         gManaHistosFolder->Add(hist);
      else
         ((TFolder *) gHistoFolderStack->Last())->Add(hist);
   }

   return hist;
}

template < typename TH2X >
    TH2X EXPRT * h2_book(const char *name, const char *title,
                         int xbins, double xmin, double xmax,
                         int ybins, double ymin, double ymax)
{
   TH2X *hist;

   /* check if histo already exists */
   if (!gHistoFolderStack->Last())
      hist = (TH2X *) gManaHistosFolder->FindObjectAny(name);
   else
      hist = (TH2X *) ((TFolder *) gHistoFolderStack->Last())->FindObjectAny(name);

   if (hist == NULL) {
      hist = new TH2X(name, title, xbins, xmin, xmax, ybins, ymin, ymax);
      if (!gHistoFolderStack->Last())
         gManaHistosFolder->Add(hist);
      else
         ((TFolder *) gHistoFolderStack->Last())->Add(hist);
   }

   return hist;
}

template < typename TH2X >
    TH2X EXPRT * h2_book(const char *name, const char *title,
                         int xbins, double xmin, double xmax, int ybins, double yedges[])
{
   TH2X *hist;

   /* check if histo already exists */
   if (!gHistoFolderStack->Last())
      hist = (TH2X *) gManaHistosFolder->FindObjectAny(name);
   else
      hist = (TH2X *) ((TFolder *) gHistoFolderStack->Last())->FindObjectAny(name);

   if (hist == NULL) {
      hist = new TH2X(name, title, xbins, xmin, xmax, ybins, yedges);
      if (!gHistoFolderStack->Last())
         gManaHistosFolder->Add(hist);
      else
         ((TFolder *) gHistoFolderStack->Last())->Add(hist);
   }

   return hist;
}

template < typename TH2X >
    TH2X EXPRT * h2_book(const char *name, const char *title,
                         int xbins, double xedges[], int ybins, double ymin, double ymax)
{
   TH2X *hist;

   /* check if histo already exists */
   if (!gHistoFolderStack->Last())
      hist = (TH2X *) gManaHistosFolder->FindObjectAny(name);
   else
      hist = (TH2X *) ((TFolder *) gHistoFolderStack->Last())->FindObjectAny(name);

   if (hist == NULL) {
      hist = new TH2X(name, title, xbins, xedges, ybins, ymin, ymax);
      if (!gHistoFolderStack->Last())
         gManaHistosFolder->Add(hist);
      else
         ((TFolder *) gHistoFolderStack->Last())->Add(hist);
   }

   return hist;
}

template < typename TH2X >
    TH2X EXPRT * h2_book(const char *name, const char *title,
                         int xbins, double xedges[], int ybins, double yedges[])
{
   TH2X *hist;

   /* check if histo already exists */
   if (!gHistoFolderStack->Last())
      hist = (TH2X *) gManaHistosFolder->FindObjectAny(name);
   else
      hist = (TH2X *) ((TFolder *) gHistoFolderStack->Last())->FindObjectAny(name);

   if (hist == NULL) {
      hist = new TH2X(name, title, xbins, xedges, ybins, yedges);
      if (!gHistoFolderStack->Last())
         gManaHistosFolder->Add(hist);
      else
         ((TFolder *) gHistoFolderStack->Last())->Add(hist);
   }

   return hist;
}

   /*
    * the following two macros allow for simple fortran like usage
    * for the most common histo types
    */
#define H1_BOOK(n,t,b,min,max) (h1_book<TH1F>(n,t,b,min,max))
#define H2_BOOK(n,t,xb,xmin,xmax,yb,ymin,ymax) (h2_book<TH2F>(n,t,xb,xmin,xmax,yb,ymin,ymax))

//TCutG *cut_book(const char *name);
#endif
#endif                          /* USE_ROOT */

#endif /* rmana_h defined */

