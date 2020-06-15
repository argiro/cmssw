#ifndef SimCalorimetry_EcalSimAlgos_EcalTDigitizer_Ph2_h
#define SimCalorimetry_EcalSimAlgos_EcalTDigitizer_Ph2_h

/** Turns hits into digis.  Assumes that 
    there's an ElectroncsSim class with the
    interface analogToDigital(CLHEP::HepRandomEngine*, const CaloSamples &, Digi &);
*/

#include "SimDataFormats/CrossingFrame/interface/MixCollection.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "CalibFormats/CaloObjects/interface/CaloTSamplesBase.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalBaseSignalGenerator.h"

class EcalHitResponse;
class EcalBaseSignalGenerator;

namespace CLHEP {
  class HepRandomEngine;
}

template <class Traits>
class EcalTDigitizer_Ph2 {
public:
  typedef typename Traits::ElectronicsSim_Ph2 ElectronicsSim_Ph2;
  typedef typename Traits::Digi Digi;
  typedef typename Traits::DigiCollection DigiCollection;
  typedef typename Traits::EcalSamples EcalSamples;

  EcalTDigitizer_Ph2<Traits>(EcalHitResponse* hitResponse, ElectronicsSim_Ph2* electronicsSim, bool addNoise);

  virtual ~EcalTDigitizer_Ph2<Traits>();

  void add(const std::vector<PCaloHit>& hits, int bunchCrossing, CLHEP::HepRandomEngine*);

  virtual void initializeHits();

  virtual void run(DigiCollection& output, CLHEP::HepRandomEngine*);

  virtual void run(MixCollection<PCaloHit>& input, DigiCollection& output) { assert(0); }

  void setNoiseSignalGenerator(EcalBaseSignalGenerator* noiseSignalGenerator);

  void addNoiseSignals();

protected:
  bool addNoise() const;

  const EcalHitResponse* hitResponse() const;

  const ElectronicsSim_Ph2* elecSim() const;

private:
  EcalHitResponse* m_hitResponse;
  ElectronicsSim_Ph2* m_electronicsSim;
  bool m_addNoise;

  EcalBaseSignalGenerator* theNoiseSignalGenerator;
};

#endif
