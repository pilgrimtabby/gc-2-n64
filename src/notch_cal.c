/*
Copyright (C) 2026 Pilgrim Tabby.

This program is free software: you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. 
If not, see <https://www.gnu.org/licenses/>.
*/

/* notch_cal.c – Notch calibration implementation
 *
 * Inspired by Zenith Labs' affine transformation matrix algorithm. See https://github.com/ZenithControlLabs/Zenith_FW.
 * I believe Zenith Labs adapted their algorithm from PhobGCC-SW, found here: https://github.com/PhobGCC/PhobGCC-SW.
 */

#include <avr/eeprom.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "notch_cal.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ------------------------------------------------------------------ */
/* EEPROM layout (22 bytes at NOTCH_CAL_EEPROM_OFFSET)                */
/* ------------------------------------------------------------------ */
#define NOTCH_CAL_MAGIC_0  'N'
#define NOTCH_CAL_MAGIC_1  'C'
#define NOTCH_CAL_MAGIC_2  'A'
#define NOTCH_CAL_MAGIC_3  'L'

struct notch_cal_eeprom {
    uint8_t magic[4];
    int8_t  notch_x[NOTCH_CAL_NUM_POINTS];
    int8_t  notch_y[NOTCH_CAL_NUM_POINTS];
};

/* ------------------------------------------------------------------ */
/* Ideal N64 target values (Clockwise: N, NE, E, SE, S, SW, W, NW)    */
/* ------------------------------------------------------------------ */
static const int8_t ideal_x[NOTCH_CAL_NUM_POINTS] = {
      0,    0,   69,   80,   69,    0,  -69,  -80,  -69
};
static const int8_t ideal_y[NOTCH_CAL_NUM_POINTS] = {
      0,   80,   69,    0,  -69,  -80,  -69,    0,   69
};

/* Runtime state */
static int8_t meas_x[NOTCH_CAL_NUM_POINTS];
static int8_t meas_y[NOTCH_CAL_NUM_POINTS];

static float affine_coeffs[8][4];
static float boundary_angles[8];

static uint8_t cal_valid   = 0;
static uint8_t cal_enabled = 0;

/* ------------------------------------------------------------------ */
/* Zenith Labs Math Implementation                                      */
/* ------------------------------------------------------------------ */

static void inverse(const float in[3][3], float out[3][3]) {
    float det = in[0][0] * (in[1][1] * in[2][2] - in[2][1] * in[1][2]) -
                in[0][1] * (in[1][0] * in[2][2] - in[1][2] * in[2][0]) +
                in[0][2] * (in[1][0] * in[2][1] - in[1][1] * in[2][0]);
    float invdet = 1.0f / det;

    out[0][0] = (in[1][1] * in[2][2] - in[2][1] * in[1][2]) * invdet;
    out[0][1] = (in[0][2] * in[2][1] - in[0][1] * in[2][2]) * invdet;
    out[0][2] = (in[0][1] * in[1][2] - in[0][2] * in[1][1]) * invdet;
    out[1][0] = (in[1][2] * in[2][0] - in[1][0] * in[2][2]) * invdet;
    out[1][1] = (in[0][0] * in[2][2] - in[0][2] * in[2][0]) * invdet;
    out[1][2] = (in[1][0] * in[0][2] - in[0][0] * in[1][2]) * invdet;
    out[2][0] = (in[1][0] * in[2][1] - in[2][0] * in[1][1]) * invdet;
    out[2][1] = (in[2][0] * in[0][1] - in[0][0] * in[2][1]) * invdet;
    out[2][2] = (in[0][0] * in[1][1] - in[1][0] * in[0][1]) * invdet;
}

static void matrix_matrix_mult(const float left[3][3], const float right[3][3], float output[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            output[i][j] = 0.0f;
            for (int k = 0; k < 3; k++) {
                output[i][j] += left[i][k] * right[k][j];
            }
        }
    }
}

