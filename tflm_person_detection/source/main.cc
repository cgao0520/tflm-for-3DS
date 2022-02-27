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

#include "main_functions.h"
#include "model_settings.h"
#include "image_provider.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

#define WAIT_TIMEOUT 300000000ULL

#define BUF_SIZE (WIDTH * HEIGHT * 2)

static u8* cam_buf = NULL;

void writeToScreen(void *fb, uint8_t* img)
{
  int i, j, draw_x, v;
  u8 *fb_8 = (u8*) fb;
  int k = 0;
  uint8_t data;

  for (j = 0; j < kNumRows; j++)
  {
    for (i = 0; i < kNumCols; i++)
    {
      draw_x = (i << 8) - (i << 4) + 239; // x * 240 + 239
      v = draw_x - j;
      v = (v << 1) + v; // v = v*3, as every pixel has 3 bytes

      data = img[k];

			fb_8[v]   = data;
			fb_8[v+1] = data;
			fb_8[v+2] = data;
      k++;
    }
  }
}
////////////

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height) {
	u8 *fb_8 = (u8*) fb;
	u16 *img_16 = (u16*) img;
	int i, j, draw_x;
  u32 v;
	for(j = 0; j < height; j++)
  {
		for(i = 0; i < width; i++)
    {
      draw_x = (i << 8) - (i << 4) + 239; // x * 240 + 239
      v = draw_x - j;
      v = (v << 1) + v; // v = v*3, as every pixel has 3 bytes
			u16 data = img_16[j * width + i];
			uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = (data & 0x1F) << 3;
			fb_8[v] = r;
			fb_8[v+1] = g;
			fb_8[v+2] = b;
		}
	}
}

void takePicture(u8 *buf)
{
	u32 bufSize;
	CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT);
  //printf(">>> bufSize=%lu\n", bufSize);
	CAMU_SetTransferBytes(PORT_CAM1, bufSize, WIDTH, HEIGHT);

	CAMU_Activate(SELECT_OUT1);

	Handle camReceiveEvent = 0;

	CAMU_ClearBuffer(PORT_CAM1);

	CAMU_StartCapture(PORT_CAM1);

	CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, BUF_SIZE, (s16)bufSize);

	svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT);

	CAMU_StopCapture(PORT_CAM1);

	svcCloseHandle(camReceiveEvent);

	CAMU_Activate(SELECT_NONE);
}

// This is the default main used on systems that have the standard C entry
// point. Other devices (for example FreeRTOS or ESP32) that have different
// requirements for entry code (like an app_main function) should specialize
// this main.cc file in a target-specific subfolder.
int main(int argc, char *argv[])
{
  setup();

  // Initializations
  acInit();
  gfxInitDefault();
  consoleInit(GFX_BOTTOM, NULL);

  // Enable double buffering to remove screen tearing
  gfxSetDoubleBuffering(GFX_TOP, true);
  gfxSetDoubleBuffering(GFX_BOTTOM, false);

  printf("Initializing camera\n");

  printf("camInit: 0x%08X\n", (unsigned int)camInit());

  printf("CAMU_SetSize: 0x%08X\n", (unsigned int)CAMU_SetSize(/*SELECT_OUT1_OUT2*/SELECT_OUT1, /*SIZE_CTR_TOP_LCD*/SIZE_QQVGA, CONTEXT_A));
  printf("CAMU_SetOutputFormat: 0x%08X\n", (unsigned int)CAMU_SetOutputFormat(/*SELECT_OUT1_OUT2*/SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A));

  printf("CAMU_SetNoiseFilter: 0x%08X\n", (unsigned int)CAMU_SetNoiseFilter(/*SELECT_OUT1_OUT2*/SELECT_OUT1, true));
  printf("CAMU_SetAutoExposure: 0x%08X\n", (unsigned int)CAMU_SetAutoExposure(/*SELECT_OUT1_OUT2*/SELECT_OUT1, true));
  printf("CAMU_SetAutoWhiteBalance: 0x%08X\n", (unsigned int)CAMU_SetAutoWhiteBalance(/*SELECT_OUT1_OUT2*/SELECT_OUT1, true));
  // printf("CAMU_SetEffect: 0x%08X\n", (unsigned int) CAMU_SetEffect(/*SELECT_OUT1_OUT2*/SELECT_OUT1, EFFECT_MONO, CONTEXT_A));

  printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int)CAMU_SetTrimming(PORT_CAM1, false));
  //printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int)CAMU_SetTrimming(PORT_CAM2, false));
  // printf("CAMU_SetTrimmingParamsCenter: 0x%08X\n", (unsigned int) CAMU_SetTrimmingParamsCenter(PORT_CAM1, 512, 240, 512, 384));

  cam_buf = (u8*)malloc(BUF_SIZE);
  if (!cam_buf)
  {
    printf("Failed to allocate memory!\n");
  }

  while (/*true*/ aptMainLoop())
  {
    // Read which buttons are currently pressed or not
    hidScanInput();

    // If START button is pressed, break loop and quit
    if (hidKeysDown() & KEY_START)
    {
      break;
    }

    // Step 1, take a picutre from camera in a resolution of 160 * 120
    takePicture(cam_buf);

    // Step 2, preprocess the picture to 96*96 grayscale one
    preprocess(cam_buf);

    // Step 3, do a inference
    loop();

    // Step 4 (optional), draw the preprocessed image onto the top screen
    writeToScreen(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), getGrayImg());

    // Flush and swap framebuffers
    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();
  }

  free(cam_buf);

  camExit();
	gfxExit();
	acExit();

  return 0;
}
