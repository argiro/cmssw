import FWCore.ParameterSet.Config as cms

phaseII = cms.EDAnalyzer('PhaseIIAnalyzer',
                         BarrelDigis=cms.InputTag('simEcalUnsuppressedDigis','','DIGI'),
                         record = cms.string('EcalLiteDTUPedestalsRcd')
)
