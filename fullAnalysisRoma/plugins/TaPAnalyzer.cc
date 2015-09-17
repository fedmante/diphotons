#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"  

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Common/interface/TriggerResults.h"     
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"     
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"   

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "flashgg/DataFormats/interface/Photon.h"
#include "flashgg/DataFormats/interface/Electron.h"

#include "TMath.h"
#include "TTree.h"
#include "TLorentzVector.h"

#define MAX_PU_REWEIGHT 60

using namespace std;
using namespace edm;
using namespace flashgg;
using namespace reco;

using pat::PackedGenParticle;   

class TaPAnalyzer : public edm::EDAnalyzer {
  
public:
  
    explicit TaPAnalyzer(const edm::ParameterSet&);
    ~TaPAnalyzer();
  
private:
  
    virtual void beginJob() override;
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;
  
    // weights
    void SetPuWeights(std::string puWeightFile);
    float GetPUWeight(float pun);

    // photons
    bool isGammaPresel( float sceta, float pt, float r9, float chiso);
    bool isGammaSelected( float rho, float pt, float sceta, float r9, float chiso, float nhiso, float phoiso, float hoe, float sieie, bool passElectronVeto);
    int  effectiveAreaGammaRegion(float sceta);

    // electrons
    float effectiveAreaEle03(float sceta);
    Ptr<reco::Vertex> chooseElectronVertex( Ptr<flashgg::Electron> &elec, const std::vector<edm::Ptr<reco::Vertex> > &vertices );
    bool isMediumEle(float scEta, float hoe, float dphi, float deta, float sIeIe, float ep, float d0, float dz, float reliso, int missHits, bool passConvVeto) ;
    bool isTightEle(float scEta, float hoe, float dphi, float deta, float sIeIe, float ep, float d0, float dz, float reliso, int missHits, bool passConvVeto) ;
  
    void bookOutputTree();
  
    // collections
    EDGetTokenT<View<reco::Vertex> > vertexToken_;
    EDGetTokenT<View<Electron> > electronToken_;
    edm::EDGetTokenT<edm::View<flashgg::Photon> > photonToken_;
    EDGetTokenT<edm::View<PileupSummaryInfo> > PileUpToken_; 
    edm::InputTag genInfo_; 
    edm::EDGetTokenT<edm::TriggerResults> triggerBits_;
    edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjects_;  

    // sample-dependent parameters needed for the analysis
    int dopureweight_;
    int sampleIndex_;
    string puWFileName_;
    float xsec_;    // pb
    float kfac_;
    float sumDataset_;

    // to compute weights for pileup
    std::vector<Double_t> puweights_;

    // to keep track of the number of events
    TH1F *h_entries;

    // to keep track of the sum of weights
    TH1F *h_sumW;
    bool isFilled;

    //---output tree branches variables
    bool ptRatioFlip_;
    edm::Service<TFileService> fs_;
    TTree* outTree_;

    int   run;
    int   event;
    int   lumi;
    int   nvtx;
    float rho;
    int   sampleID;
    float totXsec;
    float pu_weight;
    float pu_n;
    float sumDataset;
    float perEveW;

    int accEleSize;
    vector <float> electron_pt={};
    vector <float> electron_eta={};
    vector <float> electron_phi={};
    vector <bool>  isTagTightEle={};
    vector <bool>  isTagMediumEle={};
    vector <bool>  electron_matchHLT={};
   
    int  accGammaSize;                
    vector <float> gamma_pt={};
    vector <float> gamma_eta ={};
    vector <float> gamma_phi={};
    vector <float> gamma_r9={};
    vector <float> gamma_sieie={};
    vector <float> gamma_hoe ={};
    vector <float> gamma_scRawEne={};
    vector <float> gamma_chiso={};
    vector <float> gamma_phoiso={};
    vector <float> gamma_neuiso={};
    vector <float> gamma_eleveto={};
    vector <int>   gamma_presel={};
    vector <int>   gamma_fullsel={};
    vector <bool>  gamma_matchHLT={};  

    vector <float> ptRatio={};
    vector <float> invMass={};
    vector <int> eleIndex={};
    vector <int> gammaIndex={};
};  

TaPAnalyzer::TaPAnalyzer(const edm::ParameterSet& iConfig):

    // collections
    vertexToken_(consumes<View<reco::Vertex> >(iConfig.getUntrackedParameter<InputTag> ("VertexTag", InputTag("offlineSlimmedPrimaryVertices")))),
    electronToken_( consumes<View<flashgg::Electron> >( iConfig.getParameter<InputTag> ("ElectronTag"))),
    photonToken_(consumes<View<flashgg::Photon> >(iConfig.getUntrackedParameter<InputTag> ("PhotonTag", InputTag("flashggPhotons")))),
    PileUpToken_(consumes<View<PileupSummaryInfo> >(iConfig.getUntrackedParameter<InputTag> ("PileUpTag", InputTag("addPileupInfo")))),
    triggerBits_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("bits"))),
    triggerObjects_(consumes<pat::TriggerObjectStandAloneCollection>(iConfig.getParameter<edm::InputTag>("objects")))
{ 
    dopureweight_ = iConfig.getUntrackedParameter<int>("dopureweight", 0);
    sampleIndex_  = iConfig.getUntrackedParameter<int>("sampleIndex",0);
    puWFileName_  = iConfig.getParameter<std::string>("puWFileName");   
    xsec_         = iConfig.getUntrackedParameter<double>("xsec",1.); 
    kfac_         = iConfig.getUntrackedParameter<double>("kfac",1.); 
    sumDataset_   = iConfig.getUntrackedParameter<double>("sumDataset",-999.);
    genInfo_      = iConfig.getParameter<edm::InputTag>("generatorInfo");
    ptRatioFlip_  = false;
};

