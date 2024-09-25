# Fast Sphere Collision

This project is a performance test for standard sphere collision detection.

The program fills several 4k pages with random floating point values and then creates "sphere" objects using those float values as the x, y, and z coordinates. Sphere collision is the checked by calculating the square of the distance between the objects and the sum of the squares of the radii for the two spheres. In the first pass the calculation is made using SSE hardware instructions, in the second pass the same calcuation is made using standard C math.  The total time to do the calculations 1 million times is then displayed.
