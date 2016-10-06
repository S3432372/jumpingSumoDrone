# Project 106 - Jumping Sumo Drone
<h2>About</h2>
The jumping sumo drone project is a semester long programming assignment for  RMIT, Semester 2, [COSC2408 Programming Project 1](http://www1.rmit.edu.au/courses/039985 "Programming Project 1"). The goal of the project is simply to detect a predetermined object using the drone's camera by searching for it and then moving towards it when found.

<p>The "Red Cricket Ball HSV.txt" is a file with different range values detecting red; as the cricket ball is glossy and the lighting of the room may affect it, further adjustment may need to be made.</p>
<h2>Requirements</h2>

1. [ARDroneSDK3 Sample files](https://github.com/Parrot-Developers/Samples "ARDroneSDK3 Samples")
2. [Opencv 2.4.13](https://sourceforge.net/projects/opencvlibrary/ "Opencv 2.4.13")

<h2>Make File Instructions</h2>
<h3>Edit this line in the make file</h3>

1. [Line 4] SDK_DIR=/\<DIRECTORY_TO_SDK>/sdk/out/Unix-base/staging/usr<br>Change "\<DIRECTORY_TO_SDK>" to the location of your ARDroneSDK3 Sample file directory "sdk/out/Unix-base/staging/usr".

<h3>In the terminal run these commands</h3>

1. make
2. make run

<h2>Current tested and working operating systems</h2>

1. Ubuntu
2. Mac OSX

<h2>Related Works</h2>

1. [Colour Detection Code](https://github.com/Booppey/ball_detection "Booppey C++ Colour Detection")

<h2>Copyright</h2>
<p>This work was created for RMIT, Semester 2, COSC2408 Programming Project 1.</p>
The initial program was based off the [ARDroneSDK3 Sample files](https://github.com/Parrot-Developers/Samples "ARDroneSDK3 Samples") supplied by [Parrot](https://www.parrot.com/ "parrot") .
<p>Similarities in the code is based off related works and is only used for educational purposes. Any other similarities is purely coincidental.</p>
