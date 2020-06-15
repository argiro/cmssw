#ifndef SimCalorimetry_EcalSimAlgos_EcalElectronicsSim_Ph2_h
#define SimCalorimetry_EcalSimAlgos_EcalElectronicsSim_Ph2_h

#include "CalibFormats/CaloObjects/interface/CaloTSamples.h"
#include "SimCalorimetry/CaloSimAlgos/interface/CaloVNoiseSignalGenerator.h"
#include "CondFormats/EcalObjects/interface/EcalConstants.h"
#include "DataFormats/EcalDigi/interface/EcalDataFrame_Ph2.h"

class EcalLiteDTUCoder;
//class EcalDataFrame_Ph2;
class EcalSimParameterMap_Ph2;

namespace CLHEP {
  class HepRandomEngine;
}

/* \class EcalElectronicsSim_Ph2
 * \brief Converts CaloDataFrame in CaloTimeSample and vice versa.
 * 
 */

class EcalElectronicsSim_Ph2 {
public:
  typedef CaloTSamples<float, ecalPh2::sampleSize> EcalSamples;
  EcalElectronicsSim_Ph2(const EcalSimParameterMap_Ph2* parameterMap,
                         EcalLiteDTUCoder* coder,
                         bool applyConstantTerm,
                         double rmsConstantTerm);

  ~EcalElectronicsSim_Ph2();

  /// from EcalSamples to EcalDataFrame_Ph2
  void analogToDigital(CLHEP::HepRandomEngine*, EcalSamples& clf, EcalDataFrame_Ph2& df) const;

  void newEvent() {}

  void setNoiseSignalGenerator(const CaloVNoiseSignalGenerator* noiseSignalGenerator) {
    theNoiseSignalGenerator = noiseSignalGenerator;
  }

private:
  /// input signal is in pe.  Converted in GeV
  void amplify(EcalSamples& clf, CLHEP::HepRandomEngine*) const;

  /// map of parameters

  const EcalSimParameterMap_Ph2* m_simMap;

  const CaloVNoiseSignalGenerator* theNoiseSignalGenerator;

  EcalLiteDTUCoder* m_theCoder;

  const double m_thisCT;
  const bool m_applyConstantTerm;
};

#endif
