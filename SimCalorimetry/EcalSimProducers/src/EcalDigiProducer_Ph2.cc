#include "FWCore/Framework/interface/Event.h"
#include "SimCalorimetry/EcalSimProducers/interface/EcalDigiProducer_Ph2.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EBHitResponse.h"
#include "SimCalorimetry/CaloSimAlgos/interface/CaloHitResponse.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalSimParameterMap_Ph2.h"
#include "SimCalorimetry/EcalSimAlgos/interface/APDSimParameters.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalLiteDTUCoder.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalElectronicsSim_Ph2.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "CalibFormats/CaloObjects/interface/CaloSamples.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/ProducerBase.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "SimGeneral/MixingModule/interface/PileUpEventPrincipal.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "CondFormats/EcalObjects/interface/EcalLiteDTUPedestals.h"
#include "CondFormats/DataRecord/interface/EcalLiteDTUPedestalsRcd.h"
#include "CondFormats/EcalObjects/interface/EcalIntercalibConstantsMC.h"
#include "CondFormats/DataRecord/interface/EcalIntercalibConstantsMCRcd.h"
#include "CalibCalorimetry/EcalLaserCorrection/interface/EcalLaserDbService.h"
#include "CalibCalorimetry/EcalLaserCorrection/interface/EcalLaserDbRecord.h"
#include "CondFormats/EcalObjects/interface/EcalADCToGeVConstant.h"
#include "CondFormats/DataRecord/interface/EcalADCToGeVConstantRcd.h"

#include "CondFormats/EcalObjects/interface/EcalCATIAGainRatios.h"
#include "CondFormats/DataRecord/interface/EcalCATIAGainRatiosRcd.h"
//***********************************

EcalDigiProducer_Ph2::EcalDigiProducer_Ph2(const edm::ParameterSet& params,
                                           edm::ProducesCollector producesCollector,
                                           edm::ConsumesCollector& iC)
    : EcalDigiProducer_Ph2(params, iC) {
  if (m_apdSeparateDigi)
    producesCollector.produces<EBDigiCollectionPh2>(m_apdDigiTag);

  producesCollector.produces<EBDigiCollectionPh2>(m_EBdigiCollection);
}

// version for Pre-Mixing, for use outside of MixingModule
EcalDigiProducer_Ph2::EcalDigiProducer_Ph2(const edm::ParameterSet& params, edm::ConsumesCollector& iC)
    : DigiAccumulatorMixMod(),
      m_APDShape(true),
      m_EBShape(true),

      m_EBdigiCollection(params.getParameter<std::string>("EBdigiCollectionPh2")),

      m_hitsProducerTag(params.getParameter<std::string>("hitsProducer")),
      m_useLCcorrection(params.getUntrackedParameter<bool>("UseLCcorrection")),
      m_apdSeparateDigi(params.getParameter<bool>("apdSeparateDigi")),

      m_EBs25notCont(params.getParameter<double>("EBs25notContainment")),

      m_readoutFrameSize(params.getParameter<int>("readoutFrameSize")),
      m_ParameterMap(new EcalSimParameterMap_Ph2(params.getParameter<double>("simHitToPhotoelectronsBarrel"),
                                                 params.getParameter<double>("photoelectronsToAnalogBarrel"),
                                                 params.getParameter<double>("samplingFactor"),
                                                 params.getParameter<double>("timePhase"),
                                                 m_readoutFrameSize,
                                                 params.getParameter<int>("binOfMaximum"),
                                                 params.getParameter<bool>("doPhotostatistics"),
                                                 params.getParameter<bool>("syncPhase"))),

      m_apdDigiTag(params.getParameter<std::string>("apdDigiTag")),
      m_apdParameters(new APDSimParameters(params.getParameter<bool>("apdAddToBarrel"),
                                           m_apdSeparateDigi,
                                           params.getParameter<double>("apdSimToPELow"),
                                           params.getParameter<double>("apdSimToPEHigh"),
                                           params.getParameter<double>("apdTimeOffset"),
                                           params.getParameter<double>("apdTimeOffWidth"),
                                           params.getParameter<bool>("apdDoPEStats"),
                                           m_apdDigiTag,
                                           params.getParameter<std::vector<double> >("apdNonlParms"))),

      m_APDResponse(
          !m_apdSeparateDigi
              ? nullptr
              : new EBHitResponse_Ph2(m_ParameterMap.get(), &m_EBShape, true, m_apdParameters.get(), &m_APDShape)),

      m_EBResponse(new EBHitResponse_Ph2(m_ParameterMap.get(),
                                         &m_EBShape,
                                         false,  // barrel
                                         m_apdParameters.get(),
                                         &m_APDShape)),

      m_PreMix1(params.getParameter<bool>("EcalPreMixStage1")),
      m_PreMix2(params.getParameter<bool>("EcalPreMixStage2")),

      m_doEB(params.getParameter<bool>("doEB")),

      m_APDDigitizer(nullptr),
      m_BarrelDigitizer(nullptr),
      m_ElectronicsSim(nullptr),
      m_Coder(nullptr),
      m_APDElectronicsSim(nullptr),
      m_APDCoder(nullptr),
      m_Geometry(nullptr),
      m_EBCorrNoise({{nullptr, nullptr}})

