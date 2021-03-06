/*
 TiLDA Mk2

 SponsorsApp
 Displays a list of all sponsors

 The MIT License (MIT)

 Copyright (c) 2014 Electromagnetic Field LTD

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "EMF2014Config.h"
#include "App.h"

class SponsorsApp: public App {
public:
    static App* New();

    String getName() const;
private:
    SponsorsApp();
    SponsorsApp(const SponsorsApp&);

    bool keepAlive() const;

    void task();
    void afterSuspension();
    void beforeResume();
    void _draw();
    uint8_t _page;
private:
};

static const uint8_t SPONSORS_1_XBM[] = {
  128, // width
  64, // height

  /* page 0 (lines 0-7) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,
  0x1f,0x7e,0xf8,0xe7,0xfe,0x38,0x7f,0xce,0xdf,0x93,0x26,0xec,0xf9,0xde,0x96,0xa4,
  0xa4,0xa8,0x0,0x0,0x0,0x6e,0x6e,0x6e,0x24,0x24,0x3c,0x64,0x64,0x7c,0x3e,0x3e,
  0x0,0x0,0x0,0x8e,0x8e,0x8c,0x4,0xc,0x3c,0x24,0x24,0xbe,0xbe,0x9c,0x0,0x0,
  0x0,0x0,0x0,0x0,0xf0,0xf8,0x7c,0x3e,0xe,0xf,0x7,0x7,0xf,0xe,0x3e,0xfc,
  0xf8,0xf0,0x0,0x0,0xa8,0x54,0xea,0x15,0xd,0xb,0xd,0xb,0x15,0xea,0x54,0xa8,
  0xf0,0x0,0x0,0x6,0x4,0x3c,0x38,0x38,0x38,0xf8,0xf8,0xfc,0xfc,0xfe,0xfe,0xfe,
  0xff,0xfe,0x3e,0x1f,0x1f,0x3e,0x3e,0x3f,0x3f,0x23,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 1 (lines 8-15) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe0,0xe0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
  0x60,0x60,0x60,0x61,0x63,0x67,0x6e,0xe,0x1d,0x39,0x3f,0x33,0x76,0x76,0x64,0x64,
  0x6d,0x6d,0x0,0x0,0x0,0xce,0xdf,0xdf,0x9b,0x9b,0x9b,0x9b,0x9b,0xdf,0xdf,0xce,
  0x0,0x0,0x0,0x5b,0x5b,0x59,0xd1,0xd1,0xf1,0xf1,0xb9,0x3b,0x3b,0x39,0x0,0x0,
  0x0,0x0,0x0,0x0,0x3,0x3,0x0,0x18,0x1c,0x3c,0x3e,0x3e,0x3e,0x1c,0x1c,0x8,
  0x3,0x3,0x0,0x0,0xbb,0x55,0xab,0xe5,0xa7,0xf5,0xaa,0x1c,0x0,0x1,0x3,0x4,
  0x3,0x0,0x0,0xfc,0xfc,0xfc,0xfc,0xfe,0xff,0xff,0xff,0xff,0x7f,0x3f,0x1f,0xf,
  0x3,0x0,0x0,0xce,0xda,0xd8,0xde,0xc2,0xde,0xc,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 2 (lines 16-23) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7f,0x7f,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0xec,0xec,0xee,0x4e,0x4e,0x4a,0x4b,0x49,0xcd,0xed,0xed,
  0x0,0x0,0x60,0xf7,0xf7,0xf3,0x92,0x92,0x92,0x93,0xb3,0xf7,0xf7,0x63,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20,0x78,0x4c,0xc,0x6,0xc,0xc,0x7c,0x70,
  0x0,0x0,0x0,0x0,0xbf,0xf5,0x6f,0xf9,0x69,0xfe,0xab,0x3f,0x28,0x38,0x28,0x38,
  0x0,0x0,0x0,0x3f,0x3,0x3,0x1,0x1,0x1,0x1,0x0,0x80,0x0,0x0,0x80,0x80,
  0x0,0x0,0x0,0x34,0xe4,0xe4,0x67,0xc4,0xc4,0x84,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 3 (lines 24-31) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0xc,0xc,0xc,0x1c,0xfc,0xfc,0x8c,0xc,
  0xc,0xc,0xc,0xc,0x9c,0xfc,0xfc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
  0x40,0x40,0x40,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x40,0x40,0x40,0x40,0x40,0x40,
  0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x8,0x3e,0x67,0x63,0xc3,0x63,0x67,0x3e,0x1c,
  0x0,0x0,0x0,0x0,0xff,0x0,0xff,0x15,0x29,0x73,0x9e,0x73,0xc,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2,0xc7,0xcc,0xc7,0xc3,0x65,
  0x27,0x0,0x0,0x3b,0x7b,0xc9,0xc9,0xc9,0x78,0x38,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 4 (lines 32-39) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x80,0x80,0x80,0x80,0x81,0x87,0x87,0x8e,
  0x8e,0x8e,0x8e,0x8e,0x87,0x87,0x81,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x60,0xc0,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
  0xc0,0xe0,0xf0,0xf0,0xf8,0xf8,0xf8,0xf8,0xf8,0xf0,0xe0,0xc0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x66,0x66,0x66,0x66,0x66,0xe6,0xfe,0x18,
  0x0,0x0,0x0,0x0,0x77,0x55,0x77,0x55,0xf7,0xd5,0x3f,0x81,0x7f,0x55,0x77,0x55,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x30,0x78,0x40,0x70,0x39,0x59,
  0x73,0x0,0x0,0x1c,0x36,0x22,0xa2,0xb2,0xbe,0x8c,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 5 (lines 40-47) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x3,0x3,0x83,0xc3,0xe3,0xe3,0x73,0x7b,0xe3,
  0xc3,0x83,0x83,0xc3,0xe3,0xff,0x7f,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x30,0xf1,0xff,0xfe,0xfe,0x7f,0x1f,0xf,0xf,0xff,0xfb,0x1,0x1,
  0x3,0x31,0x1,0x1,0x83,0xff,0xff,0xff,0x7f,0x3f,0x3f,0x3f,0x3f,0xf8,0xf0,0xc0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xf7,0xe3,
  0xc0,0x0,0x0,0x0,0x55,0x77,0x55,0x75,0x57,0x74,0x52,0x71,0x0,0xc0,0xb0,0x90,
  0x60,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x84,0xc4,0xc4,0xc4,0xc4,0xde,
  0xdf,0x0,0x0,0x11,0x9b,0xbb,0xbf,0xbf,0xa4,0x24,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 6 (lines 48-55) */
  0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0xee,0xef,0x7,0x3,0x1,0x0,0x0,0x0,0x80,
  0xc1,0xc1,0xe3,0xc1,0xc1,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x6,0x1e,0x3f,0x7f,0xff,0xff,0xfe,0xf8,0x98,0x8f,0x7,0x0,0x0,
  0x0,0xc,0x0,0x0,0x80,0xff,0xff,0xff,0xfc,0xf8,0xfa,0xf8,0x38,0x3e,0x1f,0x7,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x60,0xfb,0x1c,0xc,0xfc,0xfc,0xcc,0xf9,0x70,
  0x0,0x0,0x0,0x0,0xe5,0xa7,0xe5,0xa7,0xdd,0xa7,0x7d,0x7,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x3,0xd3,0xd0,0xf3,0xb0,0xb0,
  0x33,0x0,0x0,0x27,0xed,0xe0,0x40,0xc1,0xcf,0x82,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 7 (lines 56-63) */
  0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1f,0x3f,0x78,0x70,0x70,0x70,0x7c,0x3f,0x1f,
  0x7,0x1,0x0,0x1,0x3,0x7f,0x7f,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x3,0x7,0x7,0xf,0xf,0xf,0xf,
  0x1f,0x3f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x3f,0x3f,0x1f,0x7,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20,0x78,0x5c,0xc,0xd,0xd,0xd,0x78,0x70,
  0x0,0x0,0x0,0x0,0xca,0xee,0x7a,0x3e,0x1a,0x3e,0x7a,0xee,0xca,0xe,0xa,0xe,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x30,0x7a,0xc2,0x73,0x3b,0x5b,
  0x73,0x0,0x0,0x22,0x23,0x23,0x21,0x21,0xf9,0x78,0x0,0x0,0x0,0x0,0x0,0x0, };

