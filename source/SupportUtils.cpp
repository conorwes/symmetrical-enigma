#include <algorithm>
#include <fstream>

#include "SupportUtils.hpp"

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

// TODO - expand capabilities to allow for arbitrary formats
bool cppspice::isValidDate( const std::string& input ) {

   // Define some lambdas for clarity
   // simple lambda to check for leap year. have it as a lambda for
   // cleanliness later
   auto isLeapYear = []( int year ) -> bool {
      if ( year % 400 == 0 ) {
         return true;
      }
      else if ( year % 100 == 0 ) {
         return false;
      }
      else if ( year % 4 == 0 ) {
         return true;
      }

      return false;
   };

   // simple lambda to check individual components of a time (i.e.
   // hour:minute:second) for compliance with range
   auto isWithinTimeBounds = []( const std::string& input,
                                 const int          offset,
                                 const std::string& timeUnit ) {
      auto inputString = input.substr( offset, 2 );
      auto inputInt    = std::atoi( inputString.c_str() );

      int lowerUnitBound{ 0 };
      int upperUnitBound{ 0 };

      if ( timeUnit == "hour" ) {
         upperUnitBound = 23;
      }
      else {
         upperUnitBound = 59;
      }

      if ( inputInt < lowerUnitBound || inputInt > upperUnitBound ) {
         std::cout << "Error: input " << timeUnit << " '" << inputString
                   << "' is not valid." << std::endl;
         return false;
      };

      return true;
   };

   // First we need to check that the input matches the expected string format
   if ( !std::regex_match( input, dateFormatRegex ) ) {
      std::cout << "Error: input epoch << '" << input
                << "' does not match the required format." << std::endl;
      return false;
   }

   // Now we can perform some validation, including:
   // 1. month is a real month
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

   // 2. day is a real day of said month
   auto dayString = input.substr( 9, 2 );
   auto dayInt    = std::atoi( dayString.c_str() );

   // add support for leap years
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

   // 3. time is 00-23, 00-59, 00-59
   if ( !isWithinTimeBounds( input, 12, "hour" ) ) {
      return false;
   }

   if ( !isWithinTimeBounds( input, 15, "minute" ) ) {
      return false;
   }

   if ( !isWithinTimeBounds( input, 18, "second" ) ) {
      return false;
   }

   // 4. year is covered by the bsp we've furnished
   SpiceDouble inputDouble{ 0.0 };
   SpiceDouble posVel[6];
   str2et_c( input.c_str(), &inputDouble );

   SpiceInt     target_id;
   SpiceInt     observer_id;
   SpiceBoolean found;
   SpiceDouble  lt;
   bodn2c_c( "SUN", &target_id, &found );
   bodn2c_c( "EARTH", &observer_id, &found );

   spkez_c(
      target_id,
      inputDouble,
      "IAU_SUN",
      "LT",
      observer_id,
      posVel,
      &lt );

   return true;
}

bool cppspice::areValidDateBounds(
   const std::string& lowerDateBound,
   const std::string& upperDateBound ) {

   // If we've already gotten here, we can be confident that the string format
   // is good for literal comparison, so we can compare on the string
   if ( lowerDateBound == upperDateBound ) {
      std::cout << "Error: lower and upper bounds are identical."
                << std::endl;
      return false;
   }

   if ( lowerDateBound > upperDateBound ) {
      std::cout << "Error: lower bound is greater than the upper bound."
                << std::endl;
      return false;
   }

   return true;
}

bool cppspice::furnishSPICEKernel( const std::string& kernelName ) {
   std::string path;
   std::cout << "Specify the " << kernelName
             << " kernel's path: " << std::endl;
   std::getline( std::cin, path );

   // Before feeding this kernel into CSPICE, let's make sure it actually
   // exists
   disambiguateRelativePath( path );
   if ( FILE* file = fopen( path.c_str(), "r" ) ) {
      fclose( file );
   }
   else {
      std::cout << "Error: the specified kernel '" << path
                << "' could not be located." << std::endl;
      return false;
   }

   // Call the CSPICE furnsh_c routine. No need to error check, since the
   // CSPICE routines are self-reporting.
   furnsh_c( path.c_str() );
   return true;
}

