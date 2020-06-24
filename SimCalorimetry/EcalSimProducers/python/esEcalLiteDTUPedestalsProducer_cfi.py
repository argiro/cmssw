import FWCore.ParameterSet.Config as cms

EcalLiteDTUPedestalsRcd =  cms.ESSource("EmptyESSource",
                                recordName = cms.string("EcalLiteDTUPedestalsRcd"),
                                firstValid = cms.vuint32(1),
                                iovIsRunNotTime = cms.bool(True)
                                )

EcalLiteDTUPedestals = cms.ESProducer("EcalLiteDTUPedestalsESProducer"),
