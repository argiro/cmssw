#include "SimCalorimetry/EcalSimAlgos/interface/EBHitResponse_Ph2.h"
#include "SimCalorimetry/EcalSimAlgos/interface/APDSimParameters.h"
#include "SimCalorimetry/CaloSimAlgos/interface/CaloVSimParameterMap.h"
#include "SimCalorimetry/CaloSimAlgos/interface/CaloSimParameters.h"
#include "SimCalorimetry/CaloSimAlgos/interface/CaloVHitFilter.h"
#include "SimCalorimetry/CaloSimAlgos/interface/CaloVShape.h"
#include "Geometry/CaloGeometry/interface/CaloGenericDetId.h"
#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandGaussQ.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "FWCore/Utilities/interface/isFinite.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "CondFormats/EcalObjects/interface/EcalConstants.h"

//using namespace ecalPh2;

EBHitResponse_Ph2::EBHitResponse_Ph2(const CaloVSimParameterMap* parameterMap,
                                     const CaloVShape* shape,
                                     bool apdOnly,
                                     const APDSimParameters* apdPars = nullptr,
                                     const CaloVShape* apdShape = nullptr)
    :

      EcalHitResponse_Ph2(parameterMap, shape),

      m_apdOnly(apdOnly),
      m_apdPars(apdPars),
      m_apdShape(apdShape),
      m_timeOffVec(kNOffsets, apdParameters()->timeOffset()),
      pcub(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[0]),
      pqua(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[1]),
      plin(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[2]),
      pcon(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[3]),
      pelo(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[4]),
      pehi(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[5]),
      pasy(nullptr == apdPars ? 0 : apdParameters()->nonlParms()[6]),
      pext(nullptr == apdPars ? 0 : nonlFunc1(pelo)),
      poff(nullptr == apdPars ? 0 : nonlFunc1(pehi)),
      pfac(nullptr == apdPars ? 0 : (pasy - poff) * 2. / M_PI),
      m_isInitialized(false) {
  const EBDetId detId(EBDetId::detIdFromDenseIndex(0));
  const CaloSimParameters& parameters(parameterMap->simParameters(detId));

  const unsigned int rSize(parameters.readoutFrameSize());
  const unsigned int nPre(parameters.binOfMaximum() - 1);

  const unsigned int size(EBDetId::kSizeForDenseIndexing);

  m_vSam.reserve(size);

  for (unsigned int i(0); i != size; ++i) {
    m_vSam.emplace_back(CaloGenericDetId(detId.det(), detId.subdetId(), i), rSize, nPre);
  }
}

EBHitResponse_Ph2::~EBHitResponse_Ph2() {}

void EBHitResponse_Ph2::initialize(CLHEP::HepRandomEngine* engine) {
  m_isInitialized = true;
  for (unsigned int i(0); i != kNOffsets; ++i) {
    m_timeOffVec[i] += CLHEP::RandGaussQ::shoot(engine, 0, apdParameters()->timeOffWidth());
  }
}

const APDSimParameters* EBHitResponse_Ph2::apdParameters() const {
  assert(nullptr != m_apdPars);
  return m_apdPars;
}

const CaloVShape* EBHitResponse_Ph2::apdShape() const {
  assert(nullptr != m_apdShape);
  return m_apdShape;
}

void EBHitResponse_Ph2::putAPDSignal(const DetId& detId, double npe, double time) {
  const CaloSimParameters& parameters(*params(detId));

  const double energyFac(1. / parameters.simHitToPhotoelectrons(detId));

  //   std::cout<<"******** Input APD Npe="<<npe<<", Efactor="<<energyFac
  //	    <<", Energy="<<npe*energyFac
  //	    <<", nonlFunc="<<nonlFunc( npe*energyFac )<<std::endl ;

  const double signal(npe * nonlFunc(npe * energyFac));

  const double jitter(time - timeOfFlight(detId));

  if (!m_isInitialized) {
    throw cms::Exception("LogicError") << "EBHitResponse_Ph2::putAPDSignal called without initializing\n";
  }

  const double tzero(apdShape()->timeToRise() - jitter - offsets()[EBDetId(detId).denseIndex() % kNOffsets] -
                     ecalPh2::Samp_Period * (parameters.binOfMaximum() - phaseShift()));

  double binTime(tzero);

  EcalSamples& result(*findSignal(detId));

  for (unsigned int bin(0); bin != result.size(); ++bin) {
    result[bin] += (*apdShape())(binTime)*signal;
    binTime += ecalPh2::Samp_Period;
  }
}

double EBHitResponse_Ph2::apdSignalAmplitude(const PCaloHit& hit, CLHEP::HepRandomEngine* engine) const {
  int iddepth = (hit.depth() & PCaloHit::kEcalDepthIdMask);
  assert(1 == iddepth || 2 == iddepth);

  double npe(hit.energy() * (2 == iddepth ? apdParameters()->simToPELow() : apdParameters()->simToPEHigh()));

  // do we need to do Poisson statistics for the photoelectrons?
  if (apdParameters()->doPEStats() && !m_apdOnly) {
    CLHEP::RandPoissonQ randPoissonQ(*engine, npe);
    npe = randPoissonQ.fire();
  }
  assert(nullptr != m_intercal);
  double fac(1);
  findIntercalibConstant(hit.id(), fac);

  npe *= fac;

  //   edm::LogError( "EBHitResponse_Ph2" ) << "--- # photoelectrons for "
  /*   std::cout << "--- # photoelectrons for "
	     << EBDetId( hit.id() ) 
	     <<" is " << npe //;
	     <<std::endl ;*/

  return npe;
}

void EBHitResponse_Ph2::setIntercal(const EcalIntercalibConstantsMC* ical) { m_intercal = ical; }

