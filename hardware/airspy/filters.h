/*
Copyright (C) 2014, Youssef Touil <youssef@airspy.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>

#define HB_KERNEL_FLOAT_LEN 47

const float HB_KERNEL_FLOAT[HB_KERNEL_FLOAT_LEN] =
{
	-0.000998606272947510,
	 0.000000000000000000,
	 0.001695637278417295,
	 0.000000000000000000,
	-0.003054430179754289,
	 0.000000000000000000,
	 0.005055504379767936,
	 0.000000000000000000,
	-0.007901319195893647,
	 0.000000000000000000,
	 0.011873357051047719,
	 0.000000000000000000,
	-0.017411159379930066,
	 0.000000000000000000,
	 0.025304817427568772,
	 0.000000000000000000,
	-0.037225225204559217,
	 0.000000000000000000,
	 0.057533286997004301,
	 0.000000000000000000,
	-0.102327462004259350,
	 0.000000000000000000,
	 0.317034472508947400,
	 0.500000000000000000,
	 0.317034472508947400,
	 0.000000000000000000,
	-0.102327462004259350,
	 0.000000000000000000,
	 0.057533286997004301,
	 0.000000000000000000,
	-0.037225225204559217,
	 0.000000000000000000,
	 0.025304817427568772,
	 0.000000000000000000,
	-0.017411159379930066,
	 0.000000000000000000,
	 0.011873357051047719,
	 0.000000000000000000,
	-0.007901319195893647,
	 0.000000000000000000,
	 0.005055504379767936,
	 0.000000000000000000,
	-0.003054430179754289,
	 0.000000000000000000,
	 0.001695637278417295,
	 0.000000000000000000,
	-0.000998606272947510
};

#define HB_KERNEL_INT16_LEN 47

const int16_t HB_KERNEL_INT16[HB_KERNEL_INT16_LEN] =
{
	-33,
	 0,
	 56,
	 0,
	-100,
	 0,
	 166,
	 0,
	-259,
	 0,
	 389,
	 0,
	-571,
	 0,
	 829,
	 0,
	-1220,
	 0,
	 1885,
	 0,
	-3353,
	 0,
	 10389,
	 16384,
	 10389,
	 0,
	-3353,
	 0,
	 1885,
	 0,
	-1220,
	 0,
	 829,
	 0,
	-571,
	 0,
	 389,
	 0,
	-259,
	 0,
	 166,
	 0,
	-100,
	 0,
	 56,
	 0,
	-33
};

#endif // FILTERS_H