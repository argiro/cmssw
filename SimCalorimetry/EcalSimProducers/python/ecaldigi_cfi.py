#raise RuntimeError, "Do not import obsolete file ecalDigi_cfi.py. If you need parameters, use 'from SimGeneral.MixingModule.ecalDigitizer_cfi import *'"
import FWCore.ParameterSet.Config as cms

from SimGeneral.MixingModule.ecalDigitizer_cfi import *

from SimGeneral.MixingModule.ecalDigitizer_Ph2_cfi import *

from Configuration.Eras.Modifier_phase2_common_cff import phase2_common
phase2_common.toReplaceWith(ecalDigitizer,ecalDigitizer_Ph2)