static const uint8_t SPONSORS_2_XBM[] = {
  128, // width
  64, // height

  /* page 0 (lines 0-7) */
  0x0,0x0,0x0,0x8b,0x8b,0x8b,0x8b,0xdb,0x7b,0x3,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0xc0,0x70,0x10,0x18,0x8,0xc,0xc,0x8c,0xcc,0xc8,0x18,0x30,0xe0,0x80,
  0x0,0x0,0x80,0xf0,0xf8,0x3c,0x1c,0xc,0xc,0x1c,0x1c,0x78,0xf0,0xe0,0x0,0x40,
  0xe0,0xe0,0xf0,0xdc,0xdf,0x9c,0x70,0xe0,0xc0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,
  0x7,0xf,0xf,0xf,0xfe,0xfe,0xfe,0x1e,0x1e,0xfc,0xfc,0xfc,0x0,0x0,0x0,0x7c,
  0x4,0x4,0x7e,0xc4,0xc4,0x7c,0x0,0x0,0x0,0x0,0x0,0x0,0x40,0xf8,0xfc,0xfe,
  0xfe,0xff,0xee,0xc6,0xce,0xee,0xe8,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xe0,0x80,

  /* page 1 (lines 8-15) */
  0x0,0x0,0x0,0xc8,0x68,0x68,0x68,0x4d,0x87,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x80,0x80,0xc0,0xc0,0xc0,0xc0,0x80,0x80,0x0,0x0,0x0,0x0,
  0x0,0xf,0x3f,0x60,0xc0,0x98,0x9c,0x1c,0x0,0x1,0x1,0x81,0x80,0xe0,0x78,0x1f,
  0x0,0x0,0x47,0x6f,0x6f,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x6f,0x6f,0x0,0x0,
  0x0,0x1,0x33,0x1f,0x1f,0x1f,0xf,0xe,0xe,0x6,0x5,0x3,0x0,0x0,0x0,0x0,
  0xf8,0xf8,0x38,0x78,0xf1,0xe7,0xf,0xef,0xef,0x8f,0xf,0x7,0x0,0x0,0x0,0xe2,
  0x12,0x12,0xf2,0x12,0x12,0xe2,0x2,0x2,0x0,0x0,0xe0,0xf0,0xf8,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfb,0xfb,0xf9,0xff,0xff,0x7f,0x7f,0x3f,0xf,

  /* page 2 (lines 16-23) */
  0xc0,0x60,0x0,0xcf,0x2c,0x2c,0x2c,0x6c,0xcf,0x0,0x0,0x0,0x0,0x0,0x0,0xe0,
  0xf8,0x1e,0x7,0x3,0x1,0x1,0x0,0x0,0x0,0x0,0x1,0x1,0x3,0x7,0x1e,0xf8,
  0xf0,0x0,0x0,0x0,0x0,0xc1,0xc1,0xc1,0xc1,0xc1,0xe1,0xe1,0xc0,0xc0,0x0,0x0,
  0x0,0x0,0xfe,0xff,0xff,0x0,0x0,0xfc,0xfe,0xfe,0x7,0xf,0xfe,0xfc,0x0,0x0,
  0x0,0x0,0xc3,0xdb,0xda,0xee,0xe6,0xc6,0xc6,0x82,0x0,0x0,0xc0,0xc0,0xc0,0x80,
  0x8f,0x8f,0x8f,0x9e,0xdf,0xdf,0xde,0xdf,0xdf,0xc7,0x0,0x0,0x0,0x0,0x0,0x11,
  0x10,0x10,0x13,0x12,0x12,0xf3,0x0,0x0,0x0,0x0,0xf,0x3f,0x7f,0x7f,0xff,0xff,
  0xff,0xff,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf0,0xc0,0x0,0x0,0x0,0x0,

  /* page 3 (lines 24-31) */
  0x3,0x6,0x6,0xb7,0xb4,0xb4,0xb4,0xb6,0xa7,0x0,0x0,0x0,0xc0,0xc0,0xc0,0xc1,
  0xcf,0xde,0xd8,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x60,0x60,0x30,0x1c,0xf,
  0x3,0x0,0x0,0x0,0x1,0xe3,0xe3,0xf1,0xf1,0xb1,0xbf,0x1f,0x1,0x0,0x0,0x0,
  0x0,0x0,0xe0,0xf3,0xf7,0x76,0x76,0xf7,0xf3,0xf1,0x70,0x70,0xf7,0xf7,0x0,0x0,
  0x0,0x0,0xde,0x5a,0x5e,0x58,0xde,0x40,0x0,0x0,0x0,0x0,0x1,0x3,0x7,0x7,
  0xf,0xf,0x1f,0x3f,0x3f,0x7b,0x7b,0xf1,0xf1,0xc0,0x0,0x0,0x0,0x0,0x0,0x22,
  0x22,0x22,0x22,0x22,0x23,0x3f,0x0,0x0,0x0,0x0,0x0,0xe4,0x24,0xe4,0xc,0xdc,
  0x0,0x0,0x0,0x0,0x0,0x38,0x3e,0x3f,0x3f,0x1f,0xf,0x3,0x0,0x0,0x0,0x0,

  /* page 4 (lines 32-39) */
  0x0,0x0,0x0,0xc8,0x48,0x78,0x68,0xc9,0x8f,0x0,0x0,0x0,0xff,0x1,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfe,0xfe,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x9c,0x9c,0x9f,0xbf,0xbf,0xb3,0xf3,0x0,0x80,0x0,0x0,
  0x0,0x0,0x3f,0x7f,0x7f,0x0,0x0,0x1f,0x1f,0xf,0x0,0x0,0x7f,0x7f,0x0,0x0,
  0x0,0x0,0x92,0x92,0xd2,0xd2,0x5e,0x42,0x62,0x62,0x0,0x0,0x0,0x0,0x0,0x0,
  0x7,0xf,0xf,0xf,0xfe,0xfe,0xde,0x1e,0x1e,0xfc,0xfc,0xfc,0x0,0x0,0x0,0x1e,
  0x1,0x1,0x3f,0x21,0x21,0x3e,0x0,0x0,0x0,0x38,0x60,0x79,0x47,0x47,0x45,0xb9,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 5 (lines 40-47) */
  0x0,0x0,0x0,0x4f,0x4c,0x48,0x48,0x6c,0xef,0x40,0x40,0x0,0x7f,0x70,0x60,0x60,
  0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7f,0x7f,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0x38,0xbb,0x0,0x0,
  0x0,0x0,0xe0,0xfc,0xfe,0xf,0x7,0x3,0x3,0x3,0x7,0x1f,0xfe,0xf8,0x0,0x0,
  0x0,0x0,0xc8,0x48,0x4d,0x5d,0xf6,0x26,0x22,0x20,0x0,0x0,0x0,0x0,0x0,0x0,
  0xf8,0xf8,0x38,0x78,0xf1,0xe7,0xf,0xef,0xef,0x8f,0xf,0x7,0x0,0x0,0x0,0x21,
  0x21,0x21,0x21,0x21,0x21,0x9f,0x0,0x0,0x0,0x0,0x0,0x9a,0xdb,0x5f,0x6d,0xa4,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 6 (lines 48-55) */
  0x0,0x0,0x0,0xf2,0x1a,0xa,0x1a,0x12,0xe2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x40,0xf3,0xfb,0x1b,0x1b,0x3b,0xf3,0xe3,0x0,0x3,0x0,0x0,
  0x0,0x0,0x1,0xe3,0xf3,0x78,0x38,0x18,0x18,0x18,0x38,0xf0,0xf3,0xc3,0x0,0x0,
  0x0,0x0,0x88,0x8b,0x8a,0x8a,0xbb,0x80,0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,
  0xef,0xef,0xef,0xde,0xdf,0xdf,0xde,0x9f,0x9f,0x87,0x80,0x0,0x0,0x0,0x0,0x23,
  0x21,0x21,0x21,0x21,0x21,0x23,0x1,0x1,0x0,0x0,0x0,0x93,0x94,0x94,0x94,0xb3,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

  /* page 7 (lines 56-63) */
  0x0,0x0,0x0,0x89,0x89,0x8f,0x8f,0xc9,0x78,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x3,0x3,0x7,0x7,0x7,0x3,0x1,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0xf,0x1f,0x3c,0x38,0x70,0x70,0x70,0x38,0x3c,0x1f,0x7,0x0,0x0,
  0x0,0x0,0x3e,0x62,0x33,0x19,0x3a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x1,0x1,0x1,0x1,0x1,0x3,0x3,0x3,0x3,0x7,0xf7,0xf7,0x0,0x0,0x0,0x10,
  0x3c,0x12,0x11,0x11,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x44,0x44,0x44,0x4c,0x3f,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, };
