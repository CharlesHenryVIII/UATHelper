# UATHelper

## How to Build

### Need
- premake (https://premake.github.io/download)

### Building
* open cmd and navigate to the UATHelper directory
* Type and run: `C:/path/to/premake5.exe vs2019`
    * fill out the correct path to premake5
* open the VS solution
* Build/run from there

### TODO
- [ ] Convert to GLFW to remove the dependancy on dlls
- [ ] Look into possibly removing one of file paths currently needed
- [X] Add "Open Log" option in the error popup as well as remove the quit option
- [ ] Make "Run" bigger?
- [ ] cleanup?
- [ ] About
- [ ] Versioning (downloading new version?)
- [X] Remove forced server switches
- [X] Added quotes around -project in case the file path includes spaces
- [ ] Do we need seperate configs for each new file path or do they share the same settings?