void EBHitResponse_Ph2::findIntercalibConstant(const DetId& detId, double& icalconst) const {
  EcalIntercalibConstantMC thisconst(1.);

  if (nullptr == m_intercal) {
    edm::LogError("EBHitResponse_Ph2") << "No intercal constant defined for EBHitResponse_Ph2";
  } else {
    const EcalIntercalibConstantMCMap& icalMap(m_intercal->getMap());
    EcalIntercalibConstantMCMap::const_iterator icalit(icalMap.find(detId));
    if (icalit != icalMap.end()) {
      thisconst = *icalit;
      if (thisconst == 0.)
        thisconst = 1.;
    } else {
      edm::LogError("EBHitResponse_Ph2") << "No intercalib const found for xtal " << detId.rawId()
                                         << "! something wrong with EcalIntercalibConstants in your DB? ";
    }
  }
  icalconst = thisconst;
}

void EBHitResponse_Ph2::initializeHits() {
  if (!index().empty())
    blankOutUsedSamples();

  const unsigned int bSize(EBDetId::kSizeForDenseIndexing);

  if (m_apdNpeVec.empty()) {
    m_apdNpeVec = std::vector<double>(bSize, (double)0.0);
    m_apdTimeVec = std::vector<double>(bSize, (double)0.0);
  }
}

void EBHitResponse_Ph2::finalizeHits() {
  const unsigned int bSize(EBDetId::kSizeForDenseIndexing);
  if (apdParameters()->addToBarrel() || m_apdOnly) {
    for (unsigned int i(0); i != bSize; ++i) {
      if (0 < m_apdNpeVec[i]) {
        putAPDSignal(EBDetId::detIdFromDenseIndex(i), m_apdNpeVec[i], m_apdTimeVec[i]);

        // now zero out for next time
        m_apdNpeVec[i] = 0.;
        m_apdTimeVec[i] = 0.;
      }
    }
  }
}

void EBHitResponse_Ph2::add(const PCaloHit& hit, CLHEP::HepRandomEngine* engine) {
  if (!edm::isNotFinite(hit.time()) && (nullptr == hitFilter() || hitFilter()->accepts(hit))) {
    int iddepth = (hit.depth() & PCaloHit::kEcalDepthIdMask);
    if (0 == iddepth)  // for now take only nonAPD hits
    {
      if (!m_apdOnly)
        putAnalogSignal(hit, engine);
    } else  // APD hits here
    {
      if (apdParameters()->addToBarrel() || m_apdOnly) {
        const unsigned int icell(EBDetId(hit.id()).denseIndex());
        m_apdNpeVec[icell] += apdSignalAmplitude(hit, engine);
        if (0 == m_apdTimeVec[icell])
          m_apdTimeVec[icell] = hit.time();
      }
    }
  }
}

void EBHitResponse_Ph2::run(MixCollection<PCaloHit>& hits, CLHEP::HepRandomEngine* engine) {
  if (!index().empty())
    blankOutUsedSamples();

  const unsigned int bSize(EBDetId::kSizeForDenseIndexing);

  if (m_apdNpeVec.empty()) {
    m_apdNpeVec = std::vector<double>(bSize, (double)0.0);
    m_apdTimeVec = std::vector<double>(bSize, (double)0.0);
  }

  for (MixCollection<PCaloHit>::MixItr hitItr(hits.begin()); hitItr != hits.end(); ++hitItr) {
    const PCaloHit& hit(*hitItr);
    const int bunch(hitItr.bunch());
    if (minBunch() <= bunch && maxBunch() >= bunch && !edm::isNotFinite(hit.time()) &&
        (nullptr == hitFilter() || hitFilter()->accepts(hit))) {
      int iddepth = (hit.depth() & PCaloHit::kEcalDepthIdMask);
      if (0 == iddepth)  // for now take only nonAPD hits
      {
        if (!m_apdOnly)
          putAnalogSignal(hit, engine);
      } else  // APD hits here
      {
        if (apdParameters()->addToBarrel() || m_apdOnly) {
          const unsigned int icell(EBDetId(hit.id()).denseIndex());
          m_apdNpeVec[icell] += apdSignalAmplitude(hit, engine);
          if (0 == m_apdTimeVec[icell])
            m_apdTimeVec[icell] = hit.time();
        }
      }
    }
  }

  if (apdParameters()->addToBarrel() || m_apdOnly) {
    for (unsigned int i(0); i != bSize; ++i) {
      if (0 < m_apdNpeVec[i]) {
        putAPDSignal(EBDetId::detIdFromDenseIndex(i), m_apdNpeVec[i], m_apdTimeVec[i]);

        // now zero out for next time
        m_apdNpeVec[i] = 0.;
        m_apdTimeVec[i] = 0.;
      }
    }
  }
}

unsigned int EBHitResponse_Ph2::samplesSize() const { return m_vSam.size(); }

unsigned int EBHitResponse_Ph2::samplesSizeAll() const { return m_vSam.size(); }

const EcalHitResponse_Ph2::EcalSamples* EBHitResponse_Ph2::operator[](unsigned int i) const { return &m_vSam[i]; }

EcalHitResponse_Ph2::EcalSamples* EBHitResponse_Ph2::operator[](unsigned int i) { return &m_vSam[i]; }

EcalHitResponse_Ph2::EcalSamples* EBHitResponse_Ph2::vSam(unsigned int i) { return &m_vSam[i]; }

EcalHitResponse_Ph2::EcalSamples* EBHitResponse_Ph2::vSamAll(unsigned int i) { return &m_vSam[i]; }

const EcalHitResponse_Ph2::EcalSamples* EBHitResponse_Ph2::vSamAll(unsigned int i) const { return &m_vSam[i]; }
