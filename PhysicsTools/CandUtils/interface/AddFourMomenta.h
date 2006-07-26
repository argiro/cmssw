#ifndef CandUtils_AddFourMomenta_h
#define CandUtils_AddFourMomenta_h
/** \class AddFourMomenta
 *
 * set up a composite reco::Candidate adding its 
 * daughters four-momenta and electric charge
 *
 * \author Luca Lista, INFN
 *
 * \version $Revision: 1.6 $
 *
 * $Id: AddFourMomenta.h,v 1.6 2006/06/21 09:36:47 llista Exp $
 *
 */
#include "DataFormats/Candidate/interface/CandidateFwd.h"

struct AddFourMomenta {
  /// set up a candidate
  void set( reco::Candidate& c );
};

#endif