{
  //std::cout<<"[EcalDigiProducer_Ph2] Constructing producer"<<     "frameSize: " <<  m_readoutFrameSize<< std::endl;
  if (m_doEB)
    iC.consumes<std::vector<PCaloHit> >(edm::InputTag(m_hitsProducerTag, "EcalHitsEB"));

  const std::vector<double> ebCorMatG10Ph2 = params.getParameter<std::vector<double> >("EBCorrNoiseMatrixG10Ph2");
  const std::vector<double> ebCorMatG01Ph2 = params.getParameter<std::vector<double> >("EBCorrNoiseMatrixG01Ph2");

  const bool applyConstantTerm = params.getParameter<bool>("applyConstantTerm");
  const double rmsConstantTerm = params.getParameter<double>("ConstantTerm");

  const bool addNoise = params.getParameter<bool>("doENoise");
  const bool cosmicsPhase = params.getParameter<bool>("cosmicsPhase");
  const double cosmicsShift = params.getParameter<double>("cosmicsShift");

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // further phase for cosmics studies
  if (cosmicsPhase) {
    if (m_doEB)
      m_EBResponse->setPhaseShift(1. + cosmicsShift);
  }

  EcalCorrMatrix_Ph2 ebMatrix[2];

  //std::cout<<"[EcalDigiProducer_Ph2] Check size10: "<<ebCorMatG10Ph2.size()<<std::endl;
  //std::cout<<"[EcalDigiProducer_Ph2] Check size01: "<<ebCorMatG01Ph2.size()<<std::endl;
  assert(ebCorMatG10Ph2.size() == m_readoutFrameSize);
  assert(ebCorMatG01Ph2.size() == m_readoutFrameSize);

  //std::cout<<"[EcalDigiProducer_Ph2] Check Cor Mat consistency"<<std::endl;
  assert(1.e-7 > fabs(ebCorMatG10Ph2[0] - 1.0));
  assert(1.e-7 > fabs(ebCorMatG01Ph2[0] - 1.0));

  //std::cout<<"[EcalDigiProducer_Ph2] Check row of cor and col mat"<<std::endl;
  for (unsigned int row(0); row != m_readoutFrameSize; ++row) {
    assert(0 == row || 1. >= ebCorMatG10Ph2[row]);
    assert(0 == row || 1. >= ebCorMatG01Ph2[row]);

    // std::cout<<"row "<<row<<std::endl;

    for (unsigned int column(0); column <= row; ++column) {
      const unsigned int index(row - column);
      // std::cout<<"index: "<<index<<std::endl;
      // std::cout<<"ebCorMat10: "<<ebCorMatG10Ph2[ index ] <<std::endl;
      // std::cout<<"ebCorMat01: "<<ebCorMatG01Ph2[ index ] <<std::endl;
      ebMatrix[0](row, column) = ebCorMatG10Ph2[index];
      //std::cout<<"col mat0: "<<column<<std::endl;
      ebMatrix[1](row, column) = ebCorMatG01Ph2[index];
      //std::cout<<"col mat1 "<<column<<std::endl;
    }
  }
  //std::cout<<"[EcalDigiProducer_Ph2] Reset"<<std::endl;
  m_EBCorrNoise[0].reset(new CorrelatedNoisifier<EcalCorrMatrix_Ph2>(ebMatrix[0]));
  m_EBCorrNoise[1].reset(new CorrelatedNoisifier<EcalCorrMatrix_Ph2>(ebMatrix[1]));

  //std::cout<<"[EcalDigiProducer_Ph2] More reset"<<std::endl;
  m_Coder.reset(new EcalLiteDTUCoder(addNoise, m_PreMix1, m_EBCorrNoise[0].get(), m_EBCorrNoise[1].get()));

  m_ElectronicsSim.reset(
      new EcalElectronicsSim_Ph2(m_ParameterMap.get(), m_Coder.get(), applyConstantTerm, rmsConstantTerm));

  if (m_apdSeparateDigi) {
    m_APDCoder.reset(new EcalLiteDTUCoder(false, m_PreMix1, m_EBCorrNoise[0].get(), m_EBCorrNoise[1].get()));

    m_APDElectronicsSim.reset(
        new EcalElectronicsSim_Ph2(m_ParameterMap.get(), m_APDCoder.get(), applyConstantTerm, rmsConstantTerm));

    m_APDDigitizer.reset(new EBDigitizer_Ph2(m_APDResponse.get(), m_APDElectronicsSim.get(), false));
  }
  //std::cout<<"[EcalDigiProducer_Ph2] Barrel reset"<<std::endl;
  if (m_doEB) {
    m_BarrelDigitizer.reset(new EBDigitizer_Ph2(m_EBResponse.get(), m_ElectronicsSim.get(), addNoise));
  }

  // std::cout<<"[EcalDigiProducer_Ph2] End constructor"<<std::endl;
}

