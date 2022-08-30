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
We need the corresponding header and the fstream header.
*/
#include <fstream>

#include "OccultationUtils.hpp"

/*
A function which determines whether the target is occulted at a specified
epoch.
*/
bool cppspice::isOccultedAtEpoch(
   const SpiceInt    targetID,
   const SpiceInt    occulterID,
   const SpiceInt    observerID,
   const SpiceDouble epoch,
   const SpiceChar*  occulterFrame,
   const SpiceChar*  occulterName,
   const SpiceChar*  targetFrame,
   const SpiceChar*  targetName,
   SpiceBoolean&     isOcculted ) {
   SpiceDouble earthToObserverJ2000[6];
   SpiceDouble lt{ 0.0 };
   spkez_c(
      observerID,
      epoch,
      "j2000",
      "LT",
      399,
      earthToObserverJ2000,
      &lt );

   SpiceDouble earthToOcculterJ2000[6];
   spkez_c(
      occulterID,
      epoch,
      "j2000",
      "LT",
      399,
      earthToOcculterJ2000,
      &lt );

   SpiceDouble occulterToObserverJ2000[3];
   occulterToObserverJ2000[0] =
      ( earthToOcculterJ2000[0] * -1.0 ) + earthToObserverJ2000[0];
   occulterToObserverJ2000[1] =
      ( earthToOcculterJ2000[1] * -1.0 ) + earthToObserverJ2000[1];
   occulterToObserverJ2000[2] =
      ( earthToOcculterJ2000[2] * -1.0 ) + earthToObserverJ2000[2];

   SpiceDouble earthToTargetJ2000[6];
   spkez_c( targetID, epoch, "j2000", "LT", 399, earthToTargetJ2000, &lt );

   SpiceDouble occulterToTargetJ2000[3];
   occulterToTargetJ2000[0] =
      ( earthToOcculterJ2000[0] * -1.0 ) + earthToTargetJ2000[0];
   occulterToTargetJ2000[1] =
      ( earthToOcculterJ2000[1] * -1.0 ) + earthToTargetJ2000[1];
   occulterToTargetJ2000[2] =
      ( earthToOcculterJ2000[2] * -1.0 ) + earthToTargetJ2000[2];

   SpiceDouble rotate[3][3];
   pxform_c( "j2000", occulterFrame, epoch, rotate );

   SpiceDouble occulterToObserverFixed[3];
   mxv_c( rotate, occulterToObserverJ2000, occulterToObserverFixed );

   SpiceDouble occulterToTargetFixed[3];
   mxv_c( rotate, occulterToTargetJ2000, occulterToTargetFixed );

   SpiceInt    n;
   SpiceDouble radii[3];
   bodvrd_c( occulterName, "RADII", 3, &n, radii );
   SpiceDouble occulterRadiusEq = radii[0];

   SpiceDouble scaleFactor = radii[0] / radii[2];

   occulterToTargetFixed[2] *= scaleFactor;
   occulterToObserverFixed[2] *= scaleFactor;

   bodvrd_c( targetName, "RADII", 3, &n, radii );

   radii[2] *= ( radii[0] / radii[2] );

   radii[0] *= scaleFactor;
   radii[1] *= scaleFactor;
   radii[2] *= scaleFactor;

   SpiceDouble observerToOcculterFixed[3];
   observerToOcculterFixed[0] = occulterToObserverFixed[0] * -1.0;
   observerToOcculterFixed[1] = occulterToObserverFixed[1] * -1.0;
   observerToOcculterFixed[2] = occulterToObserverFixed[2] * -1.0;

   SpiceDouble observerToTargetFixed[3];
   observerToTargetFixed[0] =
      occulterToTargetFixed[0] + observerToOcculterFixed[0];
   observerToTargetFixed[1] =
      occulterToTargetFixed[1] + observerToOcculterFixed[1];
   observerToTargetFixed[2] =
      occulterToTargetFixed[2] + observerToOcculterFixed[2];

   SpiceDouble distance = calculateMagnitude( observerToTargetFixed );

   if ( distance < radii[0] ) {
      std::cout << "Error: body is within the target's radius." << std::endl;
      return false;
   }

   SpiceDouble halfAngle = asin( radii[0] / distance );

   if (
      calculateMagnitude( observerToOcculterFixed ) >
      calculateMagnitude( observerToTargetFixed ) )
   {
      isOcculted = false;
      return true;
   }

   auto        v1                          = observerToTargetFixed;
   auto        v2                          = observerToOcculterFixed;
   SpiceDouble v1Length                    = calculateMagnitude( v1 );
   SpiceDouble v2Length                    = calculateMagnitude( v2 );
   SpiceDouble targetOcculterObserverAngle = acos(
      ( v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] ) /
      ( v1Length * v2Length ) );

   SpiceDouble occulterRadius = calculateMagnitude( occulterToObserverFixed );
   SpiceDouble bodyHalfAngle{ 0.0 };
   if ( occulterRadius < occulterRadiusEq ) {
      bodyHalfAngle = PI / 2;
   }
   else {
      bodyHalfAngle = asin( occulterRadiusEq / occulterRadius );
   }

   isOcculted = targetOcculterObserverAngle < halfAngle + bodyHalfAngle;

   return true;
}

