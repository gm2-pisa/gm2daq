// AMC13 header class for inheriting AMC13Simple class
#ifndef AMC13_AMC13_HH_INCLUDED
#define AMC13_AMC13_HH_INCLUDED 1

#include <string>
#include "amc13/AMC13Simple.hh"
#include "amc13/Flash.hh"
//#include "amc13/Version.hh"
#include "uhal/uhal.hpp"
#include "amc13/Status.hh"

namespace amc13{
  
  /// Main control class for AMC13
  class AMC13: public AMC13Simple{
  public:

    /*! \brief Constructor using a connection file on disk
      \param ConnectionMap path to XML connection file specifying two ids "T1" and "T2" with address tables
    */
    AMC13( const std::string& ConnectionMap )
      : AMC13Simple( ConnectionMap, "T1", "T2" )
    {
      initAMC13();
    }
      /*! \brief Constructor using a connection file on disk
      \param ConnectionMap path to XML connection file specifying two ids t1id and t2id with address tables
    */
    AMC13( const std::string& ConnectionMap, const std::string& t1id, const std::string& t2id )
      : AMC13Simple( ConnectionMap, t1id, t2id )
    {
      initAMC13();
    }  
    /*! \brief Constructor using user-provided URI and address tables
      \param p_URI1 URI for T1 board
      \param p_AddMap1 XML address table path for T1 board
      \param p_URI2 URI for T2 board
      \param p_AddMap2 XML address table path for T1 board
    */
    AMC13(const std::string& p_URI1, const std::string& p_AddMap1 ,
	  const std::string& p_URI2, const std::string& p_AddMap2)
      : AMC13Simple( p_URI1 , p_AddMap1, p_URI2, p_AddMap2 ) 
    {
      initAMC13(); 
    }
    /*! \brief Constructor using user-provided HwInterface objects 
      \param t1 HwInterface for T1 board
      \param t2 HwInterface for T2 board
     */
    AMC13(const uhal::HwInterface& t1, const uhal::HwInterface& t2)
      : AMC13Simple( t1, t2 )
    {
      initAMC13(); 
    }
    
    /// Destructor
    ~AMC13() {
      if(flash != NULL){
	delete flash ;
      }
    }

    // Initialize AMC13
    void initAMC13() {
      fprintf(stderr,"Using AMC13 software ver:%d\n",Version);
      flash = new Flash( this ) ;
      status = new Status( this ,Version);
      // additional constructor stuff for AMC13 (if needed)
    }

    /// Accessor for Flash interface
    Flash* getFlash() {
      return flash ;
    }

    /// Accessor for Flash interface
    Status* getStatus() {
      return status ;
    }

    // Control Functions
    /// General reset to tongue int chip (arg 0)
    void reset(AMC13Simple::Board chip) ;

    /// Reset counters on T1
    void resetCounters();

    /// Reset enabled DAQ links (call sfpOutputEnable() first)
    void resetDAQ();
    //Initialization
    /*! \brief Enable AMC13 inputs using 12-bit mask
      \param mask 12 bit enable mask (bit 0 = AMC1 slot) */
    void AMCInputEnable(uint32_t mask) ;
    /*! \brief Parse a comma-delimited list of inputs and convert to a mask
     * \param list list of input numbers separated by commas.  May include ranges, e.g. "1-4,6,9-12"
     * \param slotbased if true, count is 1-based
    */
    uint32_t parseInputEnableList(std::string list, bool slotbased);
    /*! \brief Force all AMC13 backplane TTC outputs enabled
     *
     * The AMC13 powers up with the LVDS TTC outputs disabled.  Usually the TTC outputs
     * for specific AMC cards are enabled when AMCInputEnable() is called.
     * However, this function will override the input link settings and force all
     * TTC data and clock outputs to be enabled.
     */
    void enableAllTTC() ;
    /// Enables/disables the S-Link Express DAQ Link (also need to call sfpOutputEnable to enable 1-3 of the outputs
    void daqLinkEnable(bool) ;
    /// Enables/disabled fake data generation in the AMC13 event builder
    void fakeDataEnable(bool) ;
    /*! \brief Enables/disables the locally generated TTC loop-back fiber signal
     *
     * When enabled, the TTS output (transmitter side of bottom SFP) sends 160MHz simulated
     * TTC signal, which may be looped-back into the TTC input to provide a local clock.
     * When disabled, the TTS output operates at 400Mb/s using the TCDS protocol.
     *
     * This also enables Local L1A as no external L1A may be received.
     */
    void localTtcSignalEnable(bool) ;

    //-------- Local Trigger control ------------