TaPAnalyzer::~TaPAnalyzer() { };

void TaPAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    // To keep track of the total number of events
    h_entries->Fill(5);

    // Sample index
    int sampleID = sampleIndex_;

    // --------------------------------------------------
    // access edm objects                                                                                    
    Handle<View<reco::Vertex> > primaryVertices;
    iEvent.getByToken(vertexToken_,primaryVertices);

    Handle<View<flashgg::Electron> > theElectrons;
    iEvent.getByToken( electronToken_, theElectrons );

    Handle<View< PileupSummaryInfo> > PileupInfos;
    iEvent.getByToken(PileUpToken_,PileupInfos);
  
    Handle<double> objs_rho;                                 
    iEvent.getByLabel("fixedGridRhoAll",objs_rho);
    // Handle<double> objs_rhoEle;                               // chiara: bisogna passare a questo, ma non c'e' nelle ntuple
    // iEvent.getByLabel("fixedGridRhoFastjetAll",objs_rhoEle);

    Handle<View<flashgg::Photon> > objs_pho;
    iEvent.getByToken(photonToken_,objs_pho);
  
    edm::Handle<edm::TriggerResults> triggerBits;
    iEvent.getByToken(triggerBits_, triggerBits);

    edm::Handle<pat::TriggerObjectStandAloneCollection> triggerObjects;
    iEvent.getByToken(triggerObjects_, triggerObjects);
  
    edm::Handle<GenEventInfoProduct> genInfo;      
    if (sampleID>0) iEvent.getByLabel(genInfo_,genInfo);  

    // --------------------------------------------------
    // Event info
    run   = iEvent.eventAuxiliary().run();
    lumi  = iEvent.eventAuxiliary().luminosityBlock();
    event = iEvent.eventAuxiliary().event();

    // # Vertices
    nvtx = primaryVertices->size(); 

    // Energy density
    rho    = *(objs_rho.product());
    // float rhoEle = *(objs_rhoEle.product());    // EA correction for electrons - chiara: non nelle ntuple

    // PU weight (for MC only and if requested)
    pu_weight = 1.;
    pu_n      = -1.;
    if (sampleID>0) {     // MC
        pu_n = 0.;
        for( unsigned int PVI = 0; PVI < PileupInfos->size(); ++PVI )
        {
            Int_t pu_bunchcrossing = PileupInfos->ptrAt( PVI )->getBunchCrossing();
            if( pu_bunchcrossing == 0 ) {
                pu_n = PileupInfos->ptrAt( PVI )->getPU_NumInteractions();
            }
        }
        if (dopureweight_) 
            pu_weight = GetPUWeight(pu_n);         
    }
  
    // x-sec * kFact for MC only 
    totXsec = 1.;
    if (sampleID>0) totXsec = xsec_ * kfac_;

    // other weights for the dataset
    sumDataset = 1.;  
    perEveW    = 1.;
    if (sampleID>0) { 
        sumDataset = sumDataset_;
        const auto & eveWeights = genInfo->weights();
        if(!eveWeights.empty()) perEveW = eveWeights[0];
    }

    // To keep track of the sum of weights
    if (!isFilled) {
        h_sumW->Fill(5,sumDataset);
        isFilled = true;
    }

    // ----------------------------------------------------
    // save events only if:
    // 1) pass TnP HLT paths
    // 1) good vertex
    // 2) at least one tag
    // 3) at least one probe
    // ----------------------------------------------------


    // ----------------------------------------------------
    // 1) analysis cuts: trigger 

    // selected HLT object
    vector<float> hltTagPt, hltTagEta, hltTagPhi;
    vector<float> hltProbePt, hltProbeEta, hltProbePhi;

    const edm::TriggerNames &names = iEvent.triggerNames(*triggerBits);

    // HLT paths for TnP
    string theTnPPath1 = "HLT_Ele25WP60_Ele8_Mass55_v";
    string theTnPPath2 = "HLT_Ele25WP60_SC4_Mass55_v";
    string theTnPPath3 = "HLT_Ele27_eta2p1_WP75_Gsf_v";
    // this is for MC. In data: HLT_Ele27_eta2p1_WPLoose_Gsf_v*

    // HLT filters for TnP
    string theTagFilter1   = "hltEle25WP60Ele8TrackIsoFilter";
    string theTagFilter2   = "hltEle25WP60SC4TrackIsoFilter";
    string theProbeFilter1 = "hltEle25WP60Ele8PixelMatchUnseededFilter";
    string theProbeFilter2 = "hltEle25WP60SC4EtUnseededFilter";

    bool fired[3]={false};

    // check if the event fired one of the TnP paths
    for (unsigned int i = 0, n = triggerBits->size(); i < n; ++i)
    {
        string thisPath = names.triggerName(i);
        if (thisPath.find(theTnPPath1)==string::npos &&
            thisPath.find(theTnPPath2)==string::npos &&
            thisPath.find(theTnPPath3)==string::npos)
            continue;
        if (!triggerBits->accept(i))
            continue;
        // cout << "firing TnP path " << thisPath << endl;
        if (thisPath.find(theTnPPath1)!=string::npos)
            fired[0] = true;
        if (thisPath.find(theTnPPath2)!=string::npos)
            fired[1] = true;
        if (thisPath.find(theTnPPath3)!=string::npos)
            fired[2] = true;
    }

    bool atLeastOneTag = false;
    bool atLeastOneProbe = false;
    if (fired[0] || fired[1])
    {
        // if (fired[0] || fired[1] || fired[2]) {  // ancora da verificare il terzo

        for (pat::TriggerObjectStandAlone obj : *triggerObjects) {
            obj.unpackPathNames(names);
      
            vector<string> pathNamesAll  = obj.pathNames(false);
            for (unsigned h = 0, n = pathNamesAll.size(); h < n; ++h) {
                string thisPath = pathNamesAll[h];
                // if ( (thisPath.find(theTnPPath1)!=string::npos && fired[0]) || (thisPath.ind(theTnPPath2)!=string::npos && fired[1]) || (thisPath.find(theTnPPath3)!=string::npos && fired[2]) ){    
                if ( (thisPath.find(theTnPPath1)!=string::npos && fired[0]) ||
                     (thisPath.find(theTnPPath2)!=string::npos && fired[1]) )
                {    
                    // check if one of the accepted filters was in
                    bool isFilterTag   = false;
                    bool isFilterProbe = false;
                    // std::cout << "ALL Filters for " << pathNamesAll[h] << endl;
                    for (unsigned h1 = 0; h1 < obj.filterLabels().size(); ++h1) {
                        string thisFilter = obj.filterLabels()[h1];
                        if (thisFilter.find(theTagFilter1)!=string::npos || thisFilter.find(theTagFilter2)!=string::npos) {
                            // std::cout << h1 << " " << thisFilter << " is ok for tag for " << thisPath << endl;
                            isFilterTag = true;
                        }
                        if (thisFilter.find(theProbeFilter1)!=string::npos || thisFilter.find(theProbeFilter2)!=string::npos) {
                            //std::cout << h1 << " " << thisFilter << " is ok for tag for " << thisPath << endl;
                            isFilterProbe = true;
                        }
                    }

                    // save the HLT objects which are ok fot T or P
                    if (isFilterTag) {
                        hltTagPt.push_back(obj.pt());
                        hltTagEta.push_back(obj.eta());
                        hltTagPhi.push_back(obj.phi());
                    }
                    if (isFilterProbe) {
                        hltProbePt.push_back(obj.pt());
                        hltProbeEta.push_back(obj.eta());
                        hltProbePhi.push_back(obj.phi());
                    }
                }
            }
        }
    }

    if (hltTagPt.size()!=hltTagEta.size() || hltTagPt.size()!=hltTagPhi.size()) cout << "problem!" << endl;
    if (hltProbePt.size()!=hltProbeEta.size() || hltProbePt.size()!=hltProbePhi.size()) cout << "problem!" << endl;
    if (hltTagPt.size()>0 && hltProbePt.size()>0)
    {
        // ----------------------------------------------------
        // 2) at least one good vertex found
        bool foundOne = false;
        for( unsigned int ivtx = 0 ; ivtx < primaryVertices->size() ; ivtx++ ) {
            if (foundOne) continue;
      
            bool goodVtx = true;
            float vtxX = primaryVertices->ptrAt(ivtx)->position().x();
            float vtxY = primaryVertices->ptrAt(ivtx)->position().y();
            float d0vtx = sqrt( vtxX*vtxX + vtxY*vtxY );
            if ( (primaryVertices->ptrAt(ivtx))->ndof()<=4 )  goodVtx = false;
            if ( fabs(d0vtx)>2 ) goodVtx = false;
            if ( fabs((primaryVertices->ptrAt(ivtx))->position().z())>=24 ) goodVtx = false;
            bool isVtxFake = ((primaryVertices->ptrAt(ivtx))->ndof()==0) && ((primaryVertices->ptrAt(ivtx))->chi2()==0);   // chiara: also && tracks.empty, but can not be used here
            if (isVtxFake) goodVtx = false;
      
            if (goodVtx) foundOne = true;
        } 
        if (foundOne) {


            // ----------------------------------------------------  
            // 3) at least one tag candidate
            atLeastOneTag = false;
      
            // Loop over electron candidates
            const std::vector<edm::Ptr<reco::Vertex> > &vertexPointers = primaryVertices->ptrs();
            const std::vector<edm::Ptr<flashgg::Electron> > &ElectronPointers = theElectrons->ptrs();
            std::vector<int> acceptEle;
            // electrons in the acceptance
            for( unsigned int ElectronIndex = 0; ElectronIndex < ElectronPointers.size(); ElectronIndex++ )
            {	
                Ptr<flashgg::Electron> Electron = ElectronPointers[ElectronIndex];
	
                // acceptance
                float scEta = fabs( Electron->superCluster()->eta() );
                float elePt = Electron->pt();
                if( (fabs(scEta)>1.442 && fabs(scEta)<1.566) || fabs(scEta)>2.5 )
                    continue;
                if( elePt<5 )
                    continue;
	
                acceptEle.push_back(ElectronIndex);
            }
      
            // full cut based selection
            for(unsigned int iEle=0; iEle<acceptEle.size(); iEle++)
            {
                int theOrigIndex = acceptEle.at(iEle);
                Ptr<flashgg::Electron> Electron = ElectronPointers[theOrigIndex];
	      
                // kine
                float scEta  = fabs( Electron->superCluster()->eta() );
                float elePt  = Electron->pt();
                float eleEta = Electron->eta();
                float elePhi = Electron->phi();
	
                // match with selected HLT objects
                bool matchHLT = false;
                TLorentzVector thisRecoEle(0,0,0,0);
                thisRecoEle.SetPtEtaPhiM(elePt,eleEta,elePhi,0);
                for (int hltTagC=0; hltTagC<(int)hltTagPt.size(); hltTagC++)
                {
                    TLorentzVector thisHLTob(0,0,0,0);  
                    float thisHLTpt  = hltTagPt[hltTagC];
                    float thisHLTeta = hltTagEta[hltTagC];
                    float thisHLTphi = hltTagPhi[hltTagC];
                    thisHLTob.SetPtEtaPhiM(thisHLTpt,thisHLTeta,thisHLTphi,0);
                    if(thisRecoEle.DeltaR(thisHLTob)<0.3)
                        matchHLT = true;
                }

                // ID
                float HoE = Electron->hcalOverEcal();
                float DeltaPhiIn = Electron->deltaPhiSuperClusterTrackAtVtx();
                float DeltaEtaIn = Electron->deltaEtaSuperClusterTrackAtVtx();
                float Full5x5Sieie = Electron->full5x5_sigmaIetaIeta(); 
                float ecalEne = Electron->ecalEnergy();
                float OneOverEoP;
                if (ecalEne==0) {
                    cout << "electron energy is zero!! " << endl;
                    OneOverEoP = 1000000.;
                } else {
                    OneOverEoP = 1.0/ecalEne - (Electron->eSuperClusterOverP())/ecalEne;
                }
	
                // isolation with rho correction
                // chiara: controlla che le variabili siano queste e che siano x dR=0.3
                reco::GsfElectron::PflowIsolationVariables pfIso = Electron->pfIsolationVariables();
                // float corrHadPlusPho = pfIso.sumNeutralHadronEt + pfIso.sumPhotonEt - rhoEle*effectiveAreaEle03(scEta);      // chiara
                float corrHadPlusPho = pfIso.sumNeutralHadronEt + pfIso.sumPhotonEt - rho*effectiveAreaEle03(scEta);
                if (corrHadPlusPho<=0) corrHadPlusPho = 0.;
                float absIsoWeffArea = pfIso.sumChargedHadronPt + corrHadPlusPho;
                float relIso = absIsoWeffArea/elePt;
	
                // Impact parameters
                Ptr<reco::Vertex> Electron_vtx = chooseElectronVertex( Electron, vertexPointers );
                float d0 = Electron->gsfTrack()->dxy(Electron_vtx->position() );
                float dz = Electron->gsfTrack()->dz( Electron_vtx->position() );
	
                // Conversions
                const reco::HitPattern &hitPattern = Electron->gsfTrack()->hitPattern();
                int mHits= hitPattern.numberOfHits(HitPattern::MISSING_INNER_HITS);
                bool passConversionVeto = !(Electron->hasMatchedConversion());
	
                // All together
                bool mediumEle = isMediumEle(scEta, HoE, DeltaPhiIn, DeltaEtaIn, Full5x5Sieie, OneOverEoP, d0, dz, relIso, mHits, passConversionVeto) ;
                bool tightEle = isTightEle(scEta, HoE, DeltaPhiIn, DeltaEtaIn, Full5x5Sieie, OneOverEoP, d0, dz, relIso, mHits, passConversionVeto) ;
	
                if (mediumEle || tightEle) atLeastOneTag = true;
	
                // Variables for the tree - for each electron in the acceptance - todo
                // accEleSize
                // elePt
                // scEta
                // scPhi
                // mediumEle
                // tightEle
                // matchHLT
                electron_pt.push_back(elePt);
                electron_eta.push_back(Electron->superCluster()->eta());
                electron_phi.push_back(Electron->superCluster()->phi());
                isTagTightEle.push_back(tightEle);
                isTagMediumEle.push_back(mediumEle);
                electron_matchHLT.push_back(matchHLT);                
            }  // tag
            accEleSize = electron_pt.size();

            // ----------------------------------------------------
            // 4) at least one probe found
            atLeastOneProbe = false;
      
            std::vector<int> acceptGamma;
            for(int phloop = 0; phloop < (int)objs_pho->size(); phloop++ ) {  
	
                Ptr<flashgg::Photon> g1 = objs_pho->ptrAt( phloop );
	
                // acceptance
                float gammaPt    = g1->et();
                float gammaScEta = (g1->superCluster())->eta();
                if (gammaPt<5)
                    continue;
                if (fabs(gammaScEta)>2.5)
                    continue;
                if (fabs(gammaScEta)>1.4442 && fabs(gammaScEta)<1.566)
                    continue;
	
                acceptGamma.push_back(phloop);
            }

            // Photon candidates in the acceptance
            for(unsigned int iGamma=0; iGamma<acceptGamma.size(); iGamma++)
            {	
                int theOrigIndex = acceptGamma.at(iGamma);  
                Ptr<flashgg::Photon> g1 = objs_pho->ptrAt( theOrigIndex );
	
                // kinematics
                float scEta  = (g1->superCluster())->eta();         
                float pt     = g1->et();
	
                // kine
                float eta = g1->eta();
                float phi = g1->phi();
	
                // match with selected HLT objects
                bool matchHLT = false;
                TLorentzVector thisRecoGamma(0,0,0,0);
                thisRecoGamma.SetPtEtaPhiM(pt,eta,phi,0);
                for (int hltProbeC=0; hltProbeC<(int)hltProbePt.size(); hltProbeC++)
                {
                    TLorentzVector thisHLTob(0,0,0,0);  
                    float thisHLTpt  = hltProbePt[hltProbeC];
                    float thisHLTeta = hltProbeEta[hltProbeC];
                    float thisHLTphi = hltProbePhi[hltProbeC];
                    thisHLTob.SetPtEtaPhiM(thisHLTpt,thisHLTeta,thisHLTphi,0);
                    if(thisRecoGamma.DeltaR(thisHLTob)<0.3)
                        matchHLT = true;
                }

                // preselection and full sel
                float R9noZS    = g1->full5x5_r9();    
                float HoE       = g1->hadTowOverEm();
                float sieienoZS = g1->full5x5_sigmaIetaIeta();   
	
                float chIso  = g1->egChargedHadronIso();
                float neuIso = g1->egNeutralHadronIso();
                float phoIso = g1->egPhotonIso();
	
                bool  eleVeto = g1->passElectronVeto();
	
                bool passPresel    = isGammaPresel( scEta, pt, R9noZS, chIso); 
                bool passFullSelel = isGammaSelected( rho, pt, scEta, R9noZS, chIso, neuIso, phoIso, HoE, sieienoZS, eleVeto); 
	
                if(passPresel)
                    atLeastOneProbe = true;
	
                gamma_pt.push_back(pt);
                gamma_eta.push_back(scEta);
                gamma_phi.push_back(g1->superCluster()->phi());
                gamma_r9.push_back(g1->full5x5_r9());
                gamma_sieie.push_back(g1->full5x5_sigmaIetaIeta());
                gamma_hoe.push_back(g1->hadTowOverEm());
                gamma_scRawEne.push_back(g1->superCluster()->rawEnergy());
                gamma_chiso.push_back(g1->egChargedHadronIso());
                gamma_phoiso.push_back(g1->egPhotonIso());
                gamma_neuiso.push_back(g1->egNeutralHadronIso());
                gamma_eleveto.push_back(g1->passElectronVeto());
                gamma_presel.push_back(passPresel);
                gamma_fullsel.push_back(passFullSelel);
                gamma_matchHLT.push_back(matchHLT);
                // Variables for the tree - for each photon in the acceptance - todo
                // accGammaSize
                // pt
                // scEta
                // scPhi
                // R9noZS, HoE, sieienoZS
                // chIso, neuIso, phoIso
                // eleVeto
                // passPresel
                // passFullSelel
                // matchHLT
            } // probe
        } // vertex
    } // HLT    
    accGammaSize = gamma_pt.size();

    //---invariant mass and pt ratio
    for(int iGam=0; iGam<accGammaSize; ++iGam)
    {
        for(int iEle=0; iEle<accEleSize; ++iEle)
        {
            TLorentzVector thisGamma(0,0,0,0);            
            TLorentzVector thisEle(0,0,0,0);
            thisGamma.SetPtEtaPhiM(gamma_pt[iGam],gamma_eta[iGam],gamma_phi[iGam],0);
            thisEle.SetPtEtaPhiM(electron_pt[iEle],electron_eta[iEle],electron_phi[iEle],0);
            invMass.push_back((thisGamma+thisEle).M());
            eleIndex.push_back(iEle); 
            gammaIndex.push_back(iGam);
            if(thisGamma.Pt() > thisEle.Pt())
            {
                if(ptRatioFlip_)
                    ptRatio.push_back(thisEle.Pt()/thisGamma.Pt());
                else
                    ptRatio.push_back(thisGamma.Pt()/thisEle.Pt());
                ptRatioFlip_ = !ptRatioFlip_;
            }
            else
            {
                if(ptRatioFlip_)
                    ptRatio.push_back(thisGamma.Pt()/thisEle.Pt());
                else
                    ptRatio.push_back(thisEle.Pt()/thisGamma.Pt());
                ptRatioFlip_ = !ptRatioFlip_;
            }
        }
    }

    //---fill output tree and reset
    if(atLeastOneProbe && atLeastOneTag)
        outTree_->Fill();

    //---tag
    electron_pt.clear();
    electron_eta.clear();
    electron_phi.clear();
    isTagTightEle.clear();
    isTagMediumEle.clear();
    electron_matchHLT.clear();
    //---probe
    gamma_pt.clear();
    gamma_eta.clear();
    gamma_phi.clear();
    gamma_r9.clear();
    gamma_sieie.clear();
    gamma_hoe.clear();
    gamma_scRawEne.clear();
    gamma_chiso.clear();
    gamma_phoiso.clear();
    gamma_neuiso.clear();
    gamma_eleveto.clear();
    gamma_presel.clear();
    gamma_fullsel.clear();
    gamma_matchHLT.clear();
    //---invariant mass and ptRatio
    ptRatio.clear();
    invMass.clear();
    eleIndex.clear();
    gammaIndex.clear();
}

