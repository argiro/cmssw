#include <memory>
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/ESProductHost.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CondFormats/DataRecord/interface/EcalLiteDTUPedestalsRcd.h"
#include "CondFormats/EcalObjects/interface/EcalLiteDTUPedestals.h"
#include "CondFormats/EcalObjects/src/classes.h"
#include "CondFormats/EcalObjects/interface/EcalConstants.h"

//
// class declaration
//
//const int kEBChannels = 61200;

class EcalLiteDTUPedestalsESProducer : public edm::ESProducer {
public:
  EcalLiteDTUPedestalsESProducer(const edm::ParameterSet& iConfig);

  typedef std::unique_ptr<EcalLiteDTUPedestalsMap> ReturnType;

  ReturnType produce(const EcalLiteDTUPedestalsRcd& iRecord);

private:
  edm::ParameterSet pset_;
};

EcalLiteDTUPedestalsESProducer::EcalLiteDTUPedestalsESProducer(const edm::ParameterSet& iConfig) : pset_(iConfig) {
  //the following line is needed to tell the framework what
  // data is being produced
  //std::cout<<"*********Creating EcalLiteDTUPedestalsESProducer"<<std::endl;
  setWhatProduced(this);
}
////
EcalLiteDTUPedestalsESProducer::ReturnType EcalLiteDTUPedestalsESProducer::produce(
    const EcalLiteDTUPedestalsRcd& iRecord) {
  //std::cout<<"********Starting Production"<<std::endl;
  auto prod = std::make_unique<EcalLiteDTUPedestalsMap>();

  //std::cout<<"**********Set EB Values "<<std::endl;

  for (unsigned int iChannel = 0; iChannel < ecalPh2::kEBChannels; iChannel++) {
    EBDetId myEBDetId = EBDetId::unhashIndex(iChannel);
    EcalLiteDTUPedestals ped;
    ped.setMean(0, 15.);
    ped.setRMS(0, 2.5);

    ped.setMean(1, 12.);
    ped.setRMS(1, 2.);

    prod->insert(std::make_pair(myEBDetId, ped));
  }

  //std::cout<<prod->size()<<std::endl;

  //std::cout<<"***********Returning"<<std::endl;
  return prod;
}

//Define this as a plug-in
DEFINE_FWK_EVENTSETUP_MODULE(EcalLiteDTUPedestalsESProducer);
