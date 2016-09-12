# Ball Detection OpenCV code
The openCVStream opens your webcam in OpenCV and converts the frame to OpenCV IPLImage format. This code is in
the jumping sumop drone main file where the frames are getting called. It should do the same thing (Open the webcam)
when the drone frame is converted correctly.

The ball detection code uses your webcam and detects colour based on your toggled value. After the first code
is working with the drone then this code will be tested next.

The ball detection C++ code is added to compare the converted .c file to the original.

<h3>Make file instructions</h3>
To compile the code run:<br/>
1. make

To run the program type the desired one:<br/>
2. a)./ball_detection<br/>
b) ./openCVStream
