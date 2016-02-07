void TimePlot()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(1,2,0.001,0.001,10);
  /*****************************************************************/
  
  
  char name2[256];

  sprintf(name2,"CaloTime_AMC13/h2_dt_tcpgotheader");

  canvas->cd(1);
  TH2D *h2 = (TH2D*)getObject(name2);
  
  if (! h2)
    {
      printf("***ERROR! Cannot retrieve timing histogram\n");
      continue;
    }
  h2->SetMarkerColor(4);
  h2->GetXaxis()->SetRangeUser(1000,100000);
  h2->Draw();
  
  canvas->cd(2);
  TH1D *h0 = h2->ProjectionX("h0");
  h0->GetXaxis()->SetRangeUser(1000,100000);
  h0->SetLineColor(0);
  h0->Draw();
  TH1D *hp[6];

  for(int i=0;i<6;i++){
    hp[i] = h2->ProjectionX(Form("p%i",i),i+3,i+3);
    hp[i]->GetXaxis()->SetRangeUser(1000,100000);
    hp[i]->SetLineColor(i+1);
    //hp[i]->Draw();
    hp[i]->Draw("same");
    canvas->Update();
  }
  p1->SetLineColor(2);
  p2->SetLineColor(3);
  p3->SetLineColor(4);
  p4->SetLineColor(5);
  p5->SetLineColor(6);
  TLegend *leg = new TLegend(0.6,0.6,0.9,0.9);
  leg->AddEntry(p0,"tcp header","l");
  leg->AddEntry(p1,"tcp data","l");
  leg->AddEntry(p2,"copy to GPU","l");
  leg->AddEntry(p3,"GPU processing","l");
  leg->AddEntry(p4,"MFE start","l");
  leg->AddEntry(p5,"MFE stop","l");
  leg->SetTextSize(0.05);
  leg->Draw();
  
}
