void Qplot()
{

  /*****************************************************************/
  // Prepare the canvas
  gStyle->SetOptStat("ne"); 
  TCanvas *canvas = (TCanvas *) gROOT->GetListOfCanvases()->At(0);
  canvas->Divide(7,5,0.001,0.001,10);
  /*****************************************************************/
  
  unsigned int NUMBER_OF_SEGMENTS = 35;
  
  int ipad = 1;
<<<<<<< HEAD
  char name[256];

  for(int iseg=0; iseg<NUMBER_OF_SEGMENTS; iseg++){
    canvas->cd(ipad++);
    sprintf(name,"Qplotter/h1_Qmethod_Seg%i",iseg+1);
=======

  for(iseg=0; iseg<NUMBER_OF_SEGMENTS; iseg++){
    canvas->cd(ipad++);
    sprintf(name,"TQplotter/h1_Qmethod_Seg%i",iSeg+1);
>>>>>>> origin/develop
    TH1D *h1 = (TH1D*)getObject(name);
    if (! h1)
      {
	printf("***ERROR! Cannot retrieve histogram [%s]\n",name);
	continue;
      }
    h1->SetLineColor(4);
<<<<<<< HEAD
    h1->GetXaxis()->SetRangeUser(0,350000);
=======
>>>>>>> origin/develop
    h1->Draw();
  }

}