void TaPAnalyzer::beginJob()
{
    // loading weights for pileup if needed
    if (dopureweight_) 
        SetPuWeights(puWFileName_);
  
    // to keep track of the original number of events
    h_entries = fs_->make<TH1F>("h_entries", "h_entries", 10,  0., 10.);
    h_entries->Sumw2();

    // to keep track of the sum of weights
    h_sumW = fs_->make<TH1F>("h_sumW", "h_sumW", 10,  0., 10.);
    h_sumW->Sumw2();
    isFilled = false;

    bookOutputTree();
}

void TaPAnalyzer::bookOutputTree() 
{
    outTree_ = fs_->make<TTree>("TaPtree", "TaPtree");
    
    cout << "Booking branches" << endl;
    
    outTree_->Branch("run", &run, "run/I");
    outTree_->Branch("event", &event, "event/I");
    outTree_->Branch("lumi", &lumi, "lumi/I");
    outTree_->Branch("nvtx", &nvtx, "nvtx/I");
    outTree_->Branch("rho", &rho, "rho/F");
    outTree_->Branch("sampleID", &sampleID, "sampleID/I");
    outTree_->Branch("totXsec", &totXsec, "totXsec/F");
    outTree_->Branch("pu_weight", &pu_weight, "pu_weight/F");
    outTree_->Branch("pu_n", &pu_n, "pu_n/F");
    outTree_->Branch("sumDataset", &sumDataset, "sumDataset/F");
    outTree_->Branch("perEveW", &perEveW, "perEveW/F");

    outTree_->Branch("accEleSize", &accEleSize, "accEleSize/I");
    outTree_->Branch("electron_pt", "std::vector<float>", &electron_pt);
    outTree_->Branch("electron_eta", "std::vector<float>", &electron_eta);
    outTree_->Branch("electron_phi", "std::vector<float>", &electron_phi);
    outTree_->Branch("isTagTightEle", "std::vector<bool>", &isTagTightEle );
    outTree_->Branch("isTagMediumEle", "std::vector<bool>", &isTagMediumEle );
    outTree_->Branch("electron_matchHLT", "std::vector<bool>", &electron_matchHLT );
 
    outTree_->Branch("accGammaSize",  &accGammaSize,  "accGammaSize/I");   
    outTree_->Branch("gamma_pt", "std::vector<float>", &gamma_pt);
    outTree_->Branch("gamma_eta", "std::vector<float>", &gamma_eta);
    outTree_->Branch("gamma_phi", "std::vector<float>", &gamma_phi);
    outTree_->Branch("gamma_r9", "std::vector<float>", &gamma_r9);
    outTree_->Branch("gamma_sieie", "std::vector<float>", &gamma_sieie);
    outTree_->Branch("gamma_hoe", "std::vector<float>", &gamma_hoe);
    outTree_->Branch("gamma_scRawEne", "std::vector<float>", &gamma_scRawEne);
    outTree_->Branch("gamma_chiso", "std::vector<float>", &gamma_chiso);
    outTree_->Branch("gamma_phoiso", "std::vector<float>", &gamma_phoiso);
    outTree_->Branch("gamma_neuiso", "std::vector<float>", &gamma_neuiso);
    outTree_->Branch("gamma_eleveto", "std::vector<float>", &gamma_eleveto);
    outTree_->Branch("gamma_presel", "std::vector<int>", &gamma_presel);
    outTree_->Branch("gamma_fullsel", "std::vector<int>", &gamma_fullsel);
    outTree_->Branch("gamma_matchHLT", "std::vector<bool>", &gamma_matchHLT);

    outTree_->Branch("ptRatio","std::vector<float>", &ptRatio);
    outTree_->Branch("invMass","std::vector<float>", &invMass);
    outTree_->Branch("eleIndex","std::vector<int>", &eleIndex);
    outTree_->Branch("gammaIndex","std::vector<int>", &gammaIndex);
}