bool cppspice::queryParticipantDetails(
   const std::string&  participantType,
   ParticipantDetails& participantInfo ) {
   std::string input;
   std::cout << participantType << " Body: " << std::endl;
   std::getline( std::cin, input );
   if ( getNAIFIDFromName( input ) == -1 ) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object."
                << std::endl;
      return false;
   }
   std::string participantName = input;

   std::cout << participantType << " Body Shape: " << std::endl;

   // TODO - add support for DSK/UNPRIORITIZED
   std::vector<std::string> validShapes = {
      "ELLIPSOID",
      "POINT" /*, "DSK/UNPRIORITIZED"*/ };

   for ( auto& s : validShapes ) {
      std::cout << "- " << s << std::endl;
   }
   std::getline( std::cin, input );

   auto shapeIterator = std::find_if(
      validShapes.begin(),
      validShapes.end(),
      [&input]( const std::string& shape ) {
         return shape == input;
      } );

   if ( shapeIterator == validShapes.end() ) {
      std::cout << "Error: the specified body shape '" << input
                << "' is not a valid option." << std::endl;
      return false;
   };
   std::string participantBodyShape = input;

   std::cout << participantType << " Body Frame: " << std::endl;
   std::getline( std::cin, input );
   SpiceInt frameCode{ 0 };
   namfrm_c( input.c_str(), &frameCode );
   if ( frameCode == 0 ) {
      std::cout << "Error: the specified body frame: '" << input
                << "' is not recognized." << std::endl;
      return false;
   }
   std::string participantBodyFrame = input;

   participantInfo = std::make_tuple(
      participantName,
      participantBodyShape,
      participantBodyFrame );

   return true;
}

bool cppspice::queryConfigurationDetails( SimulationData& data ) {

   std::string input;

   // Before we do anything else, let's furnish the kernels
   // we'll need for this
   // program. For this program, we'll need a pck, tsp, and bsp file, so
   // furnish those now
   if ( !furnishSPICEKernel( "P Constants" ) )
      return false;
   if ( !furnishSPICEKernel( "Timespan" ) )
      return false;
   if ( !furnishSPICEKernel( "Planetary Ephemerides" ) )
      return false;

   // Now we're ready to roll. Let's get the other things we'll need here.

   // Let's get the date range and perform some validation
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

   if ( !areValidDateBounds( data.LowerBoundEpoch, data.UpperBoundEpoch ) ) {
      return false;
   }

   // Now we'll get the step size
   std::cout << "Step Size (s): " << std::endl;
   std::getline( std::cin, input );
   data.StepSize = std::atof( input.c_str() );
   if ( data.StepSize < 0.0 ) {
      std::cout << "Error: negative step sizes are not supported."
                << std::endl;
      return false;
   }

   // Next retrieve the occultation type and validate
   std::cout << "Occultation Type: " << std::endl;

   for ( auto& type : validOccultationTypes ) {
      std::cout << "- " << type << std::endl;
   }
   std::getline( std::cin, input );

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

   if ( !queryParticipantDetails( "Occulting", data.OcculterDetails ) )
      return false;

   if ( !queryParticipantDetails( "Target", data.TargetDetails ) )
      return false;

   // get the observer
   std::cout << "Observing Body: " << std::endl;
   std::getline( std::cin, input );
   if ( getNAIFIDFromName( input ) == -1 ) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object."
                << std::endl;
      return false;
   }
   data.ObserverName = input;

   // get the tolerance
   std::cout << "Tolerance: " << std::endl;
   std::getline( std::cin, input );
   data.Tolerance = std::atof( input.c_str() );
   if ( data.Tolerance < 0.0 ) {
      std::cout << "Error: the tolerance value '" << data.Tolerance
                << "' is less than zero." << std::endl;
      return false;
   }

   return true;
}