    /*! \brief Configure locally-generated triggers
     *
     * The AMC13 can generate internal L1A for testing.  These L1A cause
     * event-building within the AMC13, and are also transmitted to all AMC cards
     * in the crate over the MicroTCA backplane.
     *
     * \param ena enable local L1A feature.  Note:  does not start the
     * generator running, but merely enables and configures it.
     * Use sendL1ABurst() or startContinuousL1A() to actually generate triggers.
     * \param mode repeat mode
     *   - mode 0 = periodic trigger every <em>rate</em> orbits at BX=500
     *   - mode 1 = periodic trigger every <em>rate</em> BX
     *   - mode 2 = random trigger at <em>rate</em> Hz
     * \param burst number of triggers to send in a burst. The burst rate is
     *    controlled by mode and rate.  Value may be 1-4096. 
     * \param rate sets the rate based on mode.  Value may be 1-65536. (or 2-131072 for random mode)
     * \param rules determines which of the CMS trigger rules are enforced
     *   - 0 means enforce all rules (1-4)
     *   - 1 means all except rule 4
     *   - 2 means enforce rules 1 and 2
     *   - 3 means enforce only rule 1
     *
     * For reference the trigger rules are:
     *   - Rule 1:  No more than 1 L1A per 3 BX (75ns)
     *   - Rule 2:  No more than 2 L1A per 25 BX (625ns)
     *   - Rule 3:  No more than 3 L1A per 100 BX (2.5us)
     *   - Rule 4:  No more than 4 L1A per 240 BX (6us)
     *
     * When local L1A are enabled, the external TTC input is still active
     * (unless localTtcSignalEnable() is set true).  L1A from the TTC input
     * are ignored but the B channel from the TTC input is unchanged,
     * so external BC0 and other commands are passed through.
     *
     * This function does not disable triggers if they are active,
     * so it may be used to change the rate of continuous triggers
     * on-the-fly. 
     *
     * The default settings at power-up are equivalent to:
     * configureLocalL1A( false, 0, 1, 1, 0)
     */
    void configureLocalL1A( bool ena, int mode, uint32_t burst, uint32_t rate, int rules);
    /// Enable or disable local L1A generator (configure with configureLocalL1a()
    void enableLocalL1A( bool ena);
    /// Start sending local L1A continuously (configured with configureLocalL1a())
    void startContinuousL1A() ;
    /// Stop sending local L1A continuously
    void stopContinuousL1A() ;
    /// Send a single burst of L1A (configured with configureLocalL1a())
    void sendL1ABurst() ;
    /*! \brief Enables/disables SDRAM backpressure on the AMC13
     *
     * When set to true, the AMC13 event builder will stop when the SDRAM monitor buffer
     * becomes full.  Otherwise event building continues as normal and events are discarded
     */
    void monBufBackPressEnable(bool) ;
    /*! \brief set start/end BCN for orbit gap.  Locally-generated L1A will not
     * occur within the gap.
     *
     * \param begin start of gap 0..0xdea (not included)
     * \param end end of gap 0..0xdeb (included)
     *
     * If begin=end the checking is disabled.
     */
    void setOrbitGap( uint32_t begin, uint32_t end);
    /*! \brief Sends locally-generated TTC broadcast commands to the backplane
     *
     * ECR (event count reset) sends a TTC broadcast command 0x02 over the
     * TTC network to the connected AMC cards.
     *
     * OCR (orbit count reset) sends a TTC broadcast command configured by
     * setOcrCommand().  The default value for OCR is 0x28 as used by HCAL.
     *
     * \param a send ECR (event count reset) if true
     * \param b send OCR (orbit count reset) if true
     */
    void sendLocalEvnOrnReset(uint32_t a, uint32_t b) ;
    /*! \brief Set TTC broadcast command used for OCR (orbit count reset)
     *
     * The default value for OCR is 0x28 as used by HCAL.
     * The low 2 bits of broadcast command are reserved for BCR (bit 0)
     * and ECR (bit 1) so only the upper 6 bits of this register are used.
     *
     * \param cmd 8-bit command (only upper 6 bits used)
     */
    void setOcrCommand( uint32_t cmd);
    /*! \brief setup AMC13 prescaling
     *
     * The AMC13 supports two methods of pre-scaling events which are stored in the
     * SDRAM monitor buffer for local DAQ readout over IPBus.
     *
     * The first method ("simple" prescaling) simply saves every N events
     * where can range from 1 (save every event) to 0x10000 (save every 2^16 events)
     *
     * The second method saves events where the EvN has a configurable number of zeroes
     * in the least-significant bits.  The number of zeroes can be set from 5 to 20.
     * When set to 5, every 2^5 (32) events are saved.
     *
     * To disable prescaling (the default) set mode=0 and n=1
     *
     * \param mode set to 0 for simple prescaling, 1 for EvN match
     * \param n prescale factor from 1...0x10000 for simple prescaling (when mode=0)
     * or number of zeroes to match in low bits of EvN in range 5...20 (when mode=1)
     */
    void configurePrescale( int mode, uint32_t n);
    /*! \brief Set AMC13 FED ID (12 bit FED number send in header)
     *
     * This function sets the 12-bit FED ID sent as the "Source_id"
     * in the CDF event header.  Not to be confused with the 16-bit
     * value set by setSLinkID() which is the physical link ID
     *
     * \param id FED ID
     */
    void setFEDid(uint32_t id) ;
    /*! \brief set Slink Express physical link ID
     *
     * This function sets the 16-bit physical link ID for the S-Link Express
     * DAQ fiber output.  Only the upper 14 bits are programmable.  The lower
     * 2 bits specify the output number on the AMC13 where
     * "00", "01", "10" => ( SFP0, SFP1, SFP2).
     *
     * \param id SLINK Express link id
     */
    void setSlinkID( uint32_t id) ;
    /// Set BcN offset (between AMC13 and AMC)
    void setBcnOffset( uint32_t offset) ;
    /// Set TTS disable mask
    void ttsDisableMask( uint32_t mask) ;
    /// Enables SFP Outputs (SFP 2,1,0)
    void sfpOutputEnable(uint32_t mask) ;
    /// Puts the AMC13 into run mode
    void startRun();
    /// Takes the AMC13 out of run mode
    void endRun() ;
    /// Read one event from SDRAM into heap buffer
    uint64_t* readEvent( size_t& nwords, int& rc);
    /// Read one event from SDRAM into vector
    std::vector<uint64_t> readEvent();
    /// Retrieve enabled AMCs mask
    uint16_t GetEnabledAMCMask( bool readFromBoard = false );
    /*! \brief Enable/disable T2 TTC command history monitor
     *
     * This function controls a 512-word TTC command history monitor
     * in T2.  This monitors the TTC stream as received by T2
     * (either from the front-panel fiber or internally-generated).
     * A filter is available to inhibit storing commands (e.g. BC0)
     * which are not of interest.
     *
     * For each TTC command which passes the filter, the EvN, BcN,
     * OrN and command value are stored.
     *
     * \param ena enable the history monitor or filter
     */
    void setTTCHistoryEna( bool ena);
    /// Enable/disable T2 TTC command history filter
    void setTTCFilterEna( bool ena);
    /*! \brief Set T2 TTC command history filter list item
     *
     * Set one of the 16 possible commands to be filtered out
     * of the TTC history collected on T2.
     *
     * \param n history item 0-15
     * \param filterVal bits 0-7 are 8-bit command value to exclude from history
     * bits 8-15 are mask if 1, corresponding bit[N-8] of the TTC command
     * will be ignored in the matching, bit 16 = '1' to enable this list item */
    void setTTCHistoryFilter( int n, uint32_t filterVal);
    /*! Read T2 TTC command history filter list item
     *
     * \param n history item 0-15
     * returns filter list value, bits 0-7 are 8-bit command value to exclude from history
     * bits 8-15 are mask if 1, corresponding bit[N-8] of the TTC command
     * will be ignored in the matching, bit 16='1' if this item is enabled */
    uint32_t getTTCHistoryFilter( int n);
    /// Clear TTC filter list
    void clearTTCHistoryFilter();
    /// Clear TTC command history
    void clearTTCHistory();
    /*! \brief Read T2 TTC command history
     *
     * Retrieve the stored history of last N TTC commands.
     * Each command is stored in four successive 32-bit words as follows:
     *
     * word 0 - bit 0-7 is the TTC command
     *
     * word 1 - bit 0-31 is the orbit number
     *
     * word 2 - bit 0-11 is the bunch crossing number
     *
     * word 3 - bit 0-23 is the event number of the most recent L1A
     *
     * The caller must supply a buffer with 4*N 32-bit words available
     * and a maximum number of commands to retrieve.  The function will
     * return the number of commands retrieved.  The history capture
     * WILL BE DISABLED by a call to this function.  If there is no history,
     * a value of 0 will be returned and the caller's buffer will be unchanged.
     *
     * \param buffer pointer to user-supplied buffer of <nhist>*16 bytes
     * \param nhist number of commands to retrieve
     */
    void getTTCHistory( uint32_t* buffer, int nhist);
    /*! \brief Retrieve most recent nhist entries (up to 512) in the TTC history buffer
     *
     * Note that this may be rapidly changing; best disable capture before reading.
     * If nhist > number of entries stored, throw an exception
     */
    int getTTCHistoryCount();

    /// Retrieve software revision (SVN revision number)
    int GetVersion(){return Version;}
    
  private:
    // Do Not use default constructor
    AMC13();
    
    // Prevent copying of AMC13 objects
    AMC13( const AMC13& other) ; // prevents construction-copy
    AMC13& operator=( const AMC13&) ; // prevents copying
    
    // HwInterface objects - why here, already in AMC13Simple?
    //    uhal::HwInterface* m_T1 ;
    //    uhal::HwInterface* m_T2 ;
    
    // AMC13 flash object
    Flash * flash ;

    //status generator
    Status * status;

    static const int Version;
    
    //Saved mask from AMCInputEnabled
    uint16_t m_enabledAMCMask;
    
    // compute address of TTC history item from offset -1 to -512
    uint32_t getTTCHistoryItemAddress( int item);

  };
}
#endif
  
