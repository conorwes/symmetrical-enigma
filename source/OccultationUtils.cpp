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
   /*
   First, let's convert the epoch bounds to doubles representing seconds from
   J2000.
   */
   SpiceDouble lowerEpochTime;
   SpiceDouble upperEpochTime;
   str2et_c( data.LowerBoundEpoch.c_str(), &lowerEpochTime );
   str2et_c( data.UpperBoundEpoch.c_str(), &upperEpochTime );

   /*
   Next, let's configure the cnfine using our bounds.
   */
   SPICEDOUBLE_CELL( cnfine, 200 );
   SPICEDOUBLE_CELL( result, 200 );
   wninsd_c( lowerEpochTime, upperEpochTime, &cnfine );

   /*
   For this program, we're going to use constant step size. This is a tradeoff
   between development complexity and functionality, and the performance
   associated with a constant stepsize seems acceptable.
   */
   gfsstp_c( data.StepSize );

   /*
   Finally, feed our SimulationData into gfocce_c.
   */
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
This is the function which is used to results of the occultation search. The
function accepts a SpiceCell and iterates through the results.
*/
void cppspice::reportSearchSummary( SpiceCell* result ) {
   SpiceInt    i;
   SpiceDouble left;
   SpiceDouble right;
   SpiceChar   beginEpoch[41];
   SpiceChar   endEpoch[41];

   /*
   Check if we've received an interrupt signal.
   */
   if ( gfbail_c() ) {
      /*
      If we've gotten here, we've trapped an interrupt signal and are ready to
      error out.
      */
      gfclrh_c();
      std::cout << "Error: Search was interrupted." << std::endl;
   }
   else {
      /*
      First check if we actually have any results.
      */
      if ( wncard_c( result ) == 0 ) {
         std::cout
            << "No occultations were found within the specified time window."
            << std::endl;
      }
      else {
         /*
         Now we'll iterate through any/all results and report the information
         in a user-friendly format.
         */
         for ( i = 0; i < wncard_c( result ); i++ ) {
            /*
            First we'll need to fetch the interval so we can translate it into
            a legible format.
            */
            wnfetd_c( result, i, &left, &right );

            /*
            Take the lower bound and translate it into our common calendar
            format.
            */
            timout_c(
               left,
               "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)",
               41,
               beginEpoch );
            /*
            Now do the same with the upper bound.
            */
            timout_c(
               right,
               "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)",
               41,
               endEpoch );

            /*
            Finally report the interval information to console.
            */
            std::cout << "Interval " << i << std::endl;
            std::cout << "   Start time: " << beginEpoch << std::endl;
            std::cout << "   Stop time:  " << endEpoch << std::endl;
         }
      }
   }
}
// clang-format off
/*

- Brief I/O

   Variable  I/O  DESCRIPTION
   --------  ---  --------------------------------------------------
   result     I   The results which have been output by gfocce_c.

- Detailed_Input

   result   a SpiceCell which contains the results of the occultation
            analysis. This will be parsed and the results will be iterated
            through so that the results can be reported.

- Detailed_Output

   The function returns void.

- Error Handling

   Any errors encountered in the CSPICE routines will be handled by CSPICE's
   native error handling. Otherwise, error messages are reported, and the
   function returns.

- Particulars

   None.

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
/* End OccultationUtils.cpp */