#!/usr/bin/env python3
"""
This software is the main script of the DrawingCNC project.

It takes a picture from the camera, process it and returns an EPS file ready to
be used with the CNC milling machine.

Depends on the subprogram take_calibrated_picture, imagemagick and potrace
"""
import subprocess
import sys


def getCalibratedJPEG():
    """
    Get a calibrated black and white image from the camera, ready to be
    processed by potrace.

    Returns the image as raw JPEG data.
    """
    try:
        calibrated = subprocess.Popen(
            "take_calibrated_picture/take_calibrated_picture",
            stdout=subprocess.PIPE)
        output = calibrated.communicate()[0]
    except FileNotFoundError:
        sys.exit("Please build the take_calibrated_picture script first.")
    return output


def jpegToEPS(jpg_data):
    """
    Convert a jpeg image to an eps one, using potrace.

    Params:
        - jpeg_data is the raw jpeg data to process.

    Returns the raw processed EPS data.
    """
    convert = subprocess.Popen("convert -:jpg pnm:-",
                               stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE)
    potrace = subprocess.Popen("potrace",
                               stdin=convert.stdout,
                               stdout=subprocess.PIPE)
    convert.stdout.close()
    output = potrace.communicate()[0]
    convert.wait()
    return output


def getEPSDrawing():
    """
    Get the drawing as EPS plans, from the camera.

    Returns the raw eps data.
    """
    jpeg_data = getCalibratedJPEG()
    eps_data = jpegToEPS(jpeg_data)
    return eps_data


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("Usage: " + sys.argv[0] + " OUTPUT_FILENAME.eps.")

    # Fetch the EPS data and write them to file
    eps_data = getEPSDrawing()
    with open(sys.argv[1], "w", encoding="utf-8") as fh:
        fh.write(eps_data)
