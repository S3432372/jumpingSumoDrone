<h1>Ball Detection OpenCV code</h1>
<h2>NOTE</h2>
<p>This branch was used to independently work on one functionality of the main code. It is now outdated and is only kept for reference purposes.</p>
<h2>About</h2>
The openCVStream opens your webcam in OpenCV and converts the frame to OpenCV IPLImage format. This code is in
the jumping sumop drone main file where the frames are getting called. It should do the same thing (Open the webcam)
when the drone frame is converted correctly.

The ball detection code uses your webcam and detects colour based on your toggled value. After the first code
is working with the drone then this code will be tested next.

The ball detection C++ code is added to compare the converted .c file to the original.
