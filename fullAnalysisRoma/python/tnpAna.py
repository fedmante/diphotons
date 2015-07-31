import FWCore.ParameterSet.Config as cms
import FWCore.Utilities.FileUtils as FileUtils

isMC = True

process = cms.Process("tnpAna")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff")

process.MessageLogger.cerr.FwkReport.reportEvery = 100000

from Configuration.AlCa.GlobalTag import GlobalTag
if (isMC):
    #process.GlobalTag = GlobalTag(process.GlobalTag, 'MCRUN2_74_V9A', '')           # 50ns
    process.GlobalTag = GlobalTag(process.GlobalTag, 'MCRUN2_74_V9', '')            # 25ns
else:
    process.GlobalTag = GlobalTag(process.GlobalTag, 'GR_P_V56', '')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( -1 ) )

process.source = cms.Source("PoolSource",
                            fileNames=cms.untracked.vstring(
        
        # Spring15, DY
        #"/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/RSGravToGG_kMpl-001_M-500_TuneCUEP8M1_13TeV-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt25ns_MCRUN2_74_V9-v1/150717_101901/0000/diphotonsMicroAOD_1.root")
        
        # MC Drell-Yan 50ns
        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_100.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_101.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_102.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_103.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_104.root"


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_105.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_106.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_107.root",


        "/store/group/phys_higgs/cmshgg/musella/flashgg/EXOSpring15_v1/Spring15BetaV2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/EXOSpring15_v1-Spring15BetaV2-v0-RunIISpring15DR74-Asympt50ns_MCRUN2_74_V9A-v2/150717_100654/0000/diphotonsMicroAOD_108.root",
        )
                            )

process.load("flashgg/MicroAOD/flashggPhotons_cfi")
process.load("flashgg/MicroAOD/flashggElectrons_cfi")
process.load("flashgg/MicroAOD/flashggDiPhotons_cfi")

process.TFileService = cms.Service("TFileService",
                                   fileName = cms.string("TaP_output.root"))

process.tnpAna = cms.EDAnalyzer('TaPAnalyzer',
                                VertexTag = cms.untracked.InputTag('offlineSlimmedPrimaryVertices'),
                                ElectronTag=cms.InputTag('flashggElectrons'),
                                genPhotonExtraTag = cms.InputTag("flashggGenPhotonsExtra"),    
                                DiPhotonTag = cms.untracked.InputTag('flashggDiPhotons'),
                                PileupTag = cms.untracked.InputTag('addPileupInfo'),

                                bits = cms.InputTag("TriggerResults","","HLT"),
                                objects = cms.InputTag("selectedPatTrigger"),

                                generatorInfo = cms.InputTag("generator"),
                                dopureweight = cms.untracked.int32(0),
                                sampleIndex  = cms.untracked.int32(101),   # 5
                                puWFileName  = cms.string('xxx'),   # chiara  
                                xsec         = cms.untracked.double(1.),
                                kfac         = cms.untracked.double(1.),
                                sumDataset   = cms.untracked.double(1.)
                                )

process.p = cms.Path(process.tnpAna)