EcalDigiProducer_Ph2::~EcalDigiProducer_Ph2() {}

void EcalDigiProducer_Ph2::initializeEvent(edm::Event const& event, edm::EventSetup const& eventSetup) {
  edm::Service<edm::RandomNumberGenerator> rng;
  randomEngine_ = &rng->getEngine(event.streamID());

  checkGeometry(eventSetup);
  checkCalibrations(event, eventSetup);
  if (m_doEB) {
    m_BarrelDigitizer->initializeHits();
    if (m_apdSeparateDigi) {
      m_APDDigitizer->initializeHits();
    }
  }
}

void EcalDigiProducer_Ph2::accumulateCaloHits(HitsHandle const& ebHandle, int bunchCrossing) {
  if (m_doEB && ebHandle.isValid()) {
    m_BarrelDigitizer->add(*ebHandle.product(), bunchCrossing, randomEngine_);

    if (m_apdSeparateDigi) {
      m_APDDigitizer->add(*ebHandle.product(), bunchCrossing, randomEngine_);
    }
  }
}

void EcalDigiProducer_Ph2::accumulate(edm::Event const& e, edm::EventSetup const& eventSetup) {
  // Step A: Get Inputs
  edm::Handle<std::vector<PCaloHit> > ebHandle;
  if (m_doEB) {
    m_EBShape.setEventSetup(eventSetup);   // need to set the eventSetup here, otherwise pre-mixing module will not wrk
    m_APDShape.setEventSetup(eventSetup);  //
    edm::InputTag ebTag(m_hitsProducerTag, "EcalHitsEB");
    e.getByLabel(ebTag, ebHandle);
  }

  accumulateCaloHits(ebHandle, 0);
}

void EcalDigiProducer_Ph2::accumulate(PileUpEventPrincipal const& e,
                                      edm::EventSetup const& eventSetup,
                                      edm::StreamID const& streamID) {
  // Step A: Get Inputs
  edm::Handle<std::vector<PCaloHit> > ebHandle;
  if (m_doEB) {
    edm::InputTag ebTag(m_hitsProducerTag, "EcalHitsEB");
    e.getByLabel(ebTag, ebHandle);
  }
  accumulateCaloHits(ebHandle, e.bunchCrossing());
}

void EcalDigiProducer_Ph2::finalizeEvent(edm::Event& event, edm::EventSetup const& eventSetup) {
  // Step B: Create empty output
  std::unique_ptr<EBDigiCollectionPh2> apdResult(!m_apdSeparateDigi || !m_doEB ? nullptr : new EBDigiCollectionPh2());
  std::unique_ptr<EBDigiCollectionPh2> barrelResult(new EBDigiCollectionPh2());

  // run the algorithm

  if (m_doEB) {
    //std::cout << " EcalDigiProducer_Ph2 ::finalize, running digitizer "
    //<< event.run() <<  " " << event.id() << std::endl;

    m_BarrelDigitizer->run(*barrelResult, randomEngine_);
    cacheEBDigis(&*barrelResult);

    edm::LogInfo("DigiInfo") << "EB Digis: " << barrelResult->size();

    if (m_apdSeparateDigi) {
      m_APDDigitizer->run(*apdResult, randomEngine_);
      edm::LogInfo("DigiInfo") << "APD Digis: " << apdResult->size();
    }
  }

  // Step D: Put outputs into event
  if (m_apdSeparateDigi) {
    //event.put(std::move(apdResult),    m_apdDigiTag         ) ;
  }

  event.put(std::move(barrelResult), m_EBdigiCollection);

  randomEngine_ = nullptr;  // to prevent access outside event
}