void TaPAnalyzer::endJob() { }

// Phys14_25ns_v1
float TaPAnalyzer::effectiveAreaEle03(float theEta) {
  
    float theEA = -999;			   
    if (fabs(theEta)<0.800) theEA = 0.1013; 
    else if (fabs(theEta)<1.300) theEA = 0.0988;
    else if (fabs(theEta)<2.000) theEA = 0.0572;
    else if (fabs(theEta)<2.200) theEA = 0.0842; 

    return theEA;
}

// Electron vertex choice
Ptr<reco::Vertex> TaPAnalyzer::chooseElectronVertex( Ptr<flashgg::Electron> &elec, const std::vector<edm::Ptr<reco::Vertex> > &vertices ) {
  
    double vtx_dz = 1000000;
    unsigned int min_dz_vtx = -1;
    for( unsigned int vtxi = 0; vtxi < vertices.size(); vtxi++ ) {
        Ptr<reco::Vertex> vtx = vertices[vtxi];
        if( vtx_dz > fabs(elec->gsfTrack()->dz(vtx->position())) ) {
            vtx_dz = fabs( elec->gsfTrack()->dz( vtx->position() ) );
            min_dz_vtx = vtxi;
        }
    }
    return vertices[min_dz_vtx];
}

// Egamma Cut based ID

