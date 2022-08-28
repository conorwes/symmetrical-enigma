// clang-format off
/*

- Source_File OccultationUtils.cpp (Occultation utility code)

- Abstract

   Implement the functions defined in the corresponding header file.

- Disclaimer

   This code was created by me and is provided as-is.

- Required_Reading

   FRAMES
   GF
   KERNEL
   NAIF_IDS
   SPK
   TIME
   WINDOWS

- Particulars

   This file contains the implementations of the functions defined in the
   corresponding header file (OccultationUtils.hpp). These functions support
   occultation analysis which is performed by this program.

- Literature_References

   None.

- Author

   C.P. Westphal     (self)

- Credits

   This file references the CSPICE API, which was developed by the NAIF at
   JPL.

- Restrictions

   None.

- Version

   -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
We only need to include the corresponding header file.
*/
#include "OccultationUtils.hpp"

/*
This is the function which is used to perform the occultation search. We
feed in the SimulationData which was retrieved prior to this call.
*/
SpiceCell* cppspice::performOccultationSearch( const SimulationData& data ) {

   // TODO - clean this up
   SpiceDouble lowerEpochTime;
   SpiceDouble upperEpochTime;
   str2et_c( data.LowerBoundEpoch.c_str(), &lowerEpochTime );
   str2et_c( data.UpperBoundEpoch.c_str(), &upperEpochTime );

   SPICEDOUBLE_CELL( cnfine, 200 );
   SPICEDOUBLE_CELL( result, 200 );
   wninsd_c( lowerEpochTime, upperEpochTime, &cnfine );

   gfsstp_c( data.StepSize );

   gfocce_c(
      data.OccultationType.c_str(),
      std::get<0>( data.OcculterDetails ).c_str(),
      std::get<1>( data.OcculterDetails ).c_str(),
      std::get<2>( data.OcculterDetails ).c_str(),
      std::get<0>( data.TargetDetails ).c_str(),
      std::get<1>( data.TargetDetails ).c_str(),
      std::get<2>( data.TargetDetails ).c_str(),
      "LT",
      data.ObserverName.c_str(),
      data.Tolerance,
      gfstep_c,
      gfrefn_c,
      true,
      gfrepi_c,
      gfrepu_c,
      gfrepf_c,
      true,
      gfbail_c,
      &cnfine,
      &result );

   return &result;
}

// clang-format off
/*

- Brief I/O

   Variable  I/O  DESCRIPTION
   --------  ---  --------------------------------------------------
   data       I   The simulation data which is fed into gfocce_c.

- Detailed_Input

   data     a struct which contains the simulation data used in the
            occultation analysis. The struct members include:

               LowerBoundEpoch:  The epoch in TDB which begins the range.
               UpperBoundEpoch   The epoch in TDB which ends the range.
               StepSize          The step size in seconds.
               OccultationType   The type of the occultation. The supported
                                 values are outlined in gfocce_c.c
               OcculterDetails   A tuple containing the occulting object's
                                 name, shape, and reference frame.
               TargetDetails     A tuple containing the occulting object's
                                 name, shape, and reference frame.
               ObserverName      The name of the observing object.
               Tolerance         The tolerance in seconds.

- Detailed_Output

   The function returns a SPICE window representing the set of time intervals,
   within the confinement period, when the specified occultation occurs.

   The endpoints of the time intervals comprising 'result' are interpreted as
   seconds past J2000 TDB.

- Error Handling

   This function's error handling is performed by the CSPICE API.

- Particulars

   For more information, please see the CSPICE documentation for gfocce_c.

- Literature_References

   CSPICE's documentation.

- Author

   C.P. Westphal     (self)

- Author

   C.P. Westphal     (self)

- Credits

   This file references the CSPICE API, which was developed by the NAIF at
   JPL.

- Restrictions

   None.

- Version

   -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

void cppspice::reportSearchSummary( SpiceCell* result ) {
   SpiceInt    i;
   SpiceDouble left;
   SpiceDouble right;
   SpiceChar   beginEpoch[41];
   SpiceChar   endEpoch[41];

   if ( gfbail_c() ) {
      /*
      Clear the CSPICE interrupt indication. This is
      an essential step for programs that continue
      running after an interrupt; gfbail_c will
      continue to return SPICETRUE until this step
      has been performed.
      */
      gfclrh_c();

      /*
      We've trapped an interrupt signal. In a realistic
      application, the program would continue operation
      from this point. In this simple example, we simply
      display a message and quit.
      */
      std::cout << "Error: Search was interrupted." << std::endl;
   }
   else {

      if ( wncard_c( result ) == 0 ) {
         printf( "No occultation was found.\n" );
      }
      else {
         for ( i = 0; i < wncard_c( result ); i++ ) {
            /*
            fetch and display each occultation interval.
            */
            wnfetd_c( result, i, &left, &right );

            timout_c(
               left,
               "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)",
               41,
               beginEpoch );
            timout_c(
               right,
               "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)",
               41,
               endEpoch );

            std::cout << "Interval " << i << std::endl;
            std::cout << "   Start time: " << beginEpoch << std::endl;
            std::cout << "   Stop time:  " << endEpoch << std::endl;
         }
      }
   }
}