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
   /*

   - Brief I/O

      Variable     I/O  DESCRIPTION
      --------     ---  --------------------------------------------------
      SpiceInt      I   The NAIF ID of the target.
      SpiceInt      I   The NAIF ID of the occulter.
      SpiceInt      I   The NAIF ID of the observer.
      SpiceDouble   I   The epoch being evaluated.
      SpiceChar*    I   The name of the occulter's frame.
      SpiceChar*    I   The name of the occulter.
      SpiceChar*    I   The name of the target's frame.
      SpiceChar*    I   The name of the target.
      SpiceBoolean  I   Whether an occultation is happening.

   - Detailed_Input

      targetID      an int representing the NAIF ID of the target object.
      occulterID    an int representing the NAIF ID of the occulter object.
      observerID    an int representing the NAIF ID of the observer object.
      epoch         a double representing the epoch being evaluated.
      occulterFrame the name of the occulter's frame.
      occulterName  the name of the occulter.
      targetFrame   the name of the target's frame.
      targetName    the name of the target.
      isOcculted    a boolean representing whether an occultation is
   happening.

   - Detailed_Output

      The function returns true if no errors are encountered.

   - Error Handling

      CSPICE components are handled using the native error handling.
   Otherwise, errors are reported and false is returned.

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

      Symmetrical-Enigma Version 1.X.X, 03-SEP-2022 (CPW)
      Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)
   */

   /*
   First, we need to get the J2000 observer position.
   */
   SpiceDouble earthToObserverJ2000[6];
   SpiceDouble lt{ 0.0 };
   spkez_c(
      observerID,
      epoch,
      "j2000",
      "LT",
      EARTHID,
      earthToObserverJ2000,
      &lt );

   /*
   Similarly, get the J2000 occulter position.
   */
   SpiceDouble earthToOcculterJ2000[6];
   spkez_c(
      occulterID,
      epoch,
      "j2000",
      "LT",
      EARTHID,
      earthToOcculterJ2000,
      &lt );

   /*
   We can now calculate the J2000 occulter-to-observer vector.
   */
   SpiceDouble occulterToObserverJ2000[3];
   vlcom_c(
      -1.0,
      earthToOcculterJ2000,
      1.0,
      earthToObserverJ2000,
      occulterToObserverJ2000 );

   /*
   Next, let's get the J2000 target position.
   */
   SpiceDouble earthToTargetJ2000[6];
   spkez_c(
      targetID,
      epoch,
      "j2000",
      "LT",
      EARTHID,
      earthToTargetJ2000,
      &lt );

   /*
   We can now calculate the J2000 occulter-to-target vector.
   */
   SpiceDouble occulterToTargetJ2000[3];
   vlcom_c(
      -1.0,
      earthToOcculterJ2000,
      1.0,
      earthToTargetJ2000,
      occulterToTargetJ2000 );

   /*
   Going forward, we'll want to evaluate everything in the occulter-fixed
   frame, so get the rotation matrix.
   */
   SpiceDouble rotate[3][3];
   pxform_c( "j2000", occulterFrame, epoch, rotate );

   /*
   Translate the occulter-to-observer vector to occulter-fixed.
   */
   SpiceDouble occulterToObserverFixed[3];
   mxv_c( rotate, occulterToObserverJ2000, occulterToObserverFixed );

   /*
   Translate the occulter-to-target vector to occulter-fixed.
   */
   SpiceDouble occulterToTargetFixed[3];
   mxv_c( rotate, occulterToTargetJ2000, occulterToTargetFixed );

   /*
   We want to spherize the occulter to account for flattening. So, get the
   radii from the kernel we've already furnished.
   */
   SpiceInt    n;
   SpiceDouble occulterRadii[3];
   bodvrd_c( occulterName, "RADII", 3, &n, occulterRadii );

   /*
   The equatorial radius will be used elsewhere, so save that off.
   */
   SpiceDouble occulterRadiusEq = occulterRadii[0];

   /*
   Now we can scale the relevant vectors.
   */
   SpiceDouble scaleFactor = occulterRadii[0] / occulterRadii[2];
   occulterToTargetFixed[2] *= scaleFactor;
   occulterToObserverFixed[2] *= scaleFactor;

   /*
   In addition to scaling the relevant vectors, we also need to scale the
   target.
   */
   SpiceDouble targetRadii[3];
   bodvrd_c( targetName, "RADII", 3, &n, targetRadii );
   vscl_c( scaleFactor, targetRadii, targetRadii );

   /*
   Later in our algorithm, we'll need to have the observer-to-occulter and
   observer-to-target vectors. Fortunately, we already have what we need, just
   need to reverse the direction.
   */
   SpiceDouble observerToOcculterFixed[3];
   vscl_c( -1.0, occulterToObserverFixed, observerToOcculterFixed );

   SpiceDouble observerToTargetFixed[3];
   vadd_c(
      occulterToTargetFixed,
      observerToOcculterFixed,
      observerToTargetFixed );

   /*
   Perform a quick check to ensure that the observer is not within the
   target's radius.
   */
   SpiceDouble distance = vnorm_c( observerToTargetFixed );
   if ( distance < targetRadii[0] ) {
      std::cout << "Error: observer is within the target's radius."
                << std::endl;
      return false;
   }

   /*
   Perform another quick check - if the observer-to-occulter is larger than
   the observer-to-target, the occulter is on the far side of the target, and
   thus can't be occulted.
   */
   if ( vnorm_c( observerToOcculterFixed ) > distance ) {
      isOcculted = false;
      return true;
   }

   /*
   Get the half angle between the observer-to-target vector and the target's
   radius.
   */
   SpiceDouble halfAngle = asin( targetRadii[0] / distance );

   /*
   Now we can calculate the occulter half angle/body width.
   */
   SpiceDouble occulterRadius = vnorm_c( occulterToObserverFixed );
   SpiceDouble bodyHalfAngle{ 0.0 };

   /*
   If the radius is less than the body radius, we've probably just got numeric
   noise -> asin(1) -> pi/2. So let's get that specifically.
   */
   if ( occulterRadius < occulterRadiusEq ) {
      bodyHalfAngle = PI / 2;
   }
   else {
      /*
      Otherwise, the body half angle is the asin of the ratio between our
      known equatorial radius, and the computed one.
      */
      bodyHalfAngle = asin( occulterRadiusEq / occulterRadius );
   }

   /*
   Almost there...now, get the target-occulter-observer angle.
   */
   SpiceDouble targetOcculterObserverAngle =
      vsep_c( observerToTargetFixed, observerToOcculterFixed );

   /*
   Finally! If the target-occulter-observer angle is smaller than the sum of
   the target half-angle and the occulter half angle, we're occulted!
   */
   isOcculted = targetOcculterObserverAngle < halfAngle + bodyHalfAngle;

   return true;
}