bool TaPAnalyzer::isMediumEle(float scEta, float hoe, float dphi, float deta, float sIeIe, float ep, float d0, float dz, float reliso, int missHits, bool passConvVeto) { 

    bool okDeta, okDphi, okSieIe, okHoE, okEp, okD0, okDz, okIso, okMH, okConv;
 
    if (fabs(scEta)<1.5) {

        okDeta  = fabs(deta) < 0.008925;
        okDphi  = fabs(dphi) < 0.035973;
        okSieIe = sIeIe < 0.009996;
        okHoE   = hoe < 0.050537;
        okD0    = fabs(d0) < 0.012235;
        okDz    = fabs(dz) < 0.042020;
        okEp    = fabs(ep) < 0.091942;
        okIso   = reliso < 0.107587;
        okMH    = missHits<=1;
        okConv  = passConvVeto;

    } else {

        okDeta  = fabs(deta) < 0.007429;
        okDphi  = fabs(dphi) < 0.067879;
        okSieIe = sIeIe < 0.030135 ;
        okHoE   = hoe < 0.086782;
        okD0    = fabs(d0) < 0.036719;
        okDz    = fabs(dz) < 0.138142 ;
        okEp    = fabs(ep) < 0.100683 ;
        okIso   = reliso < 0.113254;
        okMH    = missHits<=1;
        okConv  = passConvVeto;
    }

    bool okFullSel = okDeta && okDphi && okSieIe && okHoE && okEp && okD0 && okDz && okIso && okMH && okConv;
    return okFullSel;
}


