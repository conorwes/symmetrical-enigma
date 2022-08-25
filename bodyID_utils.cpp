#include "bodyID_utils.hpp"
#include <iostream>

extern "C" {
#include "SpiceZpl.h"
#include "SpiceZdf.h"
#include "SpiceErr.h"
#include "SpiceEK.h"
#include "SpiceFrm.h"
#include "SpiceCel.h"
#include "SpiceCK.h"
#include "SpiceSPK.h"
#include "SpiceGF.h"
#include "SpiceOccult.h"
#include "SpiceZpr.h"
#include "SpiceZim.h"
}

int CPPSpice::GetNAIFIDfromName(const std::string& name) {
   SpiceInt     code(0);
   SpiceBoolean found(false);
   bodn2c_c(name.c_str(), &code, &found);

   if (found) {
      return code;
   }
   else {
      std::cout << "Error: couldn't find an NAIF ID for the specified object '" << name
                << "'." << std::endl;
      return -1;
   }
}