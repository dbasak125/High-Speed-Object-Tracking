# High-Speed-Object-Tracking
A computer vision system to perform high-speed rudimentary feature tracking using an optimized pan-tilt camera assembly.
Also, discusses heuristics based algorithms to track a human face with fair accuracy.

Project implemented in C using the intel OpenCV library.
UART communications subroutine written in x86 Assembly.
Pan-Tilt camera board uses Philips 89C51 Microcontroller as the interface between PC and Digital Driver Board (clock generator, PWM, H-Bridge, etc.) . It decodes the co-ordinates sent by the Image Tracking program and uses it to drive the pan-tilt motors, so that object is always kept in the center of frame.


project website: https://visionpod.wordpress.com/high-speed-object-tracking/
