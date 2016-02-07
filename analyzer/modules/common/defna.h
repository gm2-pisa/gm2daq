#ifndef defna_h
#define defna_h

/**
 * Defna parameters of pulses
 *
 *
 */
struct DEFNA {
  double time;       // pulse time (weighted average) + WFD block time
  int    pedestal;   // pedestal  
  int    amplitude;  // amplitude (pedestal subtructed)
  double area;       // pulse area
  int    width;      // pulse width (from threshold to threshold)
};

#endif // defna_h defined
