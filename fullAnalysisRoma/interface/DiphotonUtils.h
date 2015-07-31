#ifndef DiphotonUtils_h
#define DiphotonUtils_h

#include "THStack.h"
#include "TGaxis.h"
#include "TH1F.h"
#include "TLatex.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TFrame.h"
#include "TStyle.h"
#include <iostream>


class DiphotonPlot {

 private:

  std::vector<TH1F*> _hist;
  TH1F* _data;
  TH1F* _TaPhisto;
  
  //MWL
  float    _lumi;
  TString  _xLabel;
  TString  _units;
  TLatex * _extraLabel;
  bool     _breakdown;
  int      _mass;
  bool     _nostack;
  
 public:

  //global variables

  enum samp { iRS, iQCD, iGJets, iGG, nSamples };

  const float xPos[nSamples+1] = {0.70,0.70,0.70,0.70};
  const float yOff[nSamples+1] = {0,1,2,3};

  const Float_t _tsize   = 0.03;
  const Float_t _xoffset = 0.20;
  const Float_t _yoffset = 0.05;


  DiphotonPlot();
  virtual ~DiphotonPlot();

  //-------------------

  Float_t GetMaximumIncludingErrors(TH1F* h);
  void AxisFonts(TAxis*  axis, TString coordinate, TString title);
  void THStackAxisFonts(THStack* h, TString  coordinate, TString  title);
  void DrawLegend(Float_t x1, Float_t y1, TH1F*   hist, TString label, TString option);

  //------------------

  void setMCHist   (const samp &s, TH1F *h);   
  void setDataHist (TH1F *h); 
  void setRSHist   (TH1F *h);                 
  void setGGHist   (TH1F *h);                
  void setGJetsHist(TH1F *h);                
  void setQCDHist  (TH1F *h);                 
 
  void setNoStack();

  TH1F* getDataHist();

  void setMass(const int &m);

  void DrawAndRebinTo(const int &rebinTo);

  //void Draw(const int &rebin=1);
  void Draw(const int rebin);
  void SimpleDraw(TH1F *h);

  void setLumi(const float &l);
  void setLabel(const TString &s);
  void setUnits(const TString &s);
  // void setBreakdown(const bool &b = true);
  void setBreakdown(const bool &b);
  void addLabel(const std::string &s);

  void setStyle();

};

#endif
