# ball-pivoting-algorithm

This is the implementation for the thesis **Surface Reconstruction from Point Clouds**
and implements a modified version of the ball pivoting algorithm.

This implementation was written in C++. To compile and run the program, follow these steps:

`> mkdir build`

`> cd build`

`> cmake ..`

`> make`

`> ./BPA <ball radius> <point cloud file path> <output file path> (-r)`

**ball radius**: ball radius 
**point cloud file path>**: point cloud (.obj)
**output file path**: output file path (.obj)
**-r (optional)**: reuses vertices (allows degenerated surface parts)
