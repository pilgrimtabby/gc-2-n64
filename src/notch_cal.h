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

/*  notch_cal.h – Notch calibration for gc-2-n64 firmware
 *
 *  Stores 9 reference points (centre + 8 gate notches) in a dedicated
 *  EEPROM region (offset NOTCH_CAL_EEPROM_OFFSET, 22 bytes) that is
 *  independent of the existing mapping storage.
 *
 *  Correction is an affine triangle interpolation: the measured stick
 *  space is divided into 8 sectors, each defined by two adjacent notch
 *  vectors.  Inside a sector the point is expressed as a weighted sum of
 *  the two measured notch vectors; the same weights are then applied to
 *  the corresponding ideal notch vectors to produce the corrected output.
 *
 *  All values operate in post-calb() N64 axis space (approx. –80 … +80).
 *
 *  Notch index layout (used in notch_cal_sample()):
 *      0 = centre   (release stick)
 *      1 = North    (Up)
 *      2 = NE       (Up-Right)
 *      3 = East     (Right)
 *      4 = SE       (Down-Right)
 *      5 = South    (Down)
 *      6 = SW       (Down-Left)
 *      7 = West     (Left)
 *      8 = NW       (Up-Left)
 */

#ifndef _notch_cal_h__
#define _notch_cal_h__

#include <stdint.h>

/* Number of calibration reference points (1 centre + 8 notches). */
#define NOTCH_CAL_NUM_POINTS  9

/*
 * EEPROM byte offset where calibration data is stored.
 * ATmega328p has 1 KB EEPROM; the existing raphnet struct occupies the
 * first ~268 bytes.  We park our data safely at offset 512.
 */
#define NOTCH_CAL_EEPROM_OFFSET  512

/* ------------------------------------------------------------------ */
/*  Initialise: load calibration from EEPROM (or set passthrough).     */
void notch_cal_init(void);

/*
 * Store a single measurement taken during the calibration wizard.
 *   pos  : 0–8 (see layout above)
 *   x, y : value read from gamecubeXYtoN64() with calibration bypassed
 */
void notch_cal_set_point(uint8_t pos, int8_t x, int8_t y);

/* Persist the current in-RAM calibration table to EEPROM. */
void notch_cal_save(void);

/* Load the calibration table back from EEPROM. */
void notch_cal_load(void);

/* Erase calibration (revert to passthrough). */
void notch_cal_erase(void);

/* Return 1 if a valid calibration is active, 0 otherwise. */
uint8_t notch_cal_is_valid(void);

/*
 * Enable / disable correction at runtime.
 * Call notch_cal_disable() before sampling during the calibration wizard
 * so that raw values are captured without the old correction applied.
 * Call notch_cal_enable() once the wizard is complete.
 */
void notch_cal_enable(void);
void notch_cal_disable(void);

/*
 * Apply the notch correction.
 * Input  : in_x, in_y  – post-calb() N64 axis values (~–80 … +80)
 * Output : *out_x, *out_y – corrected values in the same range
 * If calibration is disabled or not yet valid this is a passthrough.
 */
void notch_cal_correct(int8_t in_x, int8_t in_y, int8_t *out_x, int8_t *out_y);

#endif /* _notch_cal_h__ */