bool TaPAnalyzer::isTightEle(float scEta, float hoe, float dphi, float deta, float sIeIe, float ep, float d0, float dz, float reliso, int missHits, bool passConvVeto)
{ 
    bool okDeta, okDphi, okSieIe, okHoE, okEp, okD0, okDz, okIso, okMH, okConv;
                                                                                                                                      
    if (fabs(scEta)<1.5) {

        okDeta  = fabs(deta) < 0.006046;
        okDphi  = fabs(dphi) < 0.028092;
        okSieIe = sIeIe < 0.009947;
        okHoE   = hoe < 0.045772;
        okD0    = fabs(d0) < 0.008790;
        okDz    = fabs(dz) < 0.021226;
        okEp    = fabs(ep) < 0.020118;
        okIso   = reliso < 0.069527;
        okMH    = missHits<=1;
        okConv  = passConvVeto;

    } else {

        okDeta  = fabs(deta) < 0.007057;
        okDphi  = fabs(dphi) < 0.030159;
        okSieIe = sIeIe < 0.028237;
        okHoE   = hoe < 0.067778;
        okD0    = fabs(d0) < 0.027984;
        okDz    = fabs(dz) < 0.133431 ;
        okEp    = fabs(ep) < 0.098919 ;
        okIso   = reliso < 0.078265;
        okMH    = missHits<=1;
        okConv  = passConvVeto;
    }

    bool okFullSel = okDeta && okDphi && okSieIe && okHoE && okEp && okD0 && okDz && okIso && okMH && okConv;
    return okFullSel;
}



