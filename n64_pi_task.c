/**
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Konrad Beckmann
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/irq.h"

#include "n64_pi_task.h"
#include "n64_pi.h"

static inline uint32_t n64_pi_get_value(PIO pio)
{
    // gets from the shift register?
	uint32_t value = pio_sm_get_blocking(pio, 0);
    //printf("GOT 0x%02X\n", value);

	return value;
}

void set_addr_bus_mode(AddrBusConnection_t connection_type)
{
    // TODO: Set transistor GPIO
}

void n64_pi_run(void)
{
	// Init console PIO
	PIO pio_console = pio0;
	uint offset = pio_add_program(pio_console, &n64_pi_console_program);
	n64_pi_console_program_init(pio_console, 0, offset);
	pio_sm_set_enabled(pio_console, 0, true);

	// Init cartridge PIO
	PIO pio_cart = pio1;
	offset = pio_add_program(pio_cart, &n64_pi_cart_program);
	n64_pi_cart_program_init(pio_cart, 0, offset);
	pio_sm_set_enabled(pio_cart, 0, true);

	uint32_t last_addr;
	uint32_t addr;
	uint32_t next_word;

	// Read addr manually before the loop
    printf("INITIAL READ\n");
	addr = n64_pi_get_value(pio_console);

	while (1) {
		// addr must not be a WRITE or READ request here,
		// it should contain a 16-bit aligned address.
		// Assert drains performance, uncomment when debugging.
		// ASSERT((addr != 0) && ((addr & 1) == 0));

		// We got a start address
		last_addr = addr;

		// Handle access based on memory region
		// Note that the if-cases are ordered in priority from
		// most timing critical to least.
		if (true) {
			// Domain 1, Address 2 Cartridge ROM
			do {
				// Pre-fetch from the address
                printf("SECOND READ\n");
                addr = n64_pi_get_value(pio_console);
                
                //if (addr != last_addr)
                {
                    printf("ADDR: 0x%02X\n", addr);
                }

                // READ
				if (addr == 0) {
                    printf("GOT READ\n");
                    // TODO: set next_word
                    set_addr_bus_mode(CARTRIDGE);

                    next_word = 0b0101010101010101;
					pio_sm_put(pio_console, 0, next_word);
					pio_sm_put(pio_cart, 0, next_word);
					last_addr += 2;
                // WRITE
				} else if ((addr & 0xffff0000) == 0xffff0000) {
                    printf("GOT WRITE\n");
					// Ignore data since we're asked to write to the ROM.
					last_addr += 2;
				} else {
					// New address
					break;
				}
			} while (1);
		} else {
			// Don't handle this request - jump back to the beginning.
			// This way, there won't be a bus conflict in case e.g. a physical N64DD is connected.

			// Enable to log addresses to UART
#if 0
			uart_print_hex_u32(last_addr);
#endif
            printf("RESTART READ 1\n");
			// Read to empty fifo
			addr = n64_pi_get_value(pio_console);

			// Jump to start of the PIO program.
			pio_sm_exec(pio_console, 0, pio_encode_jmp(offset + 0));

			// Read and handle the following requests normally
            printf("RESTART READ 2\n");
            addr = n64_pi_get_value(pio_console);
		}
	}
}