void EcalDigiProducer_Ph2::beginLuminosityBlock(edm::LuminosityBlock const& lumi, edm::EventSetup const& setup) {
  edm::Service<edm::RandomNumberGenerator> rng;
  if (!rng.isAvailable()) {
    throw cms::Exception("Configuration") << "RandomNumberGenerator service is not available.\n"
                                             "You must add the service in the configuration file\n"
                                             "or remove the module that requires it.";
  }
  CLHEP::HepRandomEngine* engine = &rng->getEngine(lumi.index());

  if (m_doEB) {
    if (nullptr != m_APDResponse)
      m_APDResponse->initialize(engine);
    m_EBResponse->initialize(engine);
  }
}

void EcalDigiProducer_Ph2::checkCalibrations(const edm::Event& event, const edm::EventSetup& eventSetup) {
  // Pedestals from event setup

  edm::ESHandle<EcalLiteDTUPedestalsMap> dbPed;
  eventSetup.get<EcalLiteDTUPedestalsRcd>().get(dbPed);
  const EcalLiteDTUPedestalsMap* pedestals(dbPed.product());

  m_Coder->setPedestals(pedestals);
  if (nullptr != m_APDCoder)
    m_APDCoder->setPedestals(pedestals);

  // Ecal Intercalibration Constants
  edm::ESHandle<EcalIntercalibConstantsMC> pIcal;
  eventSetup.get<EcalIntercalibConstantsMCRcd>().get(pIcal);
  const EcalIntercalibConstantsMC* ical(pIcal.product());

  m_Coder->setIntercalibConstants(ical);
  if (nullptr != m_APDCoder)
    m_APDCoder->setIntercalibConstants(ical);

  m_EBResponse->setIntercal(ical);
  if (nullptr != m_APDResponse)
    m_APDResponse->setIntercal(ical);

  // Ecal LaserCorrection Constants
  edm::ESHandle<EcalLaserDbService> laser;
  eventSetup.get<EcalLaserDbRecord>().get(laser);
  const edm::TimeValue_t eventTimeValue = event.time().value();

  m_EBResponse->setEventTime(eventTimeValue);
  m_EBResponse->setLaserConstants(laser.product(), m_useLCcorrection);

  // ADC -> GeV Scale
  edm::ESHandle<EcalADCToGeVConstant> pAgc;
  eventSetup.get<EcalADCToGeVConstantRcd>().get(pAgc);
  const EcalADCToGeVConstant* agc = pAgc.product();

  // Gain Ratios
  edm::ESHandle<EcalCATIAGainRatios> pRatio;
  eventSetup.get<EcalCATIAGainRatiosRcd>().get(pRatio);
  const EcalCATIAGainRatios* gr = pRatio.product();
  m_Coder->setGainRatios(gr);
  if (nullptr != m_APDCoder)
    m_APDCoder->setGainRatios(gr);

  const double EBscale((agc->getEBValue()) * ecalPh2::gains[1] * (m_Coder->MAXADC) * m_EBs25notCont);

  LogDebug("EcalDigi") << " GeV/ADC = " << agc->getEBValue() << "\n"
                       << " notCont = " << m_EBs25notCont << "\n"
                       << " saturation for EB = " << EBscale << ", " << m_EBs25notCont;

  m_Coder->setFullScaleEnergy(EBscale);
  if (nullptr != m_APDCoder)
    m_APDCoder->setFullScaleEnergy(EBscale);
}

void EcalDigiProducer_Ph2::checkGeometry(const edm::EventSetup& eventSetup) {
  // TODO find a way to avoid doing this every event
  edm::ESHandle<CaloGeometry> hGeometry;
  eventSetup.get<CaloGeometryRecord>().get(hGeometry);

  const CaloGeometry* pGeometry = &*hGeometry;

  if (pGeometry != m_Geometry) {
    m_Geometry = pGeometry;
    updateGeometry();
  }
}

void EcalDigiProducer_Ph2::updateGeometry() {
  if (m_doEB) {
    if (nullptr != m_APDResponse)
      m_APDResponse->setGeometry(m_Geometry->getSubdetectorGeometry(DetId::Ecal, EcalBarrel));
    m_EBResponse->setGeometry(m_Geometry->getSubdetectorGeometry(DetId::Ecal, EcalBarrel));
  }
}

void EcalDigiProducer_Ph2::setEBNoiseSignalGenerator(EcalBaseSignalGenerator* noiseGenerator) {
  //noiseGenerator->setParameterMap(theParameterMap);
  if (nullptr != m_BarrelDigitizer)
    m_BarrelDigitizer->setNoiseSignalGenerator(noiseGenerator);
}

void EcalDigiProducer_Ph2::beginRun(edm::Run const& run, edm::EventSetup const& setup) {
  m_EBShape.setEventSetup(setup);
  m_APDShape.setEventSetup(setup);
}