bool cppspice::parseConfigurationFile(
   const std::string& filename,
   SimulationData&    data ) {
   std::ifstream            file( filename );
   std::string              line;
   std::vector<std::string> fileContents;
   while ( std::getline( file, line ) ) {
      fileContents.push_back( line );
   }

   char delimiter = ':';
   auto trimLeft  = []( std::string& s ) {
      s.erase(
         s.begin(),
         std::find_if( s.begin(), s.end(), []( unsigned char ch ) {
            return !std::isspace( ch );
         } ) );
   };

   auto trimRight = []( std::string& s ) {
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

   auto trim = [trimLeft, trimRight]( std::string& s ) {
      trimLeft( s );
      trimRight( s );
   };

   std::string identifier, content;

   for ( auto& c : fileContents ) {
      identifier = c.substr( 0, c.find( delimiter ) );
      content    = c.substr( c.find( delimiter ) + 1, c.length() );
      trim( identifier );
      trim( content );
      if ( identifier == "PConstants" ) {
         disambiguateRelativePath( content );
         furnsh_c( content.c_str() );
      }
      else if ( identifier == "Timespan" ) {
         disambiguateRelativePath( content );
         furnsh_c( content.c_str() );
      }
      else if ( identifier == "PlanetaryEphemerides" ) {
         disambiguateRelativePath( content );
         furnsh_c( content.c_str() );
      }
      else if ( identifier == "LowerBoundEpoch" ) {
         if ( !isValidDate( content ) ) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
         data.LowerBoundEpoch = content;
      }
      else if ( identifier == "UpperBoundEpoch" ) {
         if ( !isValidDate( content ) ) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
         data.UpperBoundEpoch = content;
      }
      else if ( identifier == "StepSize" ) {
         data.StepSize = std::atof( content.c_str() );
         if ( data.StepSize < 0.0 ) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
      }
      else if ( identifier == "OccultationType" ) {
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
         // TODO - add some validation here
         std::get<1>( data.OcculterDetails ) = content;
      }
      else if ( identifier == "OccultingBodyFrame" ) {
         // TODO - add some validation here
         std::get<2>( data.OcculterDetails ) = content;
      }
      else if ( identifier == "OccultingBody" ) {
         if ( getNAIFIDFromName( content ) == -1 ) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object."
                      << std::endl;
            return false;
         }
         std::get<0>( data.OcculterDetails ) = content;
      }
      else if ( identifier == "TargetBodyShape" ) {
         // TODO - add some validation here
         std::get<1>( data.TargetDetails ) = content;
      }
      else if ( identifier == "TargetBodyFrame" ) {
         // TODO - add some validation here
         std::get<2>( data.TargetDetails ) = content;
      }
      else if ( identifier == "TargetBody" ) {
         if ( getNAIFIDFromName( content ) == -1 ) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object."
                      << std::endl;
            return false;
         }
         std::get<0>( data.TargetDetails ) = content;
      }
      else if ( identifier == "ObservingBody" ) {
         if ( getNAIFIDFromName( content ) == -1 ) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object."
                      << std::endl;
            return false;
         }
         data.ObserverName = content;
      }
      else if ( identifier == "Tolerance" ) {
         data.Tolerance = std::atof( content.c_str() );
         if ( data.Tolerance <= 0.0 ) {
            std::cout << "Error: the value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
      }
      else {
         continue;
      }
   }

   return true;
}

void cppspice::disambiguateRelativePath( std::string& path ) {
   if ( path[0] = '.' ) {
      std::string currentPath  = _getcwd( NULL, 0 );
      auto        sourceOffset = currentPath.rfind( "source" );
      if ( sourceOffset != std::string::npos ) {
         currentPath = currentPath.substr( 0, sourceOffset );
         path        = currentPath + path.substr( 2, path.length() );
         std::replace( path.begin(), path.end(), '/', '\\' );
      }
   }
}