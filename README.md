# Dyantra

![This is the Tara yantra](images/tara-yantra-iii.png)

This project has been successfully built on Mac with the following:
1.  Xcode 14 
2.  openFrameworks of_v0.11.2_osx_release

Repo destination:
1.  Download from https://openframeworks.cc/download/ the latest version of openFrameworks
    * for example, I downloaded of_v0.11.2_osx_release.zip for  Mac
2.  Extract the zip
3.  Within the directory of_v0.11.2_osx_release, navigate to /examples/math
4.  Clone the dyantra repo so that it sits alongside the other math examples
5.  Within the dyantra directory, open the .xcodeproj file
6.  Build & run the project

Notes
1.  For Xcode, I seem to get optimal performance with the build target set to $(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)
    * in the project navigator, click on dyantra
    * under the 'General' tab, go to 'Minimum Deployments'
    * in the 'macOS' dropdown menu, ensure the selection of $(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)
    * building commit 6932ed1 in debug mode on my macbook air m2 2022 (macOS 13.3.1), 24,000 particles + 4 attractors ran at 60 FPS 

Running:
1.  Pressing::
    * 'p' turns on/off the visualization of the potential field
    * 'b' allows you to switch whether the velocity verlet integrator runs in the forward or the backward direction
    * 'r' allows you to reset the positions of the particles as they were at time zero
    * 'c' turns on/off the viz of the contour lines for the potential field
    * Spacebar pauses/plays the simulation
2.  if you want dyantra to upload a different *.svg file:
    * place the file in the /bin/data/ directory
    * change the relevant line in the setup() routine of ofApp.cpp to specify the filename
3.  If you run into performance issues, reduce the number of attractors & the number of particles
4.  Have fun!




