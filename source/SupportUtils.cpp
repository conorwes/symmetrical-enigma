// clang-format off
/*

- Source_File SupportUtils.cpp (Support utility code)

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
   corresponding header file (SupportUtils.hpp). These functions are not 
   specifically written to support occultation analysis.

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
In addition to the corresponding header file, we also need fstream.
*/
#include <fstream>

#include "SupportUtils.hpp"

/*
This is a utility to check for a NAIF ID based upon a provided body name.
Ultimately this just gets used for validation that we're working with real
objects.
*/
int cppspice::getNAIFIDFromName( const std::string& name ) {
   SpiceInt     code( 0 );
   SpiceBoolean found( false );
   bodn2c_c( name.c_str(), &code, &found );

   if ( found ) {
      return code;
   }
   else {
      std::cout
         << "Error: couldn't find an NAIF ID for the specified object '"
         << name << "'." << std::endl;
      return -1;
   }
}
// clang-format off
/*

- Brief I/O

   Variable  I/O  DESCRIPTION
   --------  ---  --------------------------------------------------
   name       I   The name of the object.

- Detailed_Input

   name     the name of the object which will be queried.
   
- Detailed_Output

   int      an int representing the NAIF ID corresponding to the
            object whose name is provided as the argument.

- Error Handling

   Any errors encountered by the CSPICE routine will be handled by the
   CSPICE error handling. Otherwise, if no NAIF ID is found, this returns
   -1.

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

/*
This function is used to validate a specified date to ensure that we're
working with a valid epoch.
*/
// TODO - expand capabilities to allow for arbitrary formats
bool cppspice::isValidDate( const std::string& input ) {
   /*
   Define some lambdas for clarity
   */

   /*
   This lambda is used to check for leap year. Although this is only used
   once, this is significantly cleaner than writing out the logic in situ.
   */
   auto isLeapYear = []( int year ) -> bool {
      /*
      Any year divisible by 400 is a leap year.
      */
      if ( year % 400 == 0 ) {
         return true;
      }
      /*
      Any year not divisible by 400, but divisible by 100 is not a leap year.
      */
      else if ( year % 100 == 0 ) {
         return false;
      }
      /*
      Finally, any year not divisible by 400, not divisible by 100, but
      divisible by 4 is a leap year.
      */
      else if ( year % 4 == 0 ) {
         return true;
      }

      return false;
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      int        I   The year in question.

   - Detailed_Input

      year     the year that we need to check for leap-year-ness.

   - Detailed_Output

      bool     returns true if the year is a leap year, false if not.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   This lambda is used to check individual components of a time string (i.e.
   hour:minute:second) for compliance with bounds.
   */
   auto isWithinTimeBounds = []( const std::string& input,
                                 const int          offset,
                                 const std::string& timeUnit ) {
      /*
      First retrieve the relevant time component that we're analysing by
      getting the string and then converting to an int.
      */
      auto inputString = input.substr( offset, 2 );
      auto inputInt    = std::atoi( inputString.c_str() );

      /*
      Zero initialize the lower bound since we don't need to do anything
      special with that, unlike the upper bound which will be specific to the
      unit being analyzed.
      */
      int lowerUnitBound{ 0 };
      int upperUnitBound;

      if ( timeUnit == "hour" ) {
         upperUnitBound = 23;
      }
      else {
         upperUnitBound = 59;
      }

      /*
      Finally, just ensure that our value is within the bounds appropriate to
      the unit.
      */
      if ( inputInt < lowerUnitBound || inputInt > upperUnitBound ) {
         std::cout << "Error: input " << timeUnit << " '" << inputString
                   << "' is not valid." << std::endl;
         return false;
      };

      return true;
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The time string to be analyzed.
      int        I   The offset from the beginning of the string which
                     points to the unit we want to analyze.
      string     I   The time unit that we want to analyze.

   - Detailed_Input

      input     the string which contains the representation of the time
                that we are analyzing.
      offset    the offset from the beginning of the string which points
                to the position of the values corresponding to the unit
                we are analyzing.
      timeUnit  the unit of time which we are analyzing, either hour,
                minute, or second.

   - Detailed_Output

      bool     returns true if the value is valid, or false if it is
               outside of the unit's bounds.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   The first step is ensuring that the date matches our date format regex.
   */
   if ( !std::regex_match( input, dateFormatRegex ) ) {
      std::cout << "Error: input epoch << '" << input
                << "' does not match the required format." << std::endl;
      return false;
   }

   /*
   Now we can perform some validation, including:
   1. Month is a real month.
   */
   auto monthString   = input.substr( 5, 3 );
   auto monthIterator = std::find_if(
      validMonths.begin(),
      validMonths.end(),
      [&monthString]( const MonthDetails& month ) {
         return month.first == monthString;
      } );

   if ( monthIterator == validMonths.end() ) {
      std::cout << "Error: input month '" << monthString
                << "' does not correspond to a valid month." << std::endl;
      return false;
   }

   /*
   2. Day is a real day of said month.
   */
   auto dayString = input.substr( 9, 2 );
   auto dayInt    = std::atoi( dayString.c_str() );

   /*
   Support leapyear if applicable.
   */
   auto maxDay = monthIterator->second;
   if (
      monthString == "FEB" &&
      isLeapYear( std::atoi( input.substr( 0, 4 ).c_str() ) ) )
   {
      maxDay++;
   }

   if ( dayInt < 1 || dayInt > maxDay ) {
      std::cout
         << "Error: input day '" << dayString
         << "' does not correspond to a valid day number for the month of '"
         << monthString << "'." << std::endl;
      return false;
   }

   /*
   3. Time is 00-23, 00-59, 00-59.
   */
   if ( !isWithinTimeBounds( input, 12, "hour" ) ) {
      return false;
   }

   if ( !isWithinTimeBounds( input, 15, "minute" ) ) {
      return false;
   }

   if ( !isWithinTimeBounds( input, 18, "second" ) ) {
      return false;
   }

   /*
   4. Year is covered by the bsp we've furnished.
   */
   SpiceDouble inputDouble{ 0.0 };
   SpiceDouble posVel[6];
   str2et_c( input.c_str(), &inputDouble );

   SpiceInt     targetID;
   SpiceInt     observerID;
   SpiceBoolean found;
   SpiceDouble  lt;
   // TODO - this assumes that sun and earth are in our spice file.
   bodn2c_c( "SUN", &targetID, &found );
   bodn2c_c( "EARTH", &observerID, &found );

   spkez_c( targetID, inputDouble, "IAU_SUN", "LT", observerID, posVel, &lt );

   return true;
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The time to be analyzed.

   - Detailed_Input

      input     the string which contains the representation of the time
                that we are analyzing.

   - Detailed_Output

      bool     returns true if the time is valid.

   - Error Handling

      Any errors encountered by the CSPICE API are handled by the native error
      handling. Otherwise, no other error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
This function is a second bit of date validation, namely to compare the
specified upper and lower bounds to ensure that the dates involved are
ready for comparison.
*/
bool cppspice::areValidDateBounds(
   const std::string& lowerDateBound,
   const std::string& upperDateBound ) {

   /*
   If we've already gotten here, we can be confident that the string format is
   good for literal comparison, so we can leverage c++'s native string
   comparison functionality.
   */

   /*
   First, let's check that the dates are not identical.
   */
   if ( lowerDateBound == upperDateBound ) {
      std::cout << "Error: lower and upper bounds are identical."
                << std::endl;
      return false;
   }

   /*
   Next, ensure that the lower bound is not a date after the upper bound.
   */
   if ( lowerDateBound > upperDateBound ) {
      std::cout << "Error: lower bound is greater than the upper bound."
                << std::endl;
      return false;
   }

   return true;
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The lower bound time to be analyzed.
      string     I   The upper bound time to be analyzed.

   - Detailed_Input

      lowerDateBound  the string which contains the representation of the time
                        which is the lower bound of the time span.
      upperDateBound  the string which contains the representation of the time
                        which is the upper bound of the time span.

   - Detailed_Output

      bool     returns true if the bounds are valid.

   - Error Handling

      None.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
This is a helper function which queries users for a kernel, and then
furnishes the kernel if it exists.
*/
bool cppspice::furnishSPICEKernel( const std::string& kernelName ) {
   /*
   First prompt for the kernel path.
   */
   std::string path;
   std::cout << "Specify the " << kernelName
             << " kernel's path: " << std::endl;
   std::getline( std::cin, path );

   /*
   Before feeding the kernel into CSPICE, we need to ensure it exists.
   */
   disambiguateRelativePath( path );
   if ( FILE* file = fopen( path.c_str(), "r" ) ) {
      fclose( file );
   }
   else {
      std::cout << "Error: the specified kernel '" << path
                << "' could not be located." << std::endl;
      return false;
   }

   /*
   Finally, call the CSPICE furnsh function.
   */
   furnsh_c( path.c_str() );
   return true;
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The name of the kernel.

   - Detailed_Input

      kernelName  a string which contains the name of the kernel for the
                  purpose of prompting the user.

   - Detailed_Output

      bool     returns false if the file cannot be found. otherwise returns
               true.

   - Error Handling

      Any errors encountered in the CSPICE API are handled using the native
      CSPICE error handling.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
This is a function to query a user for body details for one of the
participants in the occultation analysis. Validation is also performed as
part of this function.
*/
bool cppspice::queryParticipantDetails(
   const std::string&  participantType,
   ParticipantDetails& participantInfo ) {
   /*
   First prompt for the body name.
   */
   std::string input;
   std::cout << participantType << " Body: " << std::endl;
   std::getline( std::cin, input );

   /*
   Perform some validation to ensure that we're working with a valid object.
   */
   if ( getNAIFIDFromName( input ) == -1 ) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object."
                << std::endl;
      return false;
   }
   std::string participantName = input;

   /*
   Next we need to retrieve the body shape.
   */
   std::cout << participantType << " Body Shape: " << std::endl;
   for ( auto& s : validShapeTypes ) {
      std::cout << "- " << s << std::endl;
   }
   std::getline( std::cin, input );

   /*
   Iterate through the known valid shape types to ensure that we're working
   with a valid type.
   */
   auto shapeIterator = std::find_if(
      validShapeTypes.begin(),
      validShapeTypes.end(),
      [&input]( const std::string& shape ) {
         return shape == input;
      } );

   if ( shapeIterator == validShapeTypes.end() ) {
      std::cout << "Error: the specified body shape '" << input
                << "' is not a valid option." << std::endl;
      return false;
   };
   std::string participantBodyShape = input;

   /*
   Lastly, we need to retrieve the body frame.
   */
   std::cout << participantType << " Body Frame: " << std::endl;
   std::getline( std::cin, input );

   /*
   To validate, we can leverage namfrm_c and see if the frame exists to
   CSPICE's knowledge.
   */
   SpiceInt frameCode{ 0 };
   namfrm_c( input.c_str(), &frameCode );
   if ( frameCode == 0 ) {
      std::cout << "Error: the specified body frame: '" << input
                << "' is not recognized." << std::endl;
      return false;
   }
   std::string participantBodyFrame = input;

   /*
   Turn the data into a tuple and return true.
   */
   participantInfo = std::make_tuple(
      participantName,
      participantBodyShape,
      participantBodyFrame );

   return true;
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   A string specifying the label of the participant.
      tuple      O   A ParticipantDetails tuple which contains the
                     information.

   - Detailed_Input

      participantType   a string which contains the label of the participant
                        for the purpose of prompting the user.

   - Detailed_Output

      participantInfo   a ParticipantDetails tuple which receives the details
                        and is then used in later analysis.
      bool              returns false if any errors occur, otherwise returns 
                        true.

   - Error Handling

      Any errors encountered in the CSPICE API are handled using the native
      CSPICE error handling.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
This is a utility to query a user for all of the user-specified components
of the analysis using the console. The data are then used to populate the
SimulationData which then gets fed into our occultation analysis.
*/
bool cppspice::queryConfigurationDetails( SimulationData& data ) {

   std::string input;

   /*
   Before we do anything else, let's furnish the kernels we'll need for this
   program. For this program, we'll need a pck, tsp, and bsp file, so furnish
   those now.
   */
   if ( !furnishSPICEKernel( "P Constants" ) )
      return false;
   if ( !furnishSPICEKernel( "Timespan" ) )
      return false;
   if ( !furnishSPICEKernel( "Planetary Ephemerides" ) )
      return false;

   /*
   Now we're ready to roll. Let's get the other things we'll need here.
   */

   /*
   First, query the user for the date range and perform validation to ensure
   that we're working with a valid date range.
   */
   std::cout << "Lower Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::getline( std::cin, input );
   if ( !isValidDate( input ) ) {
      return false;
   }
   data.LowerBoundEpoch = input;

   std::cout << "Upper Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::getline( std::cin, input );
   if ( !isValidDate( input ) ) {
      return false;
   }
   data.UpperBoundEpoch = input;

   /*
   Now that we have our bounds and confirmed they're valid, ensure that they
   make a legitimate range.
   */
   if ( !areValidDateBounds( data.LowerBoundEpoch, data.UpperBoundEpoch ) ) {
      return false;
   }

   /*
   Next we get the step size. Ensure that we have a number that will actually
   support proper propagation.
   */
   std::cout << "Step Size (s): " << std::endl;
   std::getline( std::cin, input );
   data.StepSize = std::atof( input.c_str() );
   if ( data.StepSize <= 0.0 ) {
      std::cout << "Error: step size cannot be negative or zero."
                << std::endl;
      return false;
   }

   /*
   Next retrieve the occultation type and validate.
   */
   std::cout << "Occultation Type: " << std::endl;
   for ( auto& type : validOccultationTypes ) {
      std::cout << "- " << type << std::endl;
   }
   std::getline( std::cin, input );

   /*
   Iterate through valid types to ensure we have a type with which we can
   work.
   */
   auto type_it = std::find_if(
      validOccultationTypes.begin(),
      validOccultationTypes.end(),
      [&input]( const std::string& type ) {
         return type == input;
      } );

   if ( type_it == validOccultationTypes.end() ) {
      std::cout << "Error: the specified occultation type '" << input
                << "' is not a valid option." << std::endl;
      return false;
   };
   data.OccultationType = input;

   /*
   Retrieve the participant details for both the occulter and the target.
   */
   if ( !queryParticipantDetails( "Occulting", data.OcculterDetails ) )
      return false;

   if ( !queryParticipantDetails( "Target", data.TargetDetails ) )
      return false;

   /*
   Retrieve the name of the observer and perform validation.
   */
   std::cout << "Observing Body: " << std::endl;
   std::getline( std::cin, input );

   /*
   We can validate by checking that there is a corresponding NAIF ID.
   */
   if ( getNAIFIDFromName( input ) == -1 ) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object."
                << std::endl;
      return false;
   }
   data.ObserverName = input;

   /*
   Finally, we need the tolerance value.
   */
   std::cout << "Tolerance: " << std::endl;
   std::getline( std::cin, input );
   data.Tolerance = std::atof( input.c_str() );
   if ( data.Tolerance <= 0.0 ) {
      std::cout << "Error: the tolerance value '" << data.Tolerance
                << "' is less than or equal to zero." << std::endl;
      return false;
   }

   /*
   Now we have a fully formed data struct that is ready for use.
   */
   return true;
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      struct     O   A SimulationData struct for use in occultation
                     analysis.

   - Detailed_Input

      None.

   - Detailed_Output

      data              a SimulationData struct which gets populated
                        using user input. This struct is later used
                        in occultation analysis.
      bool              returns false if any errors occur, otherwise returns 
                        true.

   - Error Handling

      Any errors encountered in the CSPICE API are handled using the native
      CSPICE error handling.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
This utility queries the user for a configuration file, which is then
parsed. The data retrieved from the configuration file is fed into a
SimulationData object, which is then used in the occulation analysis.
*/
bool cppspice::parseConfigurationFile(
   const std::string& filename,
   SimulationData&    data ) {

   /*
   Define some helpful lambdas for I/O.
   */
   /*
   Just a simple lambda to trim whitespace from the left side of a string.
   */
   auto trimLeft = []( std::string& s ) -> void {
      s.erase(
         s.begin(),
         std::find_if( s.begin(), s.end(), []( unsigned char ch ) {
            return !std::isspace( ch );
         } ) );
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The string to trim

   - Detailed_Input

      s        the string that should have whitespace trimmed.

   - Detailed_Output

      void     returns void.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   A lambda to trim whitespace from the right side of a string.
   */
   auto trimRight = []( std::string& s ) -> void {
      s.erase(
         std::find_if(
            s.rbegin(),
            s.rend(),
            []( unsigned char ch ) {
               return !std::isspace( ch );
            } )
            .base(),
         s.end() );
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The string to trim

   - Detailed_Input

      s        the string that should have whitespace trimmed.

   - Detailed_Output

      void     returns void.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   This lambda combines the two previous lambdas to trim whitespace from both
   sides of a string.
   */
   auto trim = [trimLeft, trimRight]( std::string& s ) -> void {
      trimLeft( s );
      trimRight( s );
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The string to trim

   - Detailed_Input

      s        the string that should have whitespace trimmed.

   - Detailed_Output

      void     returns void.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   This lambda compares a specified string against the valid shape types.
   */
   auto isValidShapeType = []( std::string& s ) -> bool {
      auto shapeIterator = std::find_if(
         validShapeTypes.begin(),
         validShapeTypes.end(),
         [&s]( const std::string& shape ) {
            return shape == s;
         } );

      if ( shapeIterator == validShapeTypes.end() ) {
         std::cout << "Error: the specified body shape '" << s
                   << "' is not a valid option." << std::endl;
         return false;
      };
      return true;
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The shape type to validate.

   - Detailed_Input

      s        the string that represents the shape type which is to be
               validated.

   - Detailed_Output

      bool     returns true if the shape type is valid.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   This lambda validates a frame ID using the CSPICE API.
   */
   auto isValidBodyFrame = []( std::string& s ) -> bool {
      SpiceInt frameCode{ 0 };
      namfrm_c( s.c_str(), &frameCode );
      if ( frameCode == 0 ) {
         std::cout << "Error: the specified body frame: '" << s
                   << "' is not recognized." << std::endl;
         return false;
      }
      return true;
   };
   // clang-format off
   /*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   The body frame to validate

   - Detailed_Input

      s        the string that represents the frame ID which is to be
               validated.

   - Detailed_Output

      bool     returns true if the frame ID is valid.

   - Error Handling

      No error handling is required.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

   */
   // clang-format on

   /*
   The first step is parsing the file. We can assume that the filename has
   been validated before we get here.
   */
   std::ifstream            file( filename );
   std::string              line;
   std::vector<std::string> fileContents;
   while ( std::getline( file, line ) ) {
      fileContents.push_back( line );
   }

   /*
   This is a little ugly, but we need to now iterate through each member in
   the fileContents vector so we can populate the SimulationData.
   */
   std::string identifier, content;
   char        delimiter = ':';
   for ( auto& c : fileContents ) {
      identifier = c.substr( 0, c.find( delimiter ) );
      content    = c.substr( c.find( delimiter ) + 1, c.length() );
      trim( identifier );
      trim( content );
      if ( identifier == "PConstants" ) {
         /*
         For each of the kernels, make sure we can disambiguate the relative
         paths and then attempt to furnish the kernel.
         */
         disambiguateRelativePath( content );
         furnsh_c( content.c_str() );
      }
      else if ( identifier == "Timespan" ) {
         /*
         For each of the kernels, make sure we can disambiguate the relative
         paths and then attempt to furnish the kernel.
         */
         disambiguateRelativePath( content );
         furnsh_c( content.c_str() );
      }
      else if ( identifier == "PlanetaryEphemerides" ) {
         /*
         For each of the kernels, make sure we can disambiguate the relative
         paths and then attempt to furnish the kernel.
         */
         disambiguateRelativePath( content );
         furnsh_c( content.c_str() );
      }
      else if ( identifier == "LowerBoundEpoch" ) {
         /*
         For now, we just need to validate that this date meets our format
         expectations, and has valid unit values.
         */
         if ( !isValidDate( content ) ) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
         data.LowerBoundEpoch = content;
      }
      else if ( identifier == "UpperBoundEpoch" ) {
         /*
         For now, we just need to validate that this date meets our format
         expectations, and has valid unit values.
         */
         if ( !isValidDate( content ) ) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
         data.UpperBoundEpoch = content;
      }
      else if ( identifier == "StepSize" ) {
         /*
         Step size just needs to be greater than zero.
         */
         data.StepSize = std::atof( content.c_str() );
         if ( data.StepSize <= 0.0 ) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
      }
      else if ( identifier == "OccultationType" ) {
         /*
         Retrieve the occultation type and ensure that it is valid.
         */
         auto type_it = std::find_if(
            validOccultationTypes.begin(),
            validOccultationTypes.end(),
            [&content]( const std::string& type ) {
               return type == content;
            } );

         if ( type_it == validOccultationTypes.end() ) {
            std::cout << "Error: the value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         };
         data.OccultationType = content;
      }
      else if ( identifier == "OccultingBodyShape" ) {
         /*
         For now, we just need to ensure that we have a valid body shape.
         */
         if ( !isValidShapeType( content ) ) {
            return false;
         }
         std::get<1>( data.OcculterDetails ) = content;
      }
      else if ( identifier == "OccultingBodyFrame" ) {
         /*
         Retrieve the frame and validate.
         */
         if ( !isValidBodyFrame( content ) ) {
            return false;
         }
         std::get<2>( data.OcculterDetails ) = content;
      }
      else if ( identifier == "OccultingBody" ) {
         /*
         Retrieve the name and validate.
         */
         if ( getNAIFIDFromName( content ) == -1 ) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object."
                      << std::endl;
            return false;
         }
         std::get<0>( data.OcculterDetails ) = content;
      }
      else if ( identifier == "TargetBodyShape" ) {
         /*
         For now, we just need to ensure that we have a valid body shape.
         */
         if ( !isValidShapeType( content ) ) {
            return false;
         }
         std::get<1>( data.TargetDetails ) = content;
      }
      else if ( identifier == "TargetBodyFrame" ) {
         /*
         Retrieve the frame and validate.
         */
         if ( !isValidBodyFrame( content ) ) {
            return false;
         }
         std::get<2>( data.TargetDetails ) = content;
      }
      else if ( identifier == "TargetBody" ) {
         /*
         Retrieve the name and validate.
         */
         if ( getNAIFIDFromName( content ) == -1 ) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object."
                      << std::endl;
            return false;
         }
         std::get<0>( data.TargetDetails ) = content;
      }
      else if ( identifier == "ObservingBody" ) {
         /*
         Retrieve the name and validate.
         */
         if ( getNAIFIDFromName( content ) == -1 ) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object."
                      << std::endl;
            return false;
         }
         data.ObserverName = content;
      }
      else if ( identifier == "Tolerance" ) {
         /*
         Tolerance just needs to be nonzero and positive. So check
         accordingly.
         */
         data.Tolerance = std::atof( content.c_str() );
         if ( data.Tolerance <= 0.0 ) {
            std::cout << "Error: the value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
      }
      else {
         /*
         If we don't have a match, just ignore it and move on.
         */
         continue;
      }
   }

   /*
   Theoretically, we should not have a fully configured SimulationData struct.
   If not, we will handle errors later.
   */
   return true;
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string     I   A string containing the name of the file to parse.
      struct     O   A SimulationData struct containing the data used in
                     occultation analysis.

   - Detailed_Input

      filename       a string containing the filename of the configuration
                     file which will be parsed to populate the 
                     SimulationData struct we'll use elsewhere.     

   - Detailed_Output

      data              a SimulationData struct which gets populated
                        using file input. This struct is later used
                        in occultation analysis.
      bool              returns false if any errors occur, otherwise returns 
                        true.

   - Error Handling

      Any errors encountered in the CSPICE API are handled using the native
      CSPICE error handling.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
This is a simple utility to take relative paths and ensure that they are
translated to the correct path.
*/
void cppspice::disambiguateRelativePath( std::string& path ) {
   /*
   If the path starts with a period, we're using a relative path. We'll then
   go ahead and concatenate the working directory and the relative path to get
   a disambiguated path.
   */
   if ( path[0] = '.' ) {
      /*
      Get the current path.
      */
      std::string currentPath = _getcwd( NULL, 0 );

      /*
      Check the current path for the source folder, as this is where this has
      had issues historically.
      */
      auto sourceOffset = currentPath.rfind( "source" );

      /*
      If we have found source, then we need to concatenate the strings.
      */
      if ( sourceOffset != std::string::npos ) {
         currentPath = currentPath.substr( 0, sourceOffset );
         path        = currentPath + path.substr( 2, path.length() );

         /*
         Finally, replace any forward slashes with backslashes for
         consistency.
         */
         std::replace( path.begin(), path.end(), '/', '\\' );
      }
   }
}
// clang-format off
/*

   - Brief I/O

      Variable  I/O  DESCRIPTION
      --------  ---  --------------------------------------------------
      string    I/O   A string containing the relative path.

   - Detailed_Input

      path        a string containing the relative path which we want to
                  disambiguate.    

   - Detailed_Output

      void.

   - Error Handling

      None.

   - Author

      C.P. Westphal     (self)

   - Version

      -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on