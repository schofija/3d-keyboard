# CS450 FALL 2021 FINAL PROJECT -- schofija

This is a 3D model of a keyboard (specifically a 60% layout).

All non-modifier keys are able to be pressed/released by their corresponding key input.

As glutGetModifiers() can only be called when a callback is generated, mapping modifier keys (SHIFT, CTRL, ALT) 
to actual inputs is a bit tricky. Instead, I've given SHIFT/CTRL/ALT alternate features to modify the scene.

- Pressing SHIFT + any key will toggle key lighting

- Pressing ALT + any key will toggle a backlight for the keys.

- Pressing CTRL + any key will toggle labels for the keys.

- CAPSLOCK, while being a modifier key, does not have functionality. While CAPSLOCK is enabled,
GLUT_ACTIVE_SHIFT (the modifier key state when SHIFT is pressed) *should* be returned. However, CAPSLOCK
does not appear to be returning this modifier state on my machine. Documentation glutGetModifers() can be found [here](https://www.opengl.org/resources/libraries/glut/spec3/node73.html) 

Screenshots of the program:

![program-screenshot1](https://cdn.discordapp.com/attachments/330167370159226891/917987536327049226/unknown.png?raw=true "Program Screenshot")
![program-screenshot2](https://cdn.discordapp.com/attachments/330167370159226891/917987613091201114/unknown.png?raw=true " ")
![program-screenshot3](https://cdn.discordapp.com/attachments/330167370159226891/917987667898163210/unknown.png?raw=true " ")
![program-screenshot4](https://cdn.discordapp.com/attachments/330167370159226891/917987746096742430/unknown.png?raw=true " ")

***GETTING THIS TO WORK***

I do not include all the files needed to run this program.

A visual studio solution with all needed additional files can be found here:

http://web.engr.oregonstate.edu/~mjb/cs450/Sample2019.zip

Copy all of the files in this repo into the SampleFreeGlut2019 folder.

glslprogram.cpp will needed to be added as a source file in VS
(Right click 'Source Files' -> Add -> Existing Item)
