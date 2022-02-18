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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>
#include "main_functions.h"

u32 micbuf_pos = 0;
u32 micbuf_datasize;
u32 audiobuf_size = 0x100000;

u32 audiobuf_pos = 0;
u32 audiobuf_overflow_count = 0;
u8 *micbuf;
u8 *audiobuf;

static bool initialized = true;

void MicInit()
{
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    u32 micbuf_size = 0x30000; // 192kB = 6 seconds in 16bit 16KHz sampling

    micbuf = (u8 *)memalign(0x1000, micbuf_size);

    printf("Initializing CSND...\n");
    if (R_FAILED(csndInit()))
    {
        initialized = false;
        printf("Could not initialize CSND.\n");
    }
    else
    {
        printf("CSND initialized.\n");
    }

    printf("Initializing MIC...\n");
    if (R_FAILED(micInit(micbuf, micbuf_size)))
    {
        initialized = false;
        printf("Could not initialize MIC.\n");
    }
    else
    {
        printf("MIC initialized.\n");
    }

    micbuf_datasize = micGetSampleDataSize();
    printf("micbuf_datasize=%lu\n", micbuf_datasize);

    audiobuf = (u8 *)linearAlloc(audiobuf_size);

    if (R_SUCCEEDED(MICU_StartSampling(MICU_ENCODING_PCM16_SIGNED, MICU_SAMPLE_RATE_16360, 0, micbuf_datasize, true)))
    {
        printf("Now recording.\n");
    }
    else
    {
        printf("Failed to start sampling.\n");
    }
}

void MicExit()
{
    linearFree(audiobuf);

    micExit();
    free(micbuf);

    csndExit();
    gfxExit();
}

// This is the default main used on systems that have the standard C entry
// point. Other devices (for example FreeRTOS or ESP32) that have different
// requirements for entry code (like an app_main function) should specialize
// this main.cc file in a target-specific subfolder.
int main(int argc, char *argv[])
{
    MicInit();

    setup();
    while (/*aptMainLoop()*/ true)
    {
        hidScanInput();
        if (hidKeysDown() & KEY_START)
        {
            break; // break in order to return to hbmenu
        }

        if (initialized)
        {
            u32 micbuf_readpos = micbuf_pos;
            micbuf_pos = micGetLastSampleOffset();
            while (audiobuf_pos < audiobuf_size && micbuf_readpos != micbuf_pos)
            {
                audiobuf[audiobuf_pos] = micbuf[micbuf_readpos];
                audiobuf_pos++;
                micbuf_readpos = (micbuf_readpos + 1) % micbuf_datasize;
            }

            if (audiobuf_pos >= audiobuf_size)
            {
                printf("[audio buffer is rounded, %lu]\n", ++audiobuf_overflow_count);

                audiobuf_pos = 0;
                while (micbuf_readpos != micbuf_pos)
                {
                    audiobuf[audiobuf_pos] = micbuf[micbuf_readpos];
                    audiobuf_pos++;
                    micbuf_readpos = (micbuf_readpos + 1) % micbuf_datasize;
                }
            }

            loop();
        }
/*
        gfxFlushBuffers();
        gfxSwapBuffers();*/
    }

    MicExit();

    return 0;
}
