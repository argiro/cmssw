#include "SimCalorimetry/EcalSimAlgos/interface/EcalCorrelatedNoiseMatrix_Ph2.h"
#include "SimGeneral/NoiseGenerators/interface/CorrelatedNoisifier.icc"

template class CorrelatedNoisifier<EcalCorrMatrix_Ph2>;

template void CorrelatedNoisifier<EcalCorrMatrix_Ph2>::noisify(CaloSamples&,
                                                               CLHEP::HepRandomEngine*,
                                                               const std::vector<double>* rangau) const;
