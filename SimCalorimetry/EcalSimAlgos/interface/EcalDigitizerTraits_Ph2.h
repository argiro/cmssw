#ifndef SimCalorimetry_EcalSimAlgos_EcalDigitizerTraits_Ph2_h
#define SimCalorimetry_EcalSimAlgos_EcalDigitizerTraits_Ph2_h

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalElectronicsSim_Ph2.h"
#include "CalibFormats/CaloObjects/interface/CaloTSamples.h"
#include "DataFormats/EcalDigi/interface/EcalDataFrame_Ph2.h"
#include "CondFormats/EcalObjects/interface/EcalConstants.h"
class EcalHitResponse;

class EBDigitizerTraits_Ph2 {
public:
  /// the digis collection
  typedef EBDigiCollectionPh2 DigiCollection;
  /// the dataframes
  typedef EcalDataFrame_Ph2 Digi;
  /// the electronics simulation
  typedef EcalElectronicsSim_Ph2 ElectronicsSim_Ph2;

  typedef CaloTSamples<float, ecalPh2::sampleSize> EcalSamples;

  static void fix(Digi& digi, edm::DataFrame df){};
};

#endif
