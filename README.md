# Dyantra

![This is the Tara yantra](images/tara-yantra-iii.png)

This project has been successfully built on Mac with the following:
1.  Xcode 14 on Mac
2.  openFrameworksi of_v0.11.2_osx_release

Repo destination:
1.  Download from https://openframeworks.cc/download/ the latest version of openFrameworks
    * for example, I downloaded of_v0.11.2_osx_release.zip for  Mac
2.  Extract the zip
3.  Within the directory of_v0.11.2_osx_release, navigate to the directory /examples/math
4.  Clone the gaussian-Fields repo so that it sits alongside the other math examples
5.  Within the gaussian-Fields repo, open the .xcodeproj file
6.  Build & run the project

Notes
1.  For Xcode, I seem to get optimal performance with the build target set to $(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)
    * in the project navigator, click on gaussian-Fields
    * under the 'General' tab, go to 'Minimum Deployments'
    * in the 'macOS' dropdown menu, ensure the selection of $(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)
    * building commit 06d3176 in release mode on my macbook air m2 2022 (macOS 13.2.1), 22,000 particles + 4 attractors ran at 60 FPS 

Running:
1.  Pressing::
    * 'p' turns on/off the visualization of the potential field
    * 'z' assigns values of zero to the forces (for as long as it is pressed)
    * 'b' allows you to switch whether the velocity verlet integrator runs in the forward or the backward direction
    * Spacebar pauses/plays the simulation
2.  If you run into performance issues, reduce the number of attractors & the number of particles
3.  Have fun!




