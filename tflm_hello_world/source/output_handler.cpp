/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "output_handler.h"
#include "constants.h"
#include <stdlib.h>

#define CIRCLE_RADIUS_IN_PIXEL  10

extern unsigned char* frameBuffer;
unsigned char rr, gg, bb;

inline void setPixel(int x, int y)
{
	static int x_,y_,yy;
	x_ = 239 - y;
	y_ = x;

	yy=(y_<<8)-(y_<<4);
	frameBuffer[3*(x_+yy)] = bb;
	frameBuffer[3*(x_+yy)+1] = gg;
	frameBuffer[3*(x_+yy)+2] = rr;
}

void circle( int mx, int my, int r )
{
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y)
    {
        setPixel( mx + x, my + y);
        setPixel( mx - x, my + y);
        setPixel( mx - x, my - y);
        setPixel( mx + x, my - y);
        setPixel( mx + y, my + x);
        setPixel( mx - y, my + x);
        setPixel( mx - y, my - x);
        setPixel( mx + y, my - x);

        if (d < 0)
        {
            d = d + 4 * x + 6;
        }
        else
        {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void HandleOutput(tflite::ErrorReporter* error_reporter, float x_value, float y_value)
{
    // Log the current X and Y values
    TF_LITE_REPORT_ERROR(error_reporter, "x_value: %f, y_value: %f\n",
                         static_cast<double>(x_value),
                         static_cast<double>(y_value));

    // Calculate the coordicates on 3DS screen
    int x = x_value * 319 / kXrange;
    int y = 120 + y_value * (-66);

    // Generate random color for drawing circle
    rr = rand() & 0x000000ff;
    gg = rand() & 0x000000ff;
    bb = rand() & 0x000000ff;

    // Draw circle for each input/output pair
    circle(x, y, CIRCLE_RADIUS_IN_PIXEL);
}
