// $Id: TwoBodyCombiner.cc,v 1.12 2006/02/28 11:20:21 llista Exp $
#include "PhysicsTools/CandUtils/interface/TwoBodyCombiner.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
using namespace reco;
using namespace std;

TwoBodyCombiner::TwoBodyCombiner( const boost::shared_ptr<CandSelector> & sel,
				  bool ck, int q ) :
  checkCharge( ck ), charge( 0 ), overlap(), select( sel ) {
  if ( checkCharge ) charge = abs(q);
}
	 
bool TwoBodyCombiner::preselect( const Candidate & c1, const Candidate & c2 ) const {
  if ( checkCharge ) {
    if ( charge != abs(c1.charge() + c2.charge()) ) return false;
  }
  if ( overlap( c1, c2 ) ) return false;
  return true;
}

Candidate * TwoBodyCombiner::combine( const Candidate & c1, const Candidate & c2 ) {
  CompositeCandidate * cmp( new CompositeCandidate );
  cmp->addDaughter( c1 );
  cmp->addDaughter( c2 );
  addp4.set( * cmp );
  return cmp;
}

auto_ptr<CandidateCollection> 
TwoBodyCombiner::combine( const CandidateCollection * src1, const CandidateCollection * src2 ) {
  auto_ptr<CandidateCollection> comps( new CandidateCollection );
  if ( src1 == src2 ) {
    const CandidateCollection & cands = * src1;
    const int n = cands.size();
    for( int i1 = 0; i1 < n; ++ i1 ) {
      const Candidate & c1 = cands[ i1 ];
      for ( int i2 = i1 + 1; i2 < n; ++ i2 ) {
	const Candidate & c2 = cands[ i2 ];
	if ( preselect( c1, c2 ) ) {
	  std::auto_ptr<Candidate> c( combine( c1, c2 ) );
	  if ( (*select)( * c ) )
	    comps->push_back( c.release() );
	}
      }
    }
  } else {
    const CandidateCollection & cands1 = * src1, & cands2 = * src2;
    const int n1 = cands1.size(), n2 = cands2.size();
    for( int i1 = 0; i1 < n1; ++ i1 ) {
      const Candidate & c1 = cands1[ i1 ];
      for ( int i2 = 0; i2 < n2; ++ i2 ) {
	const Candidate & c2 = cands2[ i2 ];
	if ( preselect( c1, c2 ) ) {
	  std::auto_ptr<Candidate> c( combine( c1, c2 ) );
	  if ( (*select)( * c ) )
	    comps->push_back( c.release() );
	}
      }
    }
  }
  return comps;
}
