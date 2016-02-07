//ROOT module for slac vme sis3350 online display
//by Wes Gohn
//11.22.13

void vmeTree()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(2,4);
  /*****************************************************************/

  char name[256];

  unsigned int SIS3350_NUMBER_OF_XTALS = 8;
 
  unsigned int ix;

  int ipad = 1;

  sprintf(name,"t",0);

  // waveforms
  TTree* vmeTr = (TTree*)getObject(name);
  if( !vmeTr ){
    printf("***ERROR! Cannot retrieve TTree [%s]\n",name);
    continue;
  }
  for (ix=0; ix<SIS3350_NUMBER_OF_XTALS; ix++)
    {
      canvas->cd(ipad++);
    
      vmeTr->Draw(Form("sis.trace[%i]:Iteration$",ix), "Entry$<100");
    }
  
}