static void calculate_matrices(void) {
    /* Process notches in counter-clockwise order for Zenith algorithm 
     * Mapping: N=1, NW=8, W=7, SW=6, S=5, SE=4, E=3, NE=2 */
    const uint8_t ccw_pts[8] = {1, 8, 7, 6, 5, 4, 3, 2};

    for (int cur = 0; cur < 8; cur++) {
        int pt1 = ccw_pts[cur];
        int pt2 = ccw_pts[(cur + 1) % 8];

        float in_x1 = meas_x[pt1] - meas_x[0];
        float in_y1 = meas_y[pt1] - meas_y[0];
        float in_x2 = meas_x[pt2] - meas_x[0];
        float in_y2 = meas_y[pt2] - meas_y[0];

        float out_x1 = ideal_x[pt1];
        float out_y1 = ideal_y[pt1];
        float out_x2 = ideal_x[pt2];
        float out_y2 = ideal_y[pt2];

        float pointsIn[3][3] = {
            {0, in_x1, in_x2},
            {0, in_y1, in_y2},
            {1, 1,     1    }
        };
        float pointsOut[3][3] = {
            {0, out_x1, out_x2},
            {0, out_y1, out_y2},
            {1, 1,      1     }
        };

        float temp[3][3];
        inverse(pointsIn, temp);
        
        float A[3][3];
        matrix_matrix_mult(pointsOut, temp, A);

        affine_coeffs[cur][0] = A[0][0];
        affine_coeffs[cur][1] = A[0][1];
        affine_coeffs[cur][2] = A[1][0];
        affine_coeffs[cur][3] = A[1][1];

        boundary_angles[cur] = atan2f(in_y2, in_x2);
    }

    /* Unwrap the angles so that the first has the smallest value */
    for (int cur = 0; cur < 8; cur++) {
        if (boundary_angles[cur] < boundary_angles[0]) {
            boundary_angles[cur] += 2.0f * M_PI;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void notch_cal_init(void) {
    notch_cal_load();
}

void notch_cal_set_point(uint8_t pos, int8_t x, int8_t y) {
    if (pos >= NOTCH_CAL_NUM_POINTS) return;
    meas_x[pos] = x;
    meas_y[pos] = y;
}

void notch_cal_save(void) {
    struct notch_cal_eeprom e;
    e.magic[0] = NOTCH_CAL_MAGIC_0;
    e.magic[1] = NOTCH_CAL_MAGIC_1;
    e.magic[2] = NOTCH_CAL_MAGIC_2;
    e.magic[3] = NOTCH_CAL_MAGIC_3;
    memcpy(e.notch_x, meas_x, NOTCH_CAL_NUM_POINTS);
    memcpy(e.notch_y, meas_y, NOTCH_CAL_NUM_POINTS);
    eeprom_write_block(&e, (void*)NOTCH_CAL_EEPROM_OFFSET, sizeof(e));
    
    calculate_matrices();
    cal_valid   = 1;
    cal_enabled = 1;
}

void notch_cal_load(void) {
    struct notch_cal_eeprom e;
    eeprom_read_block(&e, (void*)NOTCH_CAL_EEPROM_OFFSET, sizeof(e));
    if (e.magic[0] != NOTCH_CAL_MAGIC_0 ||
        e.magic[1] != NOTCH_CAL_MAGIC_1 ||
        e.magic[2] != NOTCH_CAL_MAGIC_2 ||
        e.magic[3] != NOTCH_CAL_MAGIC_3) {
        cal_valid   = 0;
        cal_enabled = 0;
        return;
    }

    memcpy(meas_x, e.notch_x, NOTCH_CAL_NUM_POINTS);
    memcpy(meas_y, e.notch_y, NOTCH_CAL_NUM_POINTS);
    
    calculate_matrices();
    cal_valid   = 1;
    cal_enabled = 1;
}

void notch_cal_erase(void) {
    uint8_t zero[4] = { 0, 0, 0, 0 };
    eeprom_write_block(zero, (void*)NOTCH_CAL_EEPROM_OFFSET, 4);
    cal_valid   = 0;
    cal_enabled = 0;
}

uint8_t notch_cal_is_valid(void) { return cal_valid; }
void notch_cal_enable(void) { if (cal_valid) cal_enabled = 1; }
void notch_cal_disable(void) { cal_enabled = 0; }
uint8_t notch_cal_is_active(void) { return cal_enabled && cal_valid; }

void notch_cal_correct(int8_t in_x, int8_t in_y, int8_t *out_x, int8_t *out_y) {
    if (!cal_enabled || !cal_valid) {
        *out_x = in_x;
        *out_y = in_y;
        return;
    }

    float fx = in_x - meas_x[0];
    float fy = in_y - meas_y[0];

    /* Deadzone bypass to prevent noise amplification at center */
    if (fx >= -6.0f && fx <= 6.0f && fy >= -6.0f && fy <= 6.0f) {
        *out_x = 0;
        *out_y = 0;
        return;
    }

    float angle = atan2f(fy, fx);

    /* Unwrap based on Zenith algorithm */
    if (angle < boundary_angles[0]) {
        angle += 2.0f * M_PI;
    }

    int region = 0;
    for (int i = 0; i < 8; i++) {
        if (angle < boundary_angles[i]) {
            region = i;
            break;
        }
    }

    /* Apply affine transformation using precalculated coefficients */
    float rx = affine_coeffs[region][0] * fx + affine_coeffs[region][1] * fy;
    float ry = affine_coeffs[region][2] * fx + affine_coeffs[region][3] * fy;

    /* N64 boundary clamp */
    if (rx < -80.0f) rx = -80.0f;
    if (rx >  80.0f) rx =  80.0f;
    if (ry < -80.0f) ry = -80.0f;
    if (ry >  80.0f) ry =  80.0f;

    *out_x = (int8_t)(rx > 0.0f ? rx + 0.5f : rx - 0.5f);
    *out_y = (int8_t)(ry > 0.0f ? ry + 0.5f : ry - 0.5f);
}
