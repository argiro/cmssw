#ifndef DataFormats_EcalDigi_EcalDataFrame_Ph2_h
#define DataFormats_EcalDigi_EcalDataFrame_Ph2_h
#include "DataFormats/EcalDigi/interface/EcalLiteDTUSample.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/Common/interface/DataFrame.h"
#include "CondFormats/EcalObjects/interface/EcalConstants.h"

/** \class EcalDataFrame_Ph2
      
*/
class EcalDataFrame_Ph2 {
public:
  EcalDataFrame_Ph2() {}
  EcalDataFrame_Ph2(edm::DataFrame const& iframe) : m_data(iframe) {}

  virtual ~EcalDataFrame_Ph2() {}

  DetId id() const { return m_data.id(); }

  int size() const { return m_data.size(); }

  EcalLiteDTUSample operator[](int i) const { return m_data[i]; }
  EcalLiteDTUSample sample(int i) const { return m_data[i]; }

  // FIXME (shall we throw??)
  void setSize(int) {}
  void setSample(int i, EcalLiteDTUSample sam) { m_data[i] = sam; }

  int MAXSAMPLES = ecalPh2::sampleSize;

  edm::DataFrame const& frame() const { return m_data; }
  edm::DataFrame& frame() { return m_data; }

private:
  edm::DataFrame m_data;
};

#endif