/*
A bisection algorithm to find the transition.
*/
bool cppspice::bisectEpochs(
   const SpiceInt     targetID,
   const SpiceInt     occulterID,
   const SpiceInt     observerID,
   const SpiceDouble  lowerEpoch,
   const SpiceBoolean lowerOcculted,
   const SpiceDouble  upperEpoch,
   const SpiceBoolean upperOcculted,
   const SpiceChar*   occulterFrame,
   const SpiceChar*   occulterName,
   const SpiceChar*   targetFrame,
   const SpiceChar*   targetName,
   const SpiceDouble  tolerance ) {
   /*
   - Brief I/O

   Variable     I/O  DESCRIPTION
   --------     ---  --------------------------------------------------
   SpiceInt      I   The NAIF ID of the target.
   SpiceInt      I   The NAIF ID of the occulter.
   SpiceInt      I   The NAIF ID of the observer.
   SpiceDouble   I   The left epoch of the window being evaluated.
   SpiceBoolean  I   The occultation state at the left epoch of the window.
   SpiceDouble   I   The right epoch of the window being evaluated.
   SpiceBoolean  I   The occultation state at the right epoch of the window.
   SpiceChar*    I   The name of the occulter's frame.
   SpiceChar*    I   The name of the occulter.
   SpiceChar*    I   The name of the target's frame.
   SpiceChar*    I   The name of the target.
   SpiceDouble   I   The tolerance, in seconds, used in the bisection
   algorithm.

   - Detailed_Input

   targetID      an int representing the NAIF ID of the target object.
   occulterID    an int representing the NAIF ID of the occulter object.
   observerID    an int representing the NAIF ID of the observer object.
   lowerEpoch    a double representing the left epoch of the evaluation
   window. lowerOcculted a bool representing the occultation status of the
   left epoch of the evaluation window. upperEpoch    a double representing
   the right epoch of the evaluation window. upperOcculted a bool representing
   the occultation status of the right epoch of the evaluation window.
   occulterFrame the name of the occulter's frame. occulterName  the name of
   the occulter. targetFrame the name of the target's frame. targetName    the
   name of the target. tolerance the tolerance in seconds used in the
   bisection algorithm.

   - Detailed_Output

   The function returns true if no errors are encountered.

   - Error Handling

   CSPICE components are handled using the native error handling. Otherwise,
   errors are reported and false is returned.

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
      Symmetrical-Enigma Version 1.X.X, 03-SEP-2022 (CPW)
      Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)
   */

   /*
   Since we're going to do a lot of iteration, define our workers here.
   */
   SpiceDouble  left{ lowerEpoch };
   SpiceDouble  right{ upperEpoch };
   SpiceDouble  workingEpoch{ 0.0 };
   SpiceInt     numIterations{ 0 };
   SpiceBoolean leftOcculted{ lowerOcculted };
   SpiceBoolean rightOcculted{ upperOcculted };
   SpiceBoolean workingOcculted{ false };
   SpiceDouble  step{ STEPSIZE };
   SpiceBoolean midpointOcculted{ false };

   /*
   Perform a bisection algorithm. Our algorithm is pretty simple: while the
   difference between the two bounds are greater than the tolerance and the
   number of iterations is below our maximum, we step from the lower bound
   until we find the transition. At that point, we set the upper bound as that
   transition epoch, reduce our stepsize, and resume narrowing the bounds.
   */
   while ( left < right && ( abs( right - left ) > tolerance ) &&
           numIterations < ITERLIMIT )
   {
      /*
      Increase the iteration count.
      */
      numIterations++;

      /*
      First check the midpoint to narrow down the search a bit.
      */
      SpiceDouble midpoint = ( left + right ) / 2;
      isOccultedAtEpoch(
         targetID,
         occulterID,
         observerID,
         midpoint,
         occulterFrame,
         occulterName,
         targetFrame,
         targetName,
         midpointOcculted );

      if ( midpointOcculted == leftOcculted ) {
         left = midpoint;
      }
      else if ( midpointOcculted == rightOcculted ) {
         right = midpoint;
      }

      /*
      Take one step, and then evaluate the occultation.
      */
      workingEpoch = left + step;
      isOccultedAtEpoch(
         targetID,
         occulterID,
         observerID,
         workingEpoch,
         occulterFrame,
         occulterName,
         targetFrame,
         targetName,
         workingOcculted );

      /*
      If we have a state change, we've stepped beyond the transition, so set
      the right bound as the working epoch, and then halve the step size.
      */
      if ( workingOcculted != leftOcculted ) {
         right         = workingEpoch;
         rightOcculted = workingOcculted;
         step /= 2;
      }
      else {
         /*
         Otherwise, keep on chugging.
         */
         left         = workingEpoch;
         leftOcculted = workingOcculted;
      }

      /*
      If we've gotten below the tolerance, we've found our transition. Report
      the midway point between the two bounds
      */
      if ( abs( right - left ) < tolerance ) {

         std::cout
            << ( !leftOcculted && rightOcculted ? "Occultation started: "
                                                : "Occultation ended: " );
         SpiceChar timeOut[TIMELEN];
         timout_c( ( left + right ) / 2, TIMEFORMAT, TIMELEN, timeOut );
         std::cout << timeOut << std::endl;
      }
      else if ( numIterations >= ITERLIMIT ) {
         /*
         If we exceed the iteration count, something has gone wrong, so error
         out.
         */
         SpiceChar lowerText[TIMELEN];
         SpiceChar upperText[TIMELEN];
         timout_c( left, TIMEFORMAT, TIMELEN, lowerText );
         timout_c( right, TIMEFORMAT, TIMELEN, upperText );
         std::cout << "Error: unable to find the transition between '"
                   << lowerText << "' and '" << upperText << "'."
                   << std::endl;
         return false;
      }
   }

   return true;
}

