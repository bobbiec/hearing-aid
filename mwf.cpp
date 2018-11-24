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

#include <Arduino.h>
#include "mwf.h"
#include "sqrt_integer.h"
#include "utility/dspinst.h"
#include "arm_const_structs.h"


// 140312 - PAH - slightly faster copy
static void copy_to_fft_buffer(void *destination, const void *source)
{
	const uint16_t *src = (const uint16_t *)source;
	uint32_t *dst = (uint32_t *)destination;

	for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
		*dst++ = *src++;  // real sample plus a zero for imaginary
	}
}

static void apply_window_to_fft_buffer(void *buffer, const void *window)
{
	int16_t *buf = (int16_t *)buffer;
	const int16_t *win = (int16_t *)window;;

	for (int i=0; i < 256; i++) {
		int32_t val = *buf * *win++;
		//*buf = signed_saturate_rshift(val, 16, 15);
		*buf = val >> 15;
		buf += 2;
	}
}

static void do_fft(int16_t *buffer, int16_t *block, int16_t *prevblock) {
	copy_to_fft_buffer(buffer, prevblock->data);
	copy_to_fft_buffer(buffer+256, block->data);

	if (window) apply_window_to_fft_buffer(buffer, window);
	arm_cfft_q15(arm_cfft_sR_q15_len256, buffer, 0, 0);
}

void AudioAnalyzeFFT256::update(void)
{
	audio_block_t *block[4];


	block[0] = receiveWritable(0);
	if (!block) {
		return;
	}

	block[1] = receiveWritable(1);
	block[2] = receiveWritable(2);
	block[3] = receiveWritable(3);

	if (!prevblock[0]) {
		prevblock[0] = block[0];
		prevblock[1] = block[1];
		prevblock[2] = block[2];
		prevblock[3] = block[3];
		return;
	}

	for (int i=0; i<4; i++) {
		do_fft(buffer[i], block[i], prevblock[i]);
	}

	// Compute the short term energy and magnitude of main channel
	short_term_energy = 0; // could use arm_power_q31 here
	                       // but would duplicate the looping work
	for (int i=0; i < 128; i++) {
		uint32_t tmp = *((uint32_t *)buffer[0] + i);
		uint32_t magnitude_squared = multiply_16tx16t_add_16bx16b(tmp, tmp);
		short_term_energy += magnitude_squared
		magnitudes[i] = sqrt_uint32_approx(magnitude_squared)
	}

	// Compute the max bin frequency
	// TODO: look up bin index -> frequency conversion
	int16_t max_value = 0;
	int16_t max_index = 0;
	arm_max_q15(magnitudes, 128, &max_value, &max_index);

	// Compute the spectral flatness measure
	uint32_t total = 0;
	arm_mean_q31(magnitudes, 128, &total);
	uint32_t arithmetic_mean = total / 128;
	float ftotal = 0; // use the log-average method
	for (int i=0; i < 128; i++) {
		ftotal += log2(float(magnitudes[i]))
	}
	uint32_t geometric_mean = 2 << (ftotal >> 7); // 2^(1/128 * ftotal)
	spectral_flatness = 10 * log10(geometric_mean / arithmetic_mean);

	// 3-4
	thresh_e = energy_primthresh * log(min_e)
	thresh_f = f_primthresh
	thresh_sf = sf_primthresh

	int16_t counter = 0;
	if (short_term_energy - e > thresh_e) {
		counter++;
	}
	if (max_index - min_f > thresh_f) {
		// TODO: convert indexes to frequencies
		counter++;
	}
	if (spectral_flatness - min_sf > thresh_sf) {
		counter++;
	}

	vad_decision = false;
	if (counter > 1) {
		vad_decision = true;
	} else {
		// TODO: update the energy minimum value
	}

	// TODO: Implement 4 and 5 ("streak") as "unsure"

	// TODO: Write the MWF logic
	if (vad_decision) {
		// add to speech buffer
		memcpy(speech_buf[sb_write_i], buffer[0], 512);
		sb_write_i += MCLT_FRAME
		if (sb_write_i > 10240) {
			sb_write_i -= 10240;
		}

		// generate extra noisy stuff
		for (int i=0; i<4; i++) {
			arm_add_q15(noise_buf[nb_read_i], buffer[i], 512);
		}
		nb_read_i += MCLT_FRAME;
		if (nb_read_i > 10240) {
			nb_write_i -= 10240;
		}
	} else {
		// add to noise buffer
		memcpy(noise_buf[nb_write_i], buffer[0], 512);
		nb_write_i += MCLT_FRAME
		if (nb_write_i > 10240) {
			nb_write_i -= 10240;
		}

		// generate extra speechy stuff
		for (int i=0; i<4; i++) {
			arm_add_q15(speech_buf[sb_read_i], buffer[i], 512);
		}

		sb_read_i += MCLT_FRAME;
		if (sb_read_i > 10240) {
			sb_read_i -= 10240;
		}
	}

	for (int i=0; i<4; i++) {
		// output_buffer and error_buffer are actually not used for this one
		arm_lms_q15(&lms_generated, buffer[i], buffer[0], output_buffer[i], error_buffer[i], 512);
	}

	// copy filter coefficients
	lms_main::pCoeffs = lms_generated::pCoeffs;

	for (int i=0; i<4; i++) {
		// error_buffer is unused
		arm_lms_q15(&lms_main, buffer[i], buffer[0], output_buffer[i], error_buffer[i], 512);
	}

	for (int i=0; i<4; i++) {
		// IFFT
		arm_cfft_q15(arm_cfft_sR_q15_len256, output_buffer, 1, 0);
	}

	release(prevblock);
	prevblock = block;
}


