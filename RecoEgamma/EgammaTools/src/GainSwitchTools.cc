#include "RecoEgamma/EgammaTools/interface/GainSwitchTools.h"


#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "RecoCaloTools/Navigation/interface/CaloNavigator.h"
#include "DataFormats/DetId/interface/DetId.h"
const std::vector<int> GainSwitchTools::gainSwitchFlags_={EcalRecHit::kHasSwitchToGain6,EcalRecHit::kHasSwitchToGain1};

 //this should really live in EcalClusterTools
int GainSwitchTools::nrCrysWithFlagsIn5x5(const DetId& id,const std::vector<int>& flags,const EcalRecHitCollection* recHits,const CaloTopology *topology)
{
  int nrFound=0;
  CaloNavigator<DetId> cursor = CaloNavigator<DetId>( id, topology->getSubdetectorTopology( id ) );
  
  for ( int eastNr = -2; eastNr <= 2; ++eastNr ) { //east is eta in barrel
    for ( int northNr = -2; northNr <= 2; ++northNr ) { //north is phi in barrel
      cursor.home();
      cursor.offsetBy( eastNr, northNr);
      DetId id = *cursor;
      auto recHitIt = recHits->find(id);
      if(recHitIt!=recHits->end() && 
	 recHitIt->checkFlags(flags)){
	nrFound++;
      }
    }
  }
  return nrFound;
}

bool GainSwitchTools::hasEBGainSwitch(const reco::SuperCluster& superClus,const EcalRecHitCollection* recHits)
{
  if(!recHits || superClus.seed()->seed().subdetId()!=EcalBarrel) return false;
  for(const auto & clus : superClus.clusters()){
    for(const auto& hit : clus->hitsAndFractions()){
      auto recHitIt = recHits->find(hit.first);
      if(recHitIt!=recHits->end() && 
	 recHitIt->checkFlags(gainSwitchFlags())){
	return true;
      }
    }
  }
  return false;	
}

bool GainSwitchTools::hasEBGainSwitchIn5x5(const reco::SuperCluster& superClus,const EcalRecHitCollection* recHits,const CaloTopology *topology)
{
  if(recHits || superClus.seed()->seed().subdetId()!=EcalBarrel) return nrCrysWithFlagsIn5x5(superClus.seed()->seed(),gainSwitchFlags(),recHits,topology)!=0;
  else return false;
}

bool GainSwitchTools::hasEBGainSwitch(const EcalRecHitCollection* recHits)
{
  if(!recHits) return false;
  for(auto hit : *recHits){
    if(hit.id().subdetId()==EcalBarrel && hit.checkFlags(gainSwitchFlags())) return true;
  }
  return false;
}
  
//needs to be multifit rec-hits currently as weights dont have gs flags set
std::vector<DetId> GainSwitchTools::gainSwitchedIdsIn5x5(const DetId& id,const EcalRecHitCollection* recHits,const CaloTopology* topology)
{
  std::vector<DetId> gsDetIds;
  CaloNavigator<DetId> cursor = CaloNavigator<DetId>( id, topology->getSubdetectorTopology( id ) );
  
  for ( int eastNr = -2; eastNr <= 2; ++eastNr ) { //east is eta in barrel
    for ( int northNr = -2; northNr <= 2; ++northNr ) { //north is phi in barrel
      cursor.home();
      cursor.offsetBy( eastNr, northNr);
      DetId id = *cursor;
      auto recHitIt = recHits->find(id);
      if(recHitIt!=recHits->end() && 
	 recHitIt->checkFlags(gainSwitchFlags())){
	gsDetIds.push_back(id);
      }
    }
  }
  return gsDetIds;
}

//it should be a reasonable assumption that the GS hits should be in the supercluster with fraction ~1 for 
//isolated electrons/photons
float GainSwitchTools::newRawEnergyNoFracs(const reco::SuperCluster& superClus,const std::vector<DetId> gainSwitchedHitIds,const EcalRecHitCollection* oldRecHits,const EcalRecHitCollection* newRecHits)
{
  double oldEnergy=0.;
  double newEnergy=0.;
  for(auto& id : gainSwitchedHitIds){
    auto oldRecHitIt = oldRecHits->find(id);
    if(oldRecHitIt!=oldRecHits->end()) oldEnergy+=oldRecHitIt->energy();
    auto newRecHitIt = newRecHits->find(id);
    if(newRecHitIt!=newRecHits->end()) newEnergy+=newRecHitIt->energy();
  }
  
  float newRawEnergy = superClus.rawEnergy() - oldEnergy + newEnergy;
  
  return newRawEnergy;
}



reco::SuperClusterRef GainSwitchTools::matchSCBySeedCrys(const reco::SuperCluster& sc,edm::Handle<reco::SuperClusterCollection> scColl )
{
  for(size_t scNr=0;scNr<scColl->size();scNr++){
    reco::SuperClusterRef scRef(scColl,scNr);
    if(scRef->seed()->seed().rawId() ==sc.seed()->seed().rawId()) return scRef;
  }
  return reco::SuperClusterRef(nullptr,0);
}

reco::SuperClusterRef GainSwitchTools::matchSCBySeedCrys(const reco::SuperCluster& sc,edm::Handle<reco::SuperClusterCollection> scColl,int maxDEta,int maxDPhi)
{
  reco::SuperClusterRef bestRef(scColl.id());

  int bestDIR2 = maxDEta*maxDEta+maxDPhi*maxDPhi+1; //+1 is to make it slightly bigger than max allowed
  
  if(sc.seed()->seed().subdetId()==EcalBarrel){
    EBDetId scDetId(sc.seed()->seed());
    
    for(size_t scNr=0;scNr<scColl->size();scNr++){
      reco::SuperClusterRef matchRef(scColl,scNr);
      if(matchRef->seed()->seed().subdetId()==EcalBarrel){
	EBDetId matchDetId(matchRef->seed()->seed());
	int dIEta = calDIEta(scDetId.ieta(),matchDetId.ieta());
	int dIPhi = calDIPhi(scDetId.iphi(),matchDetId.iphi());
	int dIR2 = dIEta*dIEta+dIPhi*dIPhi;
	if(dIR2<bestDIR2){
	  bestDIR2=dIR2;
	  bestRef = reco::SuperClusterRef(scColl,scNr);
	}
      }
    }
    
    
  }
  return bestRef;
}


int GainSwitchTools::calDIEta(int lhs,int rhs)
{
  int retVal = lhs - rhs;
  if(lhs*rhs<0){ //crossing zero
    if(retVal<0) retVal++;
    else retVal--;
  }
  return retVal;
}

int GainSwitchTools::calDIPhi(int lhs,int rhs)
{
  int retVal = lhs-rhs;
  while(retVal>180) retVal-=360;
  while(retVal<-180) retVal+=360;
  return retVal;
}
