## SIMPLE FREEGLUT WATER SIMULATION

This is a simple water simulation program for assignment #3 of CS535.

Inspired by WebGL Water by Evan Wallace: http://madebyevan.com/webgl-water/.

Although the original WebGL Water by Evan Wallace is open-source,
I did not use any of it's code and approaches because I can't read js code.
Instead, I implemented the simplified wave equation using the double-buffer render pass in the template code.
Then I used 2 additional passes for simple water refractions and reflections.
These are all good-old methods not involving ray-tracing or any other high-level techniques.

The terrain generation uses siv::PerlinNoise library by Ryo Suzuki.

I did not implement much of the GUIs and all the parameters were hard-coded for a best performance.
(But I think we still can modify the parameters in the shaders since the shaders will be re-compiled every time before the program starts)

Expecting some bugs, e.g., the waves sometimes won't fade and the whole surface might blow up eventually.