void TaPAnalyzer::SetPuWeights(std::string puWeightFile)
{
    if (puWeightFile == "") {
        std::cout << "you need a weights file to use this function" << std::endl;
        return;
    }
    std::cout << "PU REWEIGHTING:: Using file " << puWeightFile << std::endl;

    TFile *f_pu  = new TFile(puWeightFile.c_str(),"READ");
    f_pu->cd();

    TH1D *puweights = 0;
    TH1D *gen_pu = 0;
    gen_pu    = (TH1D*) f_pu->Get("generated_pu");
    puweights = (TH1D*) f_pu->Get("weights");

    if (!puweights || !gen_pu) {
        std::cout << "weights histograms  not found in file " << puWeightFile << std::endl;
        return;
    }
    TH1D* weightedPU= (TH1D*)gen_pu->Clone("weightedPU");
    weightedPU->Multiply(puweights);

    // Rescaling weights in order to preserve same integral of events                               
    TH1D* weights = (TH1D*)puweights->Clone("rescaledWeights");
    weights->Scale( gen_pu->Integral(1,MAX_PU_REWEIGHT) / weightedPU->Integral(1,MAX_PU_REWEIGHT) );

    float sumPuWeights=0.;
    for (int i = 0; i<MAX_PU_REWEIGHT; i++) {
        float weight=1.;
        weight=weights->GetBinContent(i+1);
        sumPuWeights+=weight;
        puweights_.push_back(weight);
    }
}

