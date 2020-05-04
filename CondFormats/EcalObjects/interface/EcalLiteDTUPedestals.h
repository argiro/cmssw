#ifndef CondFormats_EcalObject_EcalLiteDTUPedestals_h
#define CondFormats_EcalObject_EcalLiteDTUPedestals_h

#include "CondFormats/Serialization/interface/Serializable.h"
#include "CondFormats/EcalObjects/interface/EcalCondObjectContainer.h"
#include "CondFormats/EcalObjects/interface/EcalConstants.h"

class EcalLiteDTUPedestals {
public:
  float meanarray[2] = {13., 8.};
  float rmsarray[2] = {2.8, 1.2};

  int setMean(int i, float value) {
    if (i >= ecalPh2::NGAINS || i < 0)
      return -1;
    else
      meanarray[i] = value;
    return 1;
  }

  int setRMS(int i, float value) {
    if (i >= ecalPh2::NGAINS || i < 0)
      return -1;
    else
      rmsarray[i] = value;
    return 1;
  }

  float mean(int i) const { return meanarray[i]; }

  float rms(int i) const { return rmsarray[i]; }

  COND_SERIALIZABLE;
};

typedef EcalCondObjectContainer<EcalLiteDTUPedestals> EcalLiteDTUPedestalsMap;
typedef EcalLiteDTUPedestalsMap::const_iterator EcalLiteDTUPedestalsMapIterator;

#endif
