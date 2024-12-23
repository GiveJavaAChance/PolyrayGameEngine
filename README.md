# Polyray Game Engine

## The Making
This project has been in development for several thousands of hours, while it may look small, there is many "engines" packed into one.
This is somewhat to a sequel to Graphics3D, although aimed at more modern rendering unlike Graphics3D which only used Java2D.

### Key Features:
* GPU renderer using OpenGL which has a built in 2D renderer with post processing, but can easily be used for 3D rendering with custom shaders etc.
The built in 2D renderer is designed to perfectly match Java2D's looks but be way, way faster.
AffineTransforms also behaves exactly the same which makes converting from Java2D rendering to this much easier and yeilds the same result.
* CPU renderer with a triangle rasterizer and 3D engine made from scratch which also has a frame debugger alot like GPU frame debuggers such as Nsight Graphics.
* Sound engine with common effects such as Low-Pass, Muffle, Lo-Fi, Distortion and a abillity to implement custom effects easily.
It also has a DFT for audio visualizations.
* Physics engine using Verlet integration for stable and easy simulations. Has built in collision detection with Quadtree and Octree
* Half bad Raytracer which implements basic material properties such as diffuse, reflection, gloss and diffraction.
* Many common utilities and tools for common usages.                                                     
FFT and IFFT for converting 1D data between time domain to frequency domain and vice versa, offten used in audio processing such as convolution for reverb.    
2D FFT and 2D IFFT for converting 2D data to frequency domain and vice versa, often used in image processing such as convolutional blur filters.

### Dependencies:
Requires LWJGL
