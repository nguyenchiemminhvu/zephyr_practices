/*
 * Zephyr C++ Static Library Linking Demo
 *
 * Demonstrates linking the castle 1.0 C++ library as a static lib
 * into a Zephyr application with full C++ support enabled.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "castle/core/compiler.hpp"
#include "castle/core/config.hpp"

#include "castle_ext/protocols/ubx/ubx.hpp"
#include "castle_ext/protocols/ubx/ubx_config.hpp"
#include "castle_ext/protocols/ubx/messages/messages.hpp"
#include "castle_ext/parsers/ubx_parser/ubx_parser.hpp"
#include "castle_ext/parsers/ubx_parser/decoder_registry.hpp"

/* ── Zephyr application entry point ─────────────────────────────────────── */
int main(void)
{
    printk("=== Zephyr C++ Static Library Linking Demo ===\n");
    printk("    Library: castle 2.0\n\n");

    printk("\nDone.\n");
    return 0;
}
