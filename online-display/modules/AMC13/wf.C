
void wf()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(3,4);
  // canvas->SetLogz();
  /*****************************************************************/

  char name[256];

  unsigned int AMC13_NUMBER_OF_SHELVES = 1;
  unsigned int AMC13_NUMBER_OF_AMCS_PER_SHELF = 12;
  unsigned int AMC13_NUMBER_OF_CHANNELS = 1;

  unsigned int ishelf,iamc,ichannel;

  int ipad = 1;

  // waveforms
  for (ishelf=1; ishelf<=AMC13_NUMBER_OF_SHELVES; ishelf++)
    for (iamc=1; iamc<=AMC13_NUMBER_OF_AMCS_PER_SHELF; iamc++)
      for (ichannel=1; ichannel<=AMC13_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"amc13_hist_raw/h2_wf_shelf_%02d_amc_%02d_channel_%d",ishelf,iamc,ichannel);
	  TH2D *h2 = (TH2D*)getObject(name);
	  if ( !h2 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h2->Draw();
	  h2->GetYaxis()->SetRangeUser(0,5000);
	}





}
