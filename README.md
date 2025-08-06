**JCWM - John Connection Window Manager**

_Current Stage: Alpha_
_Current Version: 1.2_05_

**_CONTACT ME:_**

**FOR FEATURE REQUESTS, BUGS, HELP**

  cabron__ on Discord
  
  johnconnection@keemail.me

**_What is JCWM?_**

  JCWM (short for John Connection Window Manager), is a minimal window manager written in Xlib and C. It is the result of countless attempts at
  writing a window manager. First attempts were very janky and fell apart relatively early. Therefore I am proud to announce that this WM is atleast somewhat
  usasble. For this attempt I heavily looked towards dwm as a way to understand how things are supposed to be done. So it is partly based on and heavily inspired
  by dwm, from a code standpoint. 

**_What can it do?_**

  The Window goes into a Window Manager that manages the window into a managed window. It draws a border around the window and leaves a little space at the top to 
  draw the window's title. Additionally, JCWM comes with jcbar, a very, **VERY**, rudimentary bar, primarily designed to tell the time, and be able to unminimize minimized
  windows. **NOTE:** jcbar was hacked together in a day, so it's not a good bar by any means, but just like the WM, it's going to get better.
  

**_Controls_**

  No matter how hard I tried, I always failed at implementing keybinds. Either they'd only work for some Windows, or not at all. For the sake
  of my own sanity, I decided to go with a purely Modifier-plus-Mouse-button approach.
  
  ALT+LMB - Move Window
  
  ALT+MMB - Minimize Window
  
  ALT+RMB - Resize Window
  
  CTRL+LMB - Close Window
  
  CTRL+MMB - Maximize Window
  
  CTRL+RMB - Fullscreen Window
  
  SHIFT+LMB (on BG) - Bring up launcher (**NOTE: THIS IS gmrun BY DEFAULT. TO CHANGE, CHANGE IT IN commons.h AND RECOMPILE**)
  
  CTRL+LMB (on BG) - Exit WM
  
  **NOTE:** If you use a mouse with two buttons, you will have to change the source code to use some other ModKey+MB Combo for those actions, then recompile
  
  **NOTE 2:** These actions only work if you do them on the frame of the window (titlebar + border)

**_Setup_**

Debian:

  sudo apt install libx11-dev
  
  sudo apt install libxcursor-dev
  
  sudo apt install libxrandr-dev

  sudo apt install alttab
  
**If you're using any other distros, just figure out what your equivalent package would be for these three and install that.**

To install:

  git clone https://github.com/JohnConnection/jcwm
  
  cd jcwm
  
  sudo make install

To use: 

  Add jcwm to your .Xsession file located in your home folder

**_Config_**

  At this moment, the WM is configured by changing the source code, mainly the Macros in the main.c file. If you want to go further, you may dig into the source code
  itself to change the WM's behaviour. Macros mainly affect Design.

**_TO-DO_**

  + Add Snapping to screen edge
  + Improve Multimonitor support
  + Rewrite jcbar
  + Add Alt+Tab
  + Fix window size issues (XTerm, Firefox, e.g)
  + Improve handling of transient windows
  + Add multiple Desktops

**_TESTING AND CONTRIBUTING_**

  As you can see, I have included a file called Xephyr.sh with the WM. If you run it, it will open a nested X Server. This is a closed environment that allows for you    to
  test the wm and try to use it without having to change your whole setup. If you wish to run something in said X Server, do this:
  
  DISPLAY=:1 [THING] &




