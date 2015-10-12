DrawingCNC
==========

The goal of this project is to be able to draw something on a wooden board,
with a marker, and cut it immediately using a CNC milling machine.

For the plan detection to work, you need a rectangle shape beneath your print.
You can either use a quadrilateral-shaped wooden board, or put a white paper
below your wooden board.


# What is in this repo?

There are two main codes in this repo.

First one is the central script, `drawingcnc.py` which calls the other programs
when needed.

Second one is `take_calibrated_picture` which is a C++ program using OpenCV to
take a calibrated picture (fix perspective and so on), filter it and return a
black and white image where white is wood and black is the trace of the marker.

In the future, there could be levels of gray to specify the milling depth.


# How to use?

You will need openCV, imagemagick and potrace for this to work.

* Clone this repo.
* Build the program to take calibrated pictures: `cd take_calibrated_picture; make`.
* Run the python script to get an EPS drawing.


# License

All the source code we wrote is under a beer-ware license, under otherwise specified.

    * --------------------------------------------------------------------------------
    * "THE BEER-WARE LICENSE" (Revision 42):
    * Phyks wrote this file. As long as you retain this notice you
    * can do whatever you want with this stuff (and you can also do whatever you want
    * with this stuff without retaining it, but that's not cool...). If we meet some
    * day, and you think this stuff is worth it, you can buy us a beer
    * in return.
    *                                                                       hackEns
    * ---------------------------------------------------------------------------------

If you need a more legally valid license, you can consider Disty to be under an MIT license.


# References

* http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