/*
This is a function which is used to perform the occultation search using
a custom written algorithm.
*/
bool cppspice::performCustOccSrch( const SimulationData& data ) {
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

      The function returns true if no errors are encountered.

   - Error Handling

      CSPICE components are handled using the native error handling.
   Otherwise, errors are reported and false is returned.

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

      -Symmetrical-Enigma Version 1.X.X, 03-SEP-2022 (CPW)
      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)
   */

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

   /*
   Here we need to find all of the instances where occultation state
   changes. To do so, we will break the span down into chunks corresponding
   to the step size, and then perform bisection to identify state changes.
   In each interval, we search by narrowing the interval (by adding the
   stepsize to the starting epoch until we either find the transition or we
   reach the maximum iterations)
   */

   /*
   First, determine the number of intervals to evaluate. We always want to hit
   the first and last.
   */
   int intervalCount =
      std::floor( ( upperEpochTime - lowerEpochTime ) / data.StepSize ) + 1;
   std::vector<double> epochTimes;
   epochTimes.reserve( intervalCount );
   for ( int i = 0; i < intervalCount; i++ ) {
      epochTimes.push_back( lowerEpochTime + ( i * data.StepSize ) );
   }
   epochTimes.push_back( upperEpochTime );

   /*
   Also reserve the same number of members in the occultation vector.
   */
   std::vector<bool> occultationVector;
   occultationVector.reserve( epochTimes.size() );

   /*
   Now, for each of the epochs, we evaluate the occultation status.
   */
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

   /*
   Next, iterate through all of the occultation vector members to find
   instances of changes. These will then be fed into our bisection algorithm.
   */
   std::vector<std::pair<
      std::pair<SpiceDouble, SpiceBoolean>,
      std::pair<SpiceDouble, SpiceBoolean>>>
      refinedIntervals;
   for ( size_t i = 1; i < epochTimes.size(); i++ ) {
      if ( occultationVector[i - 1] != occultationVector[i] ) {
         refinedIntervals.push_back( std::make_pair(
            std::make_pair( epochTimes[i - 1], occultationVector[i - 1] ),
            std::make_pair( epochTimes[i], occultationVector[i] ) ) );
      }
   }

   /*
   If no refined intervals have been found, then report as such.
   */
   if ( refinedIntervals.size() == 0 ) {
      std::cout << "No occultation events were detected." << std::endl;
      /*
      Note: even though it didn't find any events, it didn't error...just
      didn't find anything. So return true.
      */
      return true;
   }

   /*
   Now, for each interval, perform the bisection algorithm. All events will be
   reported as part of this routine.
   */
   for ( auto& p : refinedIntervals ) {
      // perform bisection within the interval
      if ( !bisectEpochs(
              targetID,
              occulterID,
              observerID,
              p.first.first,
              p.first.second,
              p.second.first,
              p.second.second,
              std::get<2>( data.OcculterDetails ).c_str(),
              std::get<0>( data.OcculterDetails ).c_str(),
              std::get<2>( data.TargetDetails ).c_str(),
              std::get<0>( data.TargetDetails ).c_str(),
              data.Tolerance ) )
      {
         return false;
      }
   }

   return true;
}

