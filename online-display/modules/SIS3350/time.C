
void time()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(8,4);
  /*****************************************************************/
  
  char name[256];
  
  unsigned int SIS3350_NUMBER_OF_CRATES = 1;
  unsigned int SIS3350_NUMBER_OF_BOARDS_PER_CRATE = 2;
  unsigned int SIS3350_NUMBER_OF_CHANNELS = 4;
  
  unsigned int icrate,iboard,ichannel;

  int ipad = 1;

  // Defna time relative to the start detector
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"sis3350_defna_hist/h1_defna_dt_start_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
	  TH1D *h1 = (TH1D*)getObject(name);
	  if ( !h1 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h1->SetLineColor( kBlue );
	  h1->SetLineWidth( 2 );
	  h1->Draw();
	}

  // raw time relative to the start detector
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"sis3350_defna_hist/h1_dt_start_raw_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
	  TH1D *h1 = (TH1D*)getObject(name);
	  if ( !h1 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h1->SetLineColor( kBlue );
	  h1->SetLineWidth( 2 );
	  h1->Draw();
	}


  // autocorrelation
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"sis3350_defna_hist/h1_defna_autocor_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
	  TH1D *h1 = (TH1D*)getObject(name);
	  if ( !h1 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h1->SetLineColor( kBlue );
	  h1->SetLineWidth( 2 );
	  h1->Draw();
	}




}
