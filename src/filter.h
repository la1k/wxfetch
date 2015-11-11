/*
 *  Atpdec
 *  Copyright (c) 2003 by Thierry Leconte (F4DWV)
 *
 *      $Id: filter.h,v 1.2 2003/10/30 23:21:35 f4dwv Exp $
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

typedef struct {
float G;
float x[3];
float y[2];
} iircoeff_t;

typedef struct {
float x[5];
float y[3];
} iirbuff_t;

float rsfir(float *buff,const float *coeff,const int len ,const double offset ,const double delta);
void iqfir(float *buff,const float *Icoeff,const float *Qcoeff, const int len ,float *I, float *Q);
float fir(float *buff,const float *coeff,const int len);
double iir(double x,iirbuff_t *buff, const iircoeff_t *coeff);

