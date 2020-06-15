#include "SimCalorimetry/EcalSimAlgos/interface/EcalElectronicsSim_Ph2.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalLiteDTUCoder.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EcalSimParameterMap_Ph2.h"
#include "DataFormats/EcalDigi/interface/EcalDataFrame_Ph2.h"

#include "CLHEP/Random/RandGaussQ.h"

#include <cstring>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <fstream>

EcalElectronicsSim_Ph2::EcalElectronicsSim_Ph2(const EcalSimParameterMap_Ph2* parameterMap,
                                               EcalLiteDTUCoder* coder,
                                               bool applyConstantTerm,
                                               double rmsConstantTerm)
    : m_simMap(parameterMap), m_theCoder(coder), m_thisCT(rmsConstantTerm), m_applyConstantTerm(applyConstantTerm) {}

EcalElectronicsSim_Ph2::~EcalElectronicsSim_Ph2() {}

void EcalElectronicsSim_Ph2::analogToDigital(CLHEP::HepRandomEngine* engine,
                                             EcalElectronicsSim_Ph2::EcalSamples& clf,
                                             EcalDataFrame_Ph2& df) const {
  //PG input signal is in pe.  Converted in GeV
  amplify(clf, engine);

  m_theCoder->analogToDigital(engine, clf, df);
}

void EcalElectronicsSim_Ph2::amplify(EcalElectronicsSim_Ph2::EcalSamples& clf, CLHEP::HepRandomEngine* engine) const {
  const double fac(m_simMap->simParameters(clf.id()).photoelectronsToAnalog());
  if (m_applyConstantTerm) {
    clf *= fac * CLHEP::RandGaussQ::shoot(engine, 1.0, m_thisCT);
  } else {
    clf *= fac;
  }
}
