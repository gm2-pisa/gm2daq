
void Energy()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(2,1,0.001,0.001,10);
  /*****************************************************************/
  
  char name[256];
  
  const unsigned int kBL_B =1;
  const unsigned int kBL_C =1;
  const unsigned int kBR_B =1;
  const unsigned int kBR_C =2;

  const unsigned int kML_B =1;
  const unsigned int kML_C =3;
  const unsigned int kMC_B =1;
  const unsigned int kMC_C =4;
  const unsigned int kMR_B =2;
  const unsigned int kMR_C =1;

  const unsigned int kTL_B =2;
  const unsigned int kTL_C =2;
  const unsigned int kTR_B =2;
  const unsigned int kTR_C =3;

  const unsigned int kSiPM_B = kTR_B;
  const unsigned int kSiPM_C = kTR_C;
  const unsigned int kR9800_B = kMC_B;
  const unsigned int kR9800_C = kMC_C;

  // SET PARAMETERS BELOW HERE 
  unsigned int icrate   = 1;
  unsigned int iboard   = kSiPM_B;
  unsigned int ichannel = kSiPM_C;
  
  int rebinParameter = 16;

  int binsLeftOfPeak = 4;
  int binsRightOfPeak = 30;
  int binsToShow = 150;

  int ipad = 1;
    gStyle->SetOptFit(1111);
  canvas->cd(ipad++);
  sprintf(name,"sis3350_defna/h1_defna_area_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
  TH1D *h1 = (TH1D*)getObject(name);
  if ( !h1 )
    {
      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
    }
  else
    {
      h1->Rebin(rebinParameter);
      //      h1->GetXaxis()->SetRangeUser(4000,14000);
      h1->SetLineColor( kBlue );
      h1->SetLineWidth( 2 );
      h1->Draw();      

      
      TF1* user = new TF1("user","[0]*exp(-0.5*((x-[1])/[2])**2)+[3]",0,100000);
      

      double peak = h1->GetMaximumBin();
      printf("%f\n",h1->GetMaximum());

      double meanguess = h1->GetBinCenter(peak);
      user->SetParameters(0.025*meanguess,meanguess,0.15*meanguess,1);
      user->SetParNames("Amp","Mean","Sigma","Bkg");

      h1->Fit("user","","",h1->GetBinCenter(peak-binsLeftOfPeak),h1->GetBinCenter(peak+binsRightOfPeak));
      
      h1->GetXaxis()->SetRange(peak-binsToShow,peak+binsToShow);
      printf("Mean/Sigma = (%f/%f)=%f\n",user->GetParameter(2), user->GetParameter(1),user->GetParameter(2)/user->GetParameter(1));
    }

  TVirtualPad *pad = canvas->cd(ipad++);
  sprintf(name,"sis3350_hist_raw/h2_wf_crate_%02d_board_%02d_channel_%d",icrate,iboard,ichannel);
  TH2D *h2 = (TH2D*)getObject(name);
  if ( !h2 )
    {
      printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
    }
  else
    {
      h2->Draw("ColZ");
      pad->SetLogz();
    }


 
}
