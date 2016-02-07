
void raw()
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

  // waveforms
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"sis3350_hist_raw/h2_wf_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
	  TH2D *h2 = (TH2D*)getObject(name);
	  if ( !h2 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h2->Draw("ColZ");
	}

  // ADCmin
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"sis3350_hist_raw/h1_ADCmin_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
	  TH1D *h1 = (TH1D*)getObject(name);
	  if ( !h1 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h1->SetLineWidth(2);
	  h1->SetLineColor( kBlue );
	  h1->Draw();
	}

  // ADCmax
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  canvas->cd(ipad++);
	  sprintf(name,"sis3350_hist_raw/h1_ADCmax_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
	  TH1D *h1 = (TH1D*)getObject(name);
	  if ( !h1 )
	    {
	      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	      continue;
	    }
	  h1->SetLineWidth(2);
	  h1->SetLineColor( kBlue );
	  h1->Draw();
	}


  
  // Number of packets vs. spill number
  canvas->cd(ipad++);
  TGraph *gr;
  gr = (TGraph*)getObject("sis3350_packet/gr_n_packets_crate_01_board_01");
  if ( gr )
    {
      gr->SetLineColor( kBlue );
      gr->SetLineWidth( 6 );
      gr->Draw("APL");
    

      gr = (TGraph*)getObject("sis3350_packet/gr_n_packets_crate_01_board_02");
      if ( gr )
	{
	  gr->SetLineColor( kRed );
	  gr->SetLineWidth( 2 );
	  gr->Draw("PL");
	}
    }


  // Number of triggers vs. spill number
  canvas->cd(ipad++);
  char *draw_opt = "APL";
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      for (ichannel=1; ichannel<=SIS3350_NUMBER_OF_CHANNELS; ichannel++)
	{
	  sprintf(name,"sis3350_packet/gr_n_triggers_crate_%02i_board_%02i_channel_%i",icrate,iboard,ichannel);
	  gr = (TGraph*)getObject(name);
	  if ( ! gr )
	    {
	      // try VME module
	      sprintf(name,"sis3350_vme/gr_n_triggers_crate_%02i_board_%02i_channel_%i",icrate,iboard,ichannel);
	      gr = (TGraph*)getObject(name); 
	    }
	  if ( gr )
	    {
	      gr->SetLineColor( kRed );
	      gr->SetLineWidth( 2 );
	      gr->Draw( draw_opt );
	      draw_opt = "PL"; 
	      if ( iboard == 1 )
		{
		  gr->SetLineColor( kBlue );
		  gr->SetLineWidth( 6 );
		}
	    } 
	}

  // bank size vs. event number
  canvas->cd(ipad++);
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    {
      sprintf(name,"sis3350_packet/gr_bk_size_crate_%02i",icrate);
      gr = (TGraph*)getObject(name);
      if ( ! gr )
	{
	  // try VME module
	  sprintf(name,"sis3350_vme/gr_bk_size_crate_%02i",icrate);
	  gr = (TGraph*)getObject(name); 
	}
      if ( gr )
	{
	  gr->SetLineColor( kBlue );
	  gr->SetLineWidth( 2 );
	  gr->Draw( "APL" );
	}	        
    }


  // ADC data size vs. spill number
  canvas->cd(ipad++);
  char *draw_opt = "APL";
  for (icrate=1; icrate<=SIS3350_NUMBER_OF_CRATES; icrate++)
    for (iboard=1; iboard<=SIS3350_NUMBER_OF_BOARDS_PER_CRATE; iboard++)
      {
	sprintf(name,"sis3350_packet/gr_ADClen_crate_%02i_board_%02i",icrate,iboard);
	gr = (TGraph*)getObject(name);
	if ( ! gr )
	  {
	    // Try VME module
	    sprintf(name,"sis3350_vme/gr_ADClen_crate_%02i_board_%02i",icrate,iboard);
	    gr = (TGraph*)getObject(name);
	  }
	if ( gr )
	  {
	    gr->SetLineColor( kRed );
	    gr->SetLineWidth( 2 );
	    gr->Draw( draw_opt );
	    draw_opt = "PL"; 
	    if ( iboard == 1 )
	      {
		gr->SetLineColor( kBlue );
		gr->SetLineWidth( 6 );
	      }
	  } 
      }



}
