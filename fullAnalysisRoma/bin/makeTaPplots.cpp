// to compile: c++ -o makeTaPplots `root-config --cflags --glibs` makeTaPplots.cpp ../src/DiphotonUtils.cc
// to run: ./makeTaPplots


#include "TMath.h"
#include "TTree.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TFile.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPaveText.h"
#include <iostream>

#include "../interface/DiphotonUtils.h"

#define NVARIABLES 10
#define NCUTS 1

int main()
{

  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0); 
  gStyle->SetOptFit(111110); 
  gStyle->SetOptFile(1); 
  
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(1.0);
  gStyle->SetMarkerColor(1);

  DiphotonPlot *myPlot = new DiphotonPlot;
  myPlot->setStyle();

  TString plotsDir="../TaPplots/";

  TFile* fOut=new TFile("TaP_histos.root","RECREATE");
  
  char icut[NCUTS][100];
  TH1F* histos[NCUTS][NVARIABLES];    
  
  TString variables[NVARIABLES];
  variables[0]="invMass";
  variables[1]="electron_pt";
  variables[2]="electron_eta";
  variables[3]="electron_phi";
  variables[4]="gamma_pt";
  variables[5]="gamma_eta";
  variables[6]="gamma_phi";
  variables[7]="gamma_r9";
  variables[8]="gamma_sieie";
  variables[9]="gamma_scRawEne";

  TString units[NVARIABLES];
  units[0]="GeV/c^{2}";
  units[1]="GeV/c^{2}";
  units[2]="";
  units[3]="";
  units[4]="GeV/c^{2}";
  units[5]="";
  units[6]="";
  units[7]="";
  units[8]="";
  units[9]="GeV/c^{2}";
 
  int nbins[NVARIABLES];
  nbins[0]=200;
  nbins[1]=100;
  nbins[2]=100;
  nbins[3]=100;
  nbins[4]=100;
  nbins[5]=100;
  nbins[6]=100;
  nbins[7]=100;
  nbins[8]=100;
  nbins[9]=100;

  float range[NVARIABLES][2]; // N variables, min, max
  // invMass
  range[0][0]=60.;
  range[0][1]=120.;
  // ele_pt
  range[1][0]=0.;
  range[1][1]=200.;
  //ele_eta
  range[2][0]=-3;
  range[2][1]=3.; 
  //ele_phi
  range[3][0]=-4.;
  range[3][1]=4.; 
  // gamma_pt
  range[4][0]=0.;
  range[4][1]=200.;
  //gamma_eta
  range[5][0]=-3.;
  range[5][1]=3.;
  //gamma_phi
  range[6][0]=-4.;
  range[6][1]=4.;
  //gamma_r9
  range[7][0]=0.;
  range[7][1]=1.1;
   //gamma_sieie
  range[8][0]=0.;
  range[8][1]=0.04;
   //gammascRawEne
  range[9][0]=0.;
  range[9][1]=600.;
   
  TString xaxisLabel[NVARIABLES];
  xaxisLabel[0]="invariant mass";
  xaxisLabel[1]="p_{T}(e)";
  xaxisLabel[2]="#eta(e)";
  xaxisLabel[3]="#phi(e)";
  xaxisLabel[4]="p_{T}(#gamma)";
  xaxisLabel[5]="#eta(#gamma)";
  xaxisLabel[6]="#phi(#gamma)";
  xaxisLabel[7]="R9(#gamma)";
  xaxisLabel[8]="#sigma_{i#etai#eta}";
  xaxisLabel[9]="SCRawEnergy(#gamma)";
 
  TString binSize[NVARIABLES];

  for (int iCut=0;iCut<NCUTS;++iCut) {
    for (int iVar=0;iVar<NVARIABLES;++iVar) {
      
      sprintf(icut[iCut],"icut%d",iCut);
      histos[iCut][iVar]=new TH1F(variables[iVar]+"_"+TString(icut[iCut]),variables[iVar]+"_"+TString(icut[iCut]),nbins[iVar],range[iVar][0],range[iVar][1]);
      histos[iCut][iVar]->SetTitle("");
      histos[iCut][iVar]->SetXTitle(xaxisLabel[iVar]+" ("+units[iVar]+")");
      histos[iCut][iVar]->SetYTitle("");
      char binsiz[10];
      sprintf(binsiz,"%2.0f",(range[iVar][1]-range[iVar][0])/nbins[iVar]);
      binSize[iVar]=TString(binsiz);
      
    }
  }

  //cuts
  TString cut[NCUTS];
  cut[0]="(invMass>0)";
  //cut[0]="(invMass>60 && invMass<120)";

  TFile *_file;
  TTree *T1;
  TString input_file = "../TaP_output.root";
  _file=TFile::Open(input_file);
  T1 = (TTree*)_file->Get("tnpAna/TaPtree");

  for (int iVar=0;iVar<NVARIABLES;++iVar) {
    for (int iCut=0;iCut<NCUTS;++iCut) {
     	fOut->cd();
	TString histoName=variables[iVar]+"_"+TString(icut[iCut]);
	std::cout << "Producing " << histoName << std::endl;
	if (T1==0) {
	  std::cout << " Tree not found" << std::endl;
	}
	T1->Project(histoName,variables[iVar],cut[iCut]);
	std::cout << "Done " << histoName << std::endl;
 

	// --- Draw and print
	//--------------------------------------------------------
	TCanvas* c1 = new TCanvas(Form("test_%d_%d_lin", iVar, iCut),
				  Form("test_%d_%d_lin", iVar, iCut));
      
	c1->SetLogy(0);
	myPlot->SimpleDraw(histos[iCut][iVar]);
	c1->GetFrame()->DrawClone();
	c1->SaveAs(plotsDir+variables[iVar]+"_MC_DY_50ns_"+TString(icut[iCut])+".png");

	TCanvas* c2 = new TCanvas(Form("test_%d_%d_log", iVar, iCut),
				  Form("test_%d_%d_log", iVar, iCut));
      
	c2->SetLogy(1);
	myPlot->SimpleDraw(histos[iCut][iVar]);
	c2->GetFrame()->DrawClone();
	c2->SaveAs(plotsDir+variables[iVar]+"_MC_DY_50ns_"+TString(icut[iCut])+"_log.png");
	
    }//iCut
  }//iVar
  
  fOut->Write();
  fOut->Close();

}