float TaPAnalyzer::GetPUWeight(float pun) {
  
    float weight=1;
    if (sampleIndex_!=0 && pun<MAX_PU_REWEIGHT && puweights_.size()>0 && dopureweight_) 
        weight = puweights_[pun];
    return weight;
}

// miniAOD preselection + ECAL acceptance
bool TaPAnalyzer::isGammaPresel( float sceta, float pt, float r9, float chiso) {

    bool isPresel = false;

    // ECAL good acceptance
    if (fabs(sceta)>2.5) return false;
    if (fabs(sceta)>1.4442 && fabs(sceta)<1.566) return false;
  
    // miniAOD preselection
    if (r9>0.8)         return true;
    if (chiso<20)       return true;
    if ((chiso/pt)<0.3) return true;
  
    return isPresel;
}

bool TaPAnalyzer::isGammaSelected( float rho, float pt, float sceta, float r9, float chiso, float nhiso, float phoiso, float hoe, float sieie, bool passElectronVeto) {

    // classes: 0 = EB highR9, 1 = EB low R9, 2 = EE high R9, 3 = EE lowR9
    int etaclass = fabs(sceta)>1.5;
    int r9class  = r9<0.94;                   
    int theclass = 2.*etaclass + r9class;                  

    // cuts - hardcoded, v1
    float chiso_cut[4]     = { 5., 5., 5., 5. };     
    float phoiso_cut[4]    = { 1., 1., 0., 0. };  
    float sieie_cut[4]     = { 1.05e-02, 1.05e-02, 2.82e-02, 2.80e-02 };                                                                       
    float sieie_infCut[4]  = { 0.001, 0.001, 0.001, 0.001 };
    float hoe_cut[4]       = { 0.05, 0.05, 0.05, 0.05 };
  
    // effective areas - hardcoded 
    float phIsoAE[5] = { 0.21,0.20,0.14,0.22,0.31 };

    // EA corrections 
    int theEAregion  = effectiveAreaGammaRegion(sceta);   
    float corrPhIso1 = phoiso - rho*phIsoAE[theEAregion];

    // pT correction
    float corrPhIso = corrPhIso1 - 0.002*pt;

    if (chiso > chiso_cut[theclass])      return false;
    if (corrPhIso > phoiso_cut[theclass]) return false;
    if (hoe > hoe_cut[theclass])          return false;
    if (sieie > sieie_cut[theclass])      return false;
    if (sieie < sieie_infCut[theclass])   return false;

    // electron veto 
    // if (!passElectronVeto) return false;

    return true;
} 

int TaPAnalyzer::effectiveAreaGammaRegion(float sceta) {

    int theEAregion = 999;
    if (fabs(sceta)<=0.9) theEAregion = 0;
    if (fabs(sceta)<=1.5 && fabs(sceta)>0.9)  theEAregion = 1;
    if (fabs(sceta)<=2.0 && fabs(sceta)>1.5)  theEAregion = 2;   
    if (fabs(sceta)<=2.2 && fabs(sceta)>2.0)  theEAregion = 3;
    if (fabs(sceta)<=2.5 && fabs(sceta)>2.2)  theEAregion = 4;
    return theEAregion;
}


DEFINE_FWK_MODULE(TaPAnalyzer);

