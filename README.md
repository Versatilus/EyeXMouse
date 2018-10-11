# EyeXMouse

This is a fork of a simple adaptation of the SDK sample code released by Tobii which moves the mouse pointer based on eye tracking.

I came across this program on GitHub after purchasing my eye tracker. It technically worked, but it was too imprecise and "squirrelly" for me to use effectively, especially due to issues related to my disability. I set the code aside for the better part of a year and mostly forgot about it. I decided to play with it again after remembering I had the code. After some experimentation with values for the dead zone and speeds, as well as an alternative acceleration implementation, I ended up with a program which would help me once again use the mouse for most of my navigation in GUI applications.

Features include:
* Simple filtering and averaging to keep pointer movements smooth.
* Nonlinear acceleration, allowing slow movements near the eyes' focus for more precise control but allowing quick movement when focus changes rapidly.
* Only updates the pointer when both eyes can be tracked. This allows reading pop-ups with minimal effort by closing one eye.

Configuration is done by directly changing the source code and recompiling. Binaries have been provided, though you should experiment with values and recompile. These numbers are just what felt right to me on my 17" 1920x1080 laptop.

I activate and deactivate this software via speech recognition using [Dragonfly](https://github.com/Danesprite/dragonfly), a simple version of which is included.

Other than a few variable names, most of the original modifications have been replaced. 

The original README is below.



EyeXMouse
=========

Mouse cursor control for the Tobii EyeX

Very crude program for controlling the mouse cursor using the Tobii EyeX development kit.

Requires Tobii EyeX Controller and Software

Useful for any Mouse Cursor games e.g. http://www.gamebase.info/magazine/read/top-10-eye-gaze-games_294.html 

Works well in The Grid 2. Set input to 'Pointer > All options > Dwell pointer over cell'


**Program DOES NOT exit gracefully (requires ctrl+alt+del) and has no options**


Purchase a Tobii EyeX development kit for £72.99 (inc. delivery) from http://www.tobii.com/en/eye-experience/buy/order-eyex



##Credit

https://github.com/gigertron/EyeTracker

http://developer.tobii.com/community/forums/topic/gaming-and-desktop-mouse-control-with-the-rex


## License
Code is under the [GPLv3 license] (https://github.com/mikethrussell/EyeXMouse/blob/master/LICENSE.txt).