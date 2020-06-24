#ifndef SimCalorimetry_EcalSimProducers_EcalTimeDigiProducer_Ph2_h
#define SimCalorimetry_EcalSimProducers_EcalTimeDigiProducer_Ph2_h

#include "DataFormats/Math/interface/Error.h"

#include "SimCalorimetry/EcalSimAlgos/interface/EcalDigitizerTraits_Ph2.h"
#include "SimGeneral/MixingModule/interface/DigiAccumulatorMixMod.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/ProducesCollector.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include <vector>

class ESDigitizer;

class CaloGeometry;
class EcalSimParameterMap_Ph2;
class PileUpEventPrincipal;
class EcalTimeMapDigitizer;

namespace edm {
  class Event;
  class EventSetup;
  template <typename T>
  class Handle;
  class ParameterSet;
}  // namespace edm

class EcalTimeDigiProducer_Ph2 : public DigiAccumulatorMixMod {
public:
  EcalTimeDigiProducer_Ph2(const edm::ParameterSet& params, edm::ProducesCollector, edm::ConsumesCollector&);
  ~EcalTimeDigiProducer_Ph2() override;

  void initializeEvent(edm::Event const& e, edm::EventSetup const& c) override;
  void accumulate(edm::Event const& e, edm::EventSetup const& c) override;
  void accumulate(PileUpEventPrincipal const& e, edm::EventSetup const& c, edm::StreamID const&) override;
  void finalizeEvent(edm::Event& e, edm::EventSetup const& c) override;

private:
  typedef edm::Handle<std::vector<PCaloHit> > HitsHandle;
  void accumulateCaloHits(HitsHandle const& ebHandle, int bunchCrossing);

  void checkGeometry(const edm::EventSetup& eventSetup);

  void updateGeometry();

  const std::string m_EBdigiCollection;
  const edm::InputTag m_hitsProducerTagEB;
  const edm::EDGetTokenT<std::vector<PCaloHit> > m_hitsProducerTokenEB;

private:
  int m_timeLayerEB;
  const CaloGeometry* m_Geometry;

  EcalTimeMapDigitizer* m_BarrelDigitizer;
};

#endif
