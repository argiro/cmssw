// -*- C++ -*-
//
// Package:    PhaseI/PhaseIAnalyzer
// Class:      PhaseIAnalyzer
//
/**\class PhaseIAnalyzer PhaseIAnalyzer.cc PhaseI/PhaseIAnalyzer/plugins/PhaseIAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Dario Soldi
//         Created:  Tue, 16 Jan 2018 11:56:05 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

//My includes
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"
#include "CondFormats/DataRecord/interface/EcalIntercalibConstantsMCRcd.h"
#include "CondFormats/EcalObjects/interface/EcalIntercalibConstantsMC.h"

#include "PhaseIAnalyzer.h"

#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "CondFormats/EcalObjects/interface/EcalConstants.h"

using namespace std;
using namespace edm;

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//

PhaseIAnalyzer::PhaseIAnalyzer(const edm::ParameterSet& iConfig) {
  //now do what ever initialization is needed
  usesResource("TFileService");

  digiTagEB_ = iConfig.getParameter<edm::InputTag>("BarrelDigis");

  digiTokenEB_ = consumes<EBDigiCollection>(digiTagEB_);

  //Files:
  edm::Service<TFileService> fs;
  //Histograms

  for (unsigned int isample = 0; isample < ecalPh1::sampleSize; isample++) {
    EBEnergyHisto[isample] =
        fs->make<TH1I>(Form("EnergyEB_%d", isample), Form("Energy sample %d  Barrel;ADC", isample), 950, 100, 2000);
    EBGainHisto[isample] =
        fs->make<TH1I>(Form("GainEB_%d", isample), Form("Gain Barrel sample %d;Gain", isample), 5, 0, 4);
  }
  SingleChannelE = fs->make<TH1I>("SingleChannelE", "Energy single channel Barrel;ADC", 17, -0.5, 16.5);
  SingleChannelELow = fs->make<TH1I>("SingleChannelELow", "Energy single channel Barrel;ADC", 17, -0.5, 16.5);

  Char_t histo[200];

  sprintf(histo, "EcalDigiTaskBarrelOccupancy");
  meEBDigiOccupancy_ = fs->make<TH2D>(histo, histo, 360, 0., 360., 170, -85., 85.);

  sprintf(histo, "EcalDigiTaskBarrelOccupancyHigh");
  meEBDigiOccupancyHigh_ = fs->make<TH2D>(histo, histo, 360, 0., 360., 170, -85., 85.);

  sprintf(histo, "EcalDigiTaskBarrelOccupancyLow");
  meEBDigiOccupancyLow_ = fs->make<TH2D>(histo, histo, 360, 0., 360., 170, -85., 85.);

  sprintf(histo, "EcalDigiTaskBarrelOccupancyMid");
  meEBDigiOccupancyMid_ = fs->make<TH2D>(histo, histo, 360, 0., 360., 170, -85., 85.);

  sprintf(histo, "EcalDigiTaskBarrelDigisMultiplicity");
  meEBDigiMultiplicity_ = fs->make<TH1D>(histo, histo, 612, 0., 61200);

  for (unsigned int i = 0; i < ecalPh1::sampleSize; i++) {
    sprintf(histo, "EcalDigiTaskBarrelAnalogPulse%02d", i + 1);
    meEBDigiADCAnalog_[i] = fs->make<TH1D>(histo, histo, 4000, 0., 400.);

    sprintf(histo, "EcalDigiTaskBarrelADCPulse%02dGain0_Saturated", i + 1);
    meEBDigiADCgS_[i] = fs->make<TH1D>(histo, histo, 4096, -0.5, 4095.5);
    sprintf(histo, "EcalDigiTaskBarrelGainPulse%02d", i + 1);
    meEBDigiGain_[i] = fs->make<TH1D>(histo, histo, 4, 0, 4);
  }

  sprintf(histo, "EcalDigiTaskBarrelPedestalForPreSample");
  meEBPedestal_ = fs->make<TH1D>(histo, histo, 4096, -0.5, 4095.5);

  sprintf(histo, "EcalDigiTaskBarrelMaximumPositionGt100ADC");
  meEBMaximumgt100ADC_ = fs->make<TH1D>(histo, histo, 10, 0., 10.);

  sprintf(histo, "EcalDigiTaskBarrelMaximumPositionGt10ADC");
  meEBMaximumgt10ADC_ = fs->make<TH1D>(histo, histo, 10, 0., 10.);

  //   sprintf (histo, "EcalDigiTask Barrel ADC counts after gain switch" ) ;
  //meEBnADCafterSwitch_ = fs->make<TH1D>histo, histo, 10, 0., 10.) ;
}

PhaseIAnalyzer::~PhaseIAnalyzer() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//

// ------------ method called for each event  ------------
void PhaseIAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  Handle<EBDigiCollection> pDigiEB;
  iEvent.getByToken(digiTokenEB_, pDigiEB);

  std::vector<double> ebAnalogSignal;
  std::vector<double> ebADCCounts;
  std::vector<double> ebADCGains_temp;
  std::vector<double> ebADCGains;

  ebAnalogSignal.reserve(EBDataFrame::MAXSAMPLES);
  ebADCCounts.reserve(EBDataFrame::MAXSAMPLES);
  ebADCGains_temp.reserve(EBDataFrame::MAXSAMPLES);
  ebADCGains.reserve(EBDataFrame::MAXSAMPLES);

  int nDigis = 0;
  int LowGain(0), MidGain(0), HighGain(0);
  int maxADCValue = 0;

  for (EBDigiCollection::const_iterator pDigi = pDigiEB->begin(); pDigi != pDigiEB->end(); ++pDigi) {
    LowGain = 0;
    MidGain = 0;
    HighGain = 0;
    maxADCValue = 0;

    EBDataFrame digi(*pDigi);
    int nrSamples = digi.size();
    std::cout << "NSamples found: " << nrSamples << std::endl;
    EBDetId ebid = digi.id();
    std::cout << " Crystall ID " << ebid << std::endl;
    nDigis++;
    std::cout << " nDigis aaaaaaa " << nDigis << std::endl;
    double Emax = 0.;
    int Pmax = 0;
    //double pedestalPreSample = 0.;
    //double pedestalPreSampleAnalog = 0.;
    //int countsAfterGainSwitch = -1;

    for (int sample = 0; sample < nrSamples; ++sample) {
      ebAnalogSignal[sample] = 0.;
      ebADCCounts[sample] = 0.;
      ebADCGains_temp[sample] = 0.;
      ebADCGains[sample] = 0.;
    }

    double gainConv_[4] = {0, 12, 6, 1};
    // saturated channels
    double barrelADCtoGeV_ = 0.048;  //GeV

    // EcalIntercalibConstantsMC* ical =

    // EcalIntercalibConstantMCMap &icalMap =

    for (int sample = 0; sample < nrSamples; ++sample) {
      int thisSample = digi[sample];

      ebADCCounts[sample] = (thisSample & 0xFFF);
      ebADCGains[sample] = (thisSample & (0x3 << 12)) >> 12;
      ebAnalogSignal[sample] = (ebADCCounts[sample] * gainConv_[(int)ebADCGains[sample]] * barrelADCtoGeV_);

      if (ebADCCounts[sample] > maxADCValue)
        maxADCValue = ebADCCounts[sample];

      if (ebid.iphi() == 333 and ebid.ieta() == 83)
        SingleChannelE->SetBinContent(sample, ebADCCounts[sample]);

      if (ebid.iphi() == 334 and ebid.ieta() == 83) {
        SingleChannelELow->SetBinContent(sample, ebADCCounts[sample]);
      }

      if (ebADCGains[sample] == 3)
        LowGain = 1;
      if (ebADCGains[sample] == 2)
        MidGain = 1;
      if (ebADCGains[sample] == 1)
        HighGain = 1;

      if (ebADCCounts[sample] > 250) {
        std::cout << "Channel: " << ebid << std::endl;
        std::cout << "Sample " << sample << std::endl;
        std::cout << "		Full data " << thisSample << std::endl;
        std::cout << "		ebADCCounts " << ebADCCounts[sample] << std::endl;
        std::cout << "		ebADCGains " << ebADCGains[sample] << std::endl;
        std::cout << "		gainConv_ " << gainConv_[(int)ebADCGains[sample]] << std::endl;
        std::cout << "		barrelADCtoGeV_ " << barrelADCtoGeV_ << std::endl;
        std::cout << "		ebAnalogSignal " << ebAnalogSignal[sample] << std::endl;
      }

      if (Emax < ebAnalogSignal[sample]) {
        Emax = ebAnalogSignal[sample];
        Pmax = sample;
      }
    }

    if (1 == 1)
      std::cout << "P max " << Pmax << std::endl;
    std::cout << "High Gain: " << HighGain << " MidGain " << MidGain << " LowGain " << LowGain << std::endl;
    std::cout << "maxADCValue " << maxADCValue << std::endl;
    if (maxADCValue > 250) {
      meEBDigiOccupancy_->SetBinContent(ebid.iphi(), ebid.ieta(), maxADCValue);
      if (HighGain == 1)
        meEBDigiOccupancyHigh_->SetBinContent(ebid.iphi(), ebid.ieta(), maxADCValue);
      if (MidGain == 1)
        meEBDigiOccupancyMid_->SetBinContent(ebid.iphi(), ebid.ieta(), maxADCValue);
      if (LowGain == 1)
        meEBDigiOccupancyLow_->SetBinContent(ebid.iphi(), ebid.ieta(), maxADCValue);
    }

  }  //end digi
}

// ------------ method called once each job just before starting event loop  ------------
void PhaseIAnalyzer::beginJob() {}

// ------------ method called once each job just after ending the event loop  ------------
void PhaseIAnalyzer::endJob() {}

//define this as a plug-in
DEFINE_FWK_MODULE(PhaseIAnalyzer);
