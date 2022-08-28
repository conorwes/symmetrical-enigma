
# Symmetrical Enigma

This is an exploratory effort into working with NAIF's SPICE toolkit in C++. This is a project which entails writing a program which can identify occultation events (e.g. eclipses) between two bodies as seen by an observer. My work has been to write a wrapper which works with the CSPICE toolkit to leverage the native SPICE functionality, and then to provide interfaces for users which make it straightforward to query these events.

## Why "Symmetrical Enigma"?

symmetrical-enigma was the name that GitHub suggested for the repository, and I think it's pretty nifty.

## Acknowledgements

 - [CSPICE Toolkit Documentation](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/)
 - [About SPICE](https://naif.jpl.nasa.gov/naif/aboutspice.html)
 - [About NAIF](https://naif.jpl.nasa.gov/naif/about.html)
 - Credit given to the NAIF SPICE team for their work on the CSPICE toolkit.

## Authors

- [@conorwes](https://www.github.com/conorwes)

## License

[MIT](https://choosealicense.com/licenses/mit/)

## Installation

Part of this effort was also an exploration of working with C++ in Visual Studio Code, and I wanted to try the Microsoft Visual Studio Build Tools. So, this code is configured for that use case.

## Roadmap

- Add support for shape types other than Ellipsoid and Point.
- Expand support for arbitrary date formats.
- Implement more robust SPICE file date checking.
