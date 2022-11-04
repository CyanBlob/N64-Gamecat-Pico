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

	return value;
}

void set_addr_bus_mode(AddrBusConnection_t connection_type)
{
    // TODO: Set transistor GPIO
}

void n64_pi_run(void)
{
	// Init PIO
	PIO pio = pio0;
	uint offset = pio_add_program(pio, &n64_pi_program);
	n64_pi_program_init(pio, 0, offset);
	pio_sm_set_enabled(pio, 0, true);

	uint32_t last_addr;
	uint32_t addr;
	uint32_t next_word;

	// Read addr manually before the loop
	addr = n64_pi_get_value(pio);

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
				addr = n64_pi_get_value(pio);

                // READ
				if (addr == 0) {
                    // TODO: set next_word
                    set_addr_bus_mode(CARTRIDGE);

					pio_sm_put(pio, 0, next_word);
					last_addr += 2;
                // WRITE
				} else if ((addr & 0xffff0000) == 0xffff0000) {
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
			// Read to empty fifo
			addr = n64_pi_get_value(pio);

			// Jump to start of the PIO program.
			pio_sm_exec(pio, 0, pio_encode_jmp(offset + 0));

			// Read and handle the following requests normally
			addr = n64_pi_get_value(pio);
		}
	}
}
