{
  /*****************************************************************/
  // Prepare the canvas

  gROOT->LoadMacro("palette.cxx");
  SetRainbowPalette();
  gStyle->SetOptStat("ne");
 
  TCanvas *MuCapCanvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  MuCapCanvas->Divide(3,3);

  /*****************************************************************/

  MuCapCanvas->cd(1);

  //getHistFromSocket("")
  //TFolder *histos = (TFolder*) getPointer("histos");
  //TFolder *histos = (TFolder*) getPointer("h1_test");
  //cout << histos << endl;
  //printf("[%d]\n",histos);

  TH1 *h1 = getHist("sis3350_packet/h1_test");
  if ( h1 )
    {
      h1->Draw();
    }
  
  TGraph *gr = (TGraph*)getHist("sis3350_packet/gr_n_packets_crate_01_board_01");
  if ( gr )
    {
      MuCapCanvas->cd(2);
      gr->SetLineColor( kBlue );
      gr->SetLineWidth( 3 );
      gr->Draw("APL");
    }

  return;
  

  TH1 *parameters_muSC = getHist("muSC_count_raw");
  parameters_muSC->SetFillColor(kBlack);
  parameters_muSC->Draw();

  MuCapCanvas->cd(2);
  gPad->SetLogy();
  TH1 *parameters_muPC1 = getHist("h1_test");
  
  //parameters_muPC1->Draw();

  return;

  parameters_muPC1->SetFillColor(kBlack);
  parameters_muPC1->Draw();

#if 0
  MuCapCanvas->cd(3);
  gPad->SetLogy();
  TH1 *caen0_32BitInterpolator = getHist("DAQ_caen0_edges_leading_32BitInterp_noRollover");
  caen0_32BitInterpolator->SetLineColor(kBlue);
  TH1 *caen1_32BitInterpolator = getHist("DAQ_caen1_edges_leading_32BitInterp_noRollover");
  caen1_32BitInterpolator->SetLineColor(kRed);
  TH1 *caen2_32BitInterpolator = getHist("DAQ_caen2_edges_leading_32BitInterp_noRollover");
  caen2_32BitInterpolator->SetLineColor(kGreen);

  TH1 *biggest;
  TH1 *smaller1;
  TH1 *smaller2;
  
  if ((caen0_32BitInterpolator->Integral() > caen1_32BitInterpolator->Integral()) &&
      (caen0_32BitInterpolator->Integral() > caen2_32BitInterpolator->Integral())) {
    biggest = caen0_32BitInterpolator;
    smaller1 = caen1_32BitInterpolator;
    smaller2 = caen2_32BitInterpolator;
  } else {
    smaller1 = caen0_32BitInterpolator;
    if (caen1_32BitInterpolator->Integral() > caen2_32BitInterpolator->Integral()) {
      biggest = caen1_32BitInterpolator;
      smaller2 =  caen2_32BitInterpolator;
    } else {
      biggest = caen2_32BitInterpolator;
      smaller2 =  caen1_32BitInterpolator;
    }
  }
  biggest->SetTitle("CAEN Interpolator Distributions");
  biggest->SetMinimum(1); 
  biggest->Draw();
  smaller1->Draw("same");
  smaller2->Draw("same");
  TText *txt0 = new TText;
  txt0->SetTextColor(kBlue);
  txt0->DrawTextNDC(.7, .25, "CAEN 0");
  TText *txt1 = new TText;
  txt1->SetTextColor(kRed);
  txt1->DrawTextNDC(.7, .20, "CAEN 1");
  TText *txt2 = new TText;
  txt2->SetTextColor(kGreen);
  txt2->DrawTextNDC(.7, .15, "CAEN 2");
#endif

  MuCapCanvas->cd(3);
  TH1 *blocks_total = getHist("blocks_total");
  TH1 *blocks_analyzed = getHist("blocks_analyzed");
  TH1 *blocks_skipped_duplication = getHist("blocks_skipped_duplication");
  TH1 *blocks_skipped_caen_error = getHist("blocks_skipped_caen_error");
  TH1 *blocks_skipped_comp_error = getHist("blocks_skipped_comp_error");
  TH1 *blocks_skipped_caen_rollover_error = getHist("blocks_skipped_caen_rollover_error");
  TH1 *blocks_skipped_tdc400_time_order_error = getHist("blocks_skipped_tdc400_time_order_error");
  TH1 *blocks_skipped_tdc400_DAQ_flagged_error = getHist("blocks_skipped_tdc400_DAQ_flagged_error");
  TH1 *blocks_skipped_stack_bank_overflow = getHist("blocks_skipped_stack_bank_overflow");
  TH1 *blocks_skipped_muSC_matching_errors = getHist("blocks_skipped_muSC_matching_errors");
  TH1 *blocks_skipped_eSC_caen_only = getHist("blocks_skipped_eSC_caen_only");
  TH1 *blocks_skipped_eSC_comp_only = getHist("blocks_skipped_eSC_comp_only");
  TH1 *blocks_skipped_global_sparks = getHist("blocks_skipped_global_sparks");

  TH1 *blocks_skipped_all = new TH1D("blocks_skipped_all", "blocks_skipped_all", 11, 0.5, 11.5);
  TH1 *blocks_total2 = new TH1D("blocks_total2", "blocks_total2", 11, 0.5, 11.5);
  blocks_total2->SetBinContent(1, blocks_total->Integral());
  blocks_skipped_all->SetBinContent(2, blocks_skipped_caen_error->Integral());
  blocks_skipped_all->SetBinContent(3, blocks_skipped_comp_error->Integral());
  blocks_skipped_all->SetBinContent(4, blocks_skipped_caen_rollover_error->Integral());
  blocks_skipped_all->SetBinContent(5, blocks_skipped_tdc400_time_order_error->Integral());
  blocks_skipped_all->SetBinContent(6, blocks_skipped_tdc400_DAQ_flagged_error->Integral());
  blocks_skipped_all->SetBinContent(7, blocks_skipped_stack_bank_overflow->Integral());
  blocks_skipped_all->SetBinContent(8, blocks_skipped_muSC_matching_errors->Integral());
  blocks_skipped_all->SetBinContent(9, blocks_skipped_eSC_caen_only->Integral());
  blocks_skipped_all->SetBinContent(10, blocks_skipped_eSC_comp_only->Integral());
  blocks_skipped_all->SetBinContent(11, blocks_skipped_global_sparks->Integral());
  blocks_skipped_all->SetLineColor(kRed);

  blocks_total2->SetTitle("Blocks: total (black), cut by analysis (red)");
  blocks_total2->Draw();
  blocks_total2->GetXaxis()->SetBinLabel(1, "Total");
  blocks_total2->GetXaxis()->SetBinLabel(2, "CAEN");
  blocks_total2->GetXaxis()->SetBinLabel(3, "COMP");
  blocks_total2->GetXaxis()->SetBinLabel(4, "Rollover");
  blocks_total2->GetXaxis()->SetBinLabel(5, "TDC400 order");
  blocks_total2->GetXaxis()->SetBinLabel(6, "TDC400 DAQ");
  blocks_total2->GetXaxis()->SetBinLabel(7, "Stack");
  blocks_total2->GetXaxis()->SetBinLabel(8, "muSC");
  blocks_total2->GetXaxis()->SetBinLabel(9, "eSC CAEN");
  blocks_total2->GetXaxis()->SetBinLabel(10, "eSC COMP");
  blocks_total2->GetXaxis()->SetBinLabel(11, "Spark");
  blocks_skipped_all->Draw("same");

  MuCapCanvas->cd(4);
   gPad->SetLogy();
  TH1 *TPC_Thresh0AnodeHits = getHist("TPC_Thresh0AnodeHits");
  TH1 *TPC_Thresh1AnodeHits = getHist("TPC_Thresh1AnodeHits");
  TH1 *TPC_Thresh2AnodeHits = getHist("TPC_Thresh2AnodeHits");
  TPC_Thresh0AnodeHits->SetLineColor(kGreen);
  TPC_Thresh0AnodeHits->SetTitle("TPC Anode Hits");
  TPC_Thresh0AnodeHits->SetMinimum(1);
  TPC_Thresh0AnodeHits->Draw();
  TPC_Thresh1AnodeHits->SetLineColor(kBlue);
  TPC_Thresh1AnodeHits->Draw("same");
  TPC_Thresh2AnodeHits->SetLineColor(kRed);
  TPC_Thresh2AnodeHits->Draw("same");

  double thresh0_int = TPC_Thresh0AnodeHits->Integral(12,64);
  double thresh1_int = TPC_Thresh1AnodeHits->Integral(12,64);
  double thresh2_int = TPC_Thresh2AnodeHits->Integral(12,64);


printf("integrals from 12 to 64 were : %lf, %lf and %lf \n",thresh0_int/(52), thresh1_int/52, thresh2_int/52);
if (thresh1_int>0)
  printf("red/blue = %lf*10^-3\n",1000*thresh2_int/thresh1_int);


  MuCapCanvas->cd(5);
  gPad->SetLogy();
  TH1 *TPC_Thresh0CathodeHits = getHist("TPC_Thresh0CathodeHits");
  TH1 *TPC_Thresh1CathodeHits = getHist("TPC_Thresh1CathodeHits");
  TPC_Thresh0CathodeHits->SetTitle("TPC Cathode Hits");
  TPC_Thresh0CathodeHits->SetLineColor(kGreen);
  TPC_Thresh0CathodeHits->SetMinimum(1);
  TPC_Thresh0CathodeHits->Draw();
  TPC_Thresh1CathodeHits->SetLineColor(kBlue);
  TPC_Thresh1CathodeHits->Draw("same");

  MuCapCanvas->cd(6);
  gPad->SetLogy();
  TH1 *parameters_all = getHist("parameters_All");
  parameters_all->Draw();

  MuCapCanvas->cd(7);
  TH1 *parameters_ePC1 = getHist("parameters_ePC1");
  gPad->SetLogy();
  parameters_ePC1->SetFillColor(kBlack);
  parameters_ePC1->Draw();

  MuCapCanvas->cd(8);
  TH1 *parameters_ePC2 = getHist("parameters_ePC2");
  gPad->SetLogy();
  parameters_ePC2->SetFillColor(kBlack);
  parameters_ePC2->Draw();

  MuCapCanvas->cd(9);
  TH1 *parameters_hodoscope = getHist("parameters_eSC_caen");
  parameters_hodoscope->SetFillColor(kBlack);
  parameters_hodoscope->Draw();

  /*****************************************************************/
}