/*
This is the function which is used to perform the occultation search using
the cspice gfoclt_c routine. We feed in the SimulationData which was
retrieved prior to this call.
*/
SpiceCell* cppspice::performCSPICEOccSrch( const SimulationData& data ) {
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

      The function returns a SPICE window representing the set of time
   intervals, within the confinement period, when the specified occultation
   occurs.

      The endpoints of the time intervals comprising 'result' are interpreted
   as seconds past J2000 TDB.

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

      -Symmetrical-Enigma Version 1.X.X, 03-SEP-2022 (CPW)
      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)
   */

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
   SPICEDOUBLE_CELL( cnfine, CELLSIZE );
   SPICEDOUBLE_CELL( result, CELLSIZE );
   wninsd_c( lowerEpochTime, upperEpochTime, &cnfine );

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

/*
This is the function which is used to results of the occultation search. The
function accepts a SpiceCell and iterates through the results.
*/
void cppspice::reportSearchSummary( SpiceCell* result ) {
   SpiceInt    i{ 0 };
   SpiceDouble left{ 0.0 };
   SpiceDouble right{ 0.0 };
   SpiceChar   beginEpoch[TIMELEN];
   SpiceChar   endEpoch[TIMELEN];
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

      Any errors encountered in the CSPICE routines will be handled by
   CSPICE's native error handling. Otherwise, error messages are reported, and
   the function returns.

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

      -Symmetrical-Enigma Version 1.X.X, 03-SEP-2022 (CPW)
      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)
   */

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
         timout_c( left, TIMEFORMAT, TIMELEN, beginEpoch );
         /*
         Now do the same with the upper bound.
         */
         timout_c( right, TIMEFORMAT, TIMELEN, endEpoch );

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
/* End OccultationUtils.cpp */