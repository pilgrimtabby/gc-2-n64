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

/*  retrospy.h – RetroSpy / NintendoSpy serial output
 *
 *  Sends the N64 controller state over the Arduino Nano's USB-serial
 *  connection (USART0, PD1/TX) in the NintendoSpy wire protocol so that
 *  the RetroSpy PC application can display inputs live.
 *
 *  Protocol (NintendoSpy N64 format):
 *    – 32 bytes, one per bit of the 4-byte N64 controller response.
 *      Each byte is 0x01 if the bit is set, 0x00 if clear (MSB first
 *      within each byte).
 *    – Followed by 0x0A (newline) as the packet terminator.
 *    Total: 33 bytes per packet at 115200 baud ≈ 2.9 ms.
 *
 *  Bit layout of the 32 input bits (matches n64_status_to_output()):
 *    Bits  0– 7 : A B Z Start DUp DDown DLeft DRight   (byte 0, MSB first)
 *    Bits  8–15 : Reset 0 L R CUp CDown CLeft CRight   (byte 1)
 *    Bits 16–23 : X-axis (signed, MSB first)            (byte 2)
 *    Bits 24–31 : Y-axis (signed, MSB first)            (byte 3)
 *
 *  Hardware note:
 *    PD1 (TXD) is unused by the rest of the firmware at runtime; USART0 is
 *    initialised here for TX-only operation and does not interfere with
 *    the N64 or GC data lines.
 */

#ifndef _retrospy_h__
#define _retrospy_h__

#include <stdint.h>

/*
 * Initialise USART0 for 115200-8N1 TX-only at 16 MHz.
 * Must be called once before retrospy_send().
 */
void retrospy_init(void);

/*
 * Send one RetroSpy packet (33 bytes) for the given 4-byte N64 output buffer.
 * buf must point to exactly 4 bytes as produced by n64_status_to_output().
 * This call blocks for ~2.9 ms while the bytes drain through hardware UART.
 */
void retrospy_send(const uint8_t buf[4]);

#endif /* _retrospy_h__ */
