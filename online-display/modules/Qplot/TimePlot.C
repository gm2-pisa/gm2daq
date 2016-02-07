void TimePlot()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(1,2,0.001,0.001,10);
  /*****************************************************************/
  
  
  char name1[256];
  char name2[256];

  sprintf(name1,"CaloReadoutTCPIP_RB/h2_dt_emulatorEOF");
  sprintf(name2,"CaloReadoutTCPIP_RB/h2_dt_tcpgotheader");

  for(int i=0; i<2; i++){
    canvas->cd(i+1);
    if(i==0) TH2D *h2 = (TH2D*)getObject(name1);
    if(i==0) TH2D *h2 = (TH2D*)getObject(name2);

    if (! h2)
      {
	printf("***ERROR! Cannot retrieve timing histogram\n");
	continue;
      }
    h2->SetMarkerColor(2);
    h2->Draw();
  }

}