/*
This is a function which is used to perform the occultation search using
a custom written algorithm.
*/
bool cppspice::performOccultationSearch_native( const SimulationData& data ) {
   /*
   First, let's convert the epoch bounds to doubles representing seconds from
   J2000.
   */
   SpiceDouble lowerEpochTime{ 0.0 };
   SpiceDouble upperEpochTime{ 0.0 };
   str2et_c( data.LowerBoundEpoch.c_str(), &lowerEpochTime );
   str2et_c( data.UpperBoundEpoch.c_str(), &upperEpochTime );

   /*
   Retrieve the NAIF IDs of the parties involved.
   */
   SpiceInt     targetID{ 0 };
   SpiceBoolean found{ false };
   bodn2c_c( std::get<0>( data.TargetDetails ).c_str(), &targetID, &found );
   SpiceInt occulterID{ 0 };
   bodn2c_c(
      std::get<0>( data.OcculterDetails ).c_str(),
      &occulterID,
      &found );
   SpiceInt observerID{ 0 };
   bodn2c_c( data.ObserverName.c_str(), &observerID, &found );
   SpiceDouble epoch{ 0.0 };

   SpiceBoolean isOcculted{ false };

   // TODO - Implement root finding algorithm
   // Here we need to find all of the instances where occultation state
   // changes. To do so, we will break the span down into chunks corresponding
   // to the step size, and then perform bisection to identify state changes.
   // In each interval, we search by narrowing the interval (by adding the
   // stepsize to the starting epoch until we either find the transition or we
   // reach the maximum iterations)
   int intervalCount =
      std::floor( ( upperEpochTime - lowerEpochTime ) / data.StepSize ) + 1;
   std::vector<double> epochTimes;
   epochTimes.reserve( intervalCount );
   for ( int i = 0; i < intervalCount; i++ ) {
      epochTimes.push_back( lowerEpochTime + ( i * data.StepSize ) );
   }
   epochTimes.push_back( upperEpochTime );

   std::vector<bool> occultationVector;
   occultationVector.reserve( epochTimes.size() );

   for ( auto& et : epochTimes ) {
      isOccultedAtEpoch(
         targetID,
         occulterID,
         observerID,
         et,
         std::get<2>( data.OcculterDetails ).c_str(),
         std::get<0>( data.OcculterDetails ).c_str(),
         std::get<2>( data.TargetDetails ).c_str(),
         std::get<0>( data.TargetDetails ).c_str(),
         isOcculted );
      occultationVector.push_back( isOcculted );
   }

   return true;
}

/*
This is the function which is used to perform the occultation search using
the cspice gfoclt_c routine. We feed in the SimulationData which was
retrieved prior to this call.
*/
SpiceCell*
cppspice::performOccultationSearch_cspice( const SimulationData& data ) {
   /*
   First, let's convert the epoch bounds to doubles representing seconds
   from J2000.
   */
   SpiceDouble lowerEpochTime{ 0.0 };
   SpiceDouble upperEpochTime{ 0.0 };
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
   Finally, feed our SimulationData into gfoclt_c.
   */
   gfoclt_c(
      data.OccultationType.c_str(),
      std::get<0>( data.OcculterDetails ).c_str(),
      std::get<1>( data.OcculterDetails ).c_str(),
      std::get<2>( data.OcculterDetails ).c_str(),
      std::get<0>( data.TargetDetails ).c_str(),
      std::get<1>( data.TargetDetails ).c_str(),
      std::get<2>( data.TargetDetails ).c_str(),
      "LT",
      data.ObserverName.c_str(),
      data.StepSize,
      &cnfine,
      &result );

   return &result;
}
// clang-format off
/*

- Brief I/O

   Variable  I/O  DESCRIPTION
   --------  ---  --------------------------------------------------
   data       I   The simulation data which is fed into gfoclt_c.

- Detailed_Input

   data     a struct which contains the simulation data used in the
            occultation analysis. The struct members include:

               LowerBoundEpoch:  The epoch in TDB which begins the range.
               UpperBoundEpoch   The epoch in TDB which ends the range.
               StepSize          The step size in seconds.
               OccultationType   The type of the occultation. The supported
                                 values are outlined in gfoclt_c.c
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

   For more information, please see the CSPICE documentation for gfoclt_c.

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
   SpiceInt    i{ 0 };
   SpiceDouble left{ 0.0 };
   SpiceDouble right{ 0.0 };
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

         std::ofstream out( "output.txt" );
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

            /*
            Also report to file.
            */
            out << "Interval " << i << std::endl;
            out << "   Start time: " << beginEpoch << std::endl;
            out << "   Stop time: " << endEpoch << std::endl;
         }

         out.close();
      }
   }
}
// clang-format off
/*

- Brief I/O

   Variable  I/O  DESCRIPTION
   --------  ---  --------------------------------------------------
   result     I   The results which have been output by gfoclt_c.

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