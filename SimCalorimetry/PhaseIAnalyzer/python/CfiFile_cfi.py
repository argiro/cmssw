import FWCore.ParameterSet.Config as cms

phaseI = cms.EDAnalyzer('PhaseIAnalyzer',
                         BarrelDigis=cms.InputTag('simEcalUnsuppressedDigis','','DIGI')
)
