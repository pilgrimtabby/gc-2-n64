/* retrospy.c – RetroSpy / NintendoSpy serial output implementation  */

#include <avr/io.h>
#include "retrospy.h"

/* ------------------------------------------------------------------ */
/* USART0 initialisation (16 MHz, 115200 baud, U2X=1)                 */
/* */
/* UBRR = F_CPU / (8 * BAUD) - 1 = 16000000 / (8 * 115200) - 1      */
/* = 16.36 → round to 16 → actual = 117647 baud (2.1% error)   */
/* This is within the ±2.5% tolerance of the USART receiver.          */
/* ------------------------------------------------------------------ */
#define RETROSPY_UBRR  16

void retrospy_init(void)
{
    /* Double-speed mode for better baud accuracy. */
    UCSR0A = (1 << U2X0);

    /* Set baud rate. */
    UBRR0H = (uint8_t)(RETROSPY_UBRR >> 8);
    UBRR0L = (uint8_t)(RETROSPY_UBRR);

    /* Enable transmitter only (no RX needed, no interrupts). */
    UCSR0B = (1 << TXEN0);

    /* 8 data bits, 1 stop bit, no parity (8N1). */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    /* PD1 (TXD) direction is taken over automatically when TXEN0 is set. */
}

/* ------------------------------------------------------------------ */
/* Blocking single-byte transmit                                        */
/* ------------------------------------------------------------------ */
static inline void uart_putchar(uint8_t c)
{
    /* Wait until the transmit data register is empty. */
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = c;
}

/* ------------------------------------------------------------------ */
/* Send one RetroSpy N64 packet                                         */
/* ------------------------------------------------------------------ */
void retrospy_send(const uint8_t buf[4])
{
    uint8_t i, mask;

    /*
     * For each of the 4 bytes, emit 8 bytes on the serial link:
     * ASCII '1' if the bit is set, ASCII '0' if clear, MSB first.
     */
    for (i = 0; i < 4; i++) {
        for (mask = 0x80; mask != 0; mask >>= 1) {
            uart_putchar((buf[i] & mask) ? 0x01 : 0x00);
        }
    }

    /* Packet terminator. */
    uart_putchar('\n');
}
