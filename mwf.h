/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef analyze_fft256_h_
#define analyze_fft256_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"

// windows.c
extern "C" {
extern const int16_t AudioWindowHanning256[];
extern const int16_t AudioWindowBartlett256[];
extern const int16_t AudioWindowBlackman256[];
extern const int16_t AudioWindowFlattop256[];
extern const int16_t AudioWindowBlackmanHarris256[];
extern const int16_t AudioWindowNuttall256[];
extern const int16_t AudioWindowBlackmanNuttall256[];
extern const int16_t AudioWindowWelch256[];
extern const int16_t AudioWindowHamming256[];
extern const int16_t AudioWindowCosine256[];
extern const int16_t AudioWindowTukey256[];
}

class MWF : public AudioStream
{
public:
	MWF() : AudioStream(4, inputQueueArray),
	  		window(AudioWindowHanning256),
			count(0),
			arm_lms_init_q15(&lms_main, 64, &mainCoeffs, &mainState, 0.1, 512, 0),
			arm_lms_init_q15(&lms_generated, 64, &genCoeffs, &genState, 0.1, 512, 0); {
		prevblock = NULL;
	}

	virtual void update(void);
private:
	const int16_t *window;
	audio_block_t *prevblock[4];
	int16_t buffer[4][512] __attribute__ ((aligned (4)));

	int16_t output_buffer[4][512] __attribute__ ((aligned (4)));
	int16_t error_buffer[4][512] __attribute__ ((aligned (4)));



	uint32_t sum[128];
	uint16_t bins[128]; // FFT bins

	uint8_t count; // number of blocks processed so far
	audio_block_t *inputQueueArray[4]; // 4 inputs

	/* VAD parameters */
	// primary thresholds
	int32_t energy_primthresh
	int16_t f_primthresh
	int16_t sf_primthresh

	// minimum parameters
	int32_t min_e
	int16_t min_f
	int16_t min_sf

	// decision thresholds
	int32_t thresh_e
	int32_t thresh_f
	int16_t thresh_sf

	// VAD inputs
	int32_t short_term_energy;
	int16_t spectral_flatness;
	int16_t max_bin;

	bool vad_decision;

	/* end VAD parameters */

	/* MWF things */
	// Use P = 1

	static const int16_t MCLT_FRAME = 512; // same as size of buffer

	int16_t speech_buf[10240];
	int16_t sb_read_i;
	int16_t sb_write_i;

	int16_t noise_buf[10240];
	int16_t nb_read_i;
	int16_t nb_write_i;

	int16_t mainCoeffs[64];
	int16_t mainState[575]; // numTaps + buffer size - 1 = 64 + 512 - 1
 	arm_lms_instance_q15 lms_main;

	int16_t genCoeffs[64];
	int16_t genState[575]; // numTaps + buffer size - 1 = 64 + 512 - 1
 	arm_lms_instance_q15 lms_generated;




	/* end MWF things */
};
