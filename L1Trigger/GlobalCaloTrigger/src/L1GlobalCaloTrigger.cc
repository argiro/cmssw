#include "L1Trigger/GlobalCaloTrigger/interface/L1GlobalCaloTrigger.h"

#include "L1Trigger/GlobalCaloTrigger/interface/L1GctSourceCard.h"

#include "L1Trigger/GlobalCaloTrigger/interface/L1GctElectronSorter.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctElectronFinalSort.h"

#include "L1Trigger/GlobalCaloTrigger/interface/L1GctJetLeafCard.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctWheelJetFpga.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctJetFinalStage.h"

#include "L1Trigger/GlobalCaloTrigger/interface/L1GctWheelEnergyFpga.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctGlobalEnergyAlgos.h"


L1GlobalCaloTrigger* L1GlobalCaloTrigger::instance = 0;

// constructor
L1GlobalCaloTrigger::L1GlobalCaloTrigger() :
	theJetLeafCards(6),
	theElectronSorters(4),
	theWheelJetFpgas(2),
	theWheelEnergyFpgas(2)
{
  
  build();
  setup();
  
}

L1GlobalCaloTrigger::~L1GlobalCaloTrigger()
{
  theSourceCards.clear();
}

L1GlobalCaloTrigger* L1GlobalCaloTrigger::theGct() {

  if (L1GlobalCaloTrigger::instance==0) {
    L1GlobalCaloTrigger::instance = new L1GlobalCaloTrigger();
  }
  return L1GlobalCaloTrigger::instance;

}

// instantiate hardware/algorithms
void L1GlobalCaloTrigger::build() {

  for (int i=0; i<54; i++) {
    L1GctSourceCard* sc = new L1GctSourceCard();
    theSourceCards.push_back(sc);
  }

  for (int i=0; i<6; i++) {
    theJetLeafCards[i] = new L1GctJetLeafCard();
  }
  
  for (int i=0; i<4; i++) {
    theElectronSorters[i] = new L1GctElectronSorter();
  }		
  
  for (int i=0; i<2; i++) {
    theWheelJetFpgas[i] = new L1GctWheelJetFpga();
    theWheelEnergyFpgas[i] = new L1GctWheelEnergyFpga();
  }
  
  theJetFinalStage = new L1GctJetFinalStage();
  theElectronFinalStage = new L1GctElectronFinalSort();
  theEnergyFinalStage = new L1GctGlobalEnergyAlgos();

}

// wire up the hardware/algos
void L1GlobalCaloTrigger::setup() {

}

void L1GlobalCaloTrigger::process() {
		
  // Source cards
  for (int i=0; i<54; i++) {
    theSourceCards[i]->fetchInput();
  }

  // Jet Leaf cards
  for (int i=0; i<6; i++) {
    theJetLeafCards[i]->fetchInput();
    theJetLeafCards[i]->process();
  }

  // Wheel Cards
  for (int i=0; i<2; i++) {
    theWheelJetFpgas[i]->fetchInput();
    theWheelJetFpgas[i]->process();
  }

  for (int i=0; i<2; i++) {
    theWheelEnergyFpgas[i]->fetchInput();
    theWheelEnergyFpgas[i]->process();
  }

  // Jet Final Stage
  theJetFinalStage->fetchInput();
  theJetFinalStage->process();

  // Jet Final Stage
  theEnergyFinalStage->fetchInput();
  theEnergyFinalStage->process();
  
  // Electron Sorters
  for (int i=0; i<4; i++) {
    theElectronSorters[i]->fetchInput();
    theElectronSorters[i]->process();
  }

  // Electron Final Stage
  theElectronFinalStage->fetchInput();
  theElectronFinalStage->process();

	
}
