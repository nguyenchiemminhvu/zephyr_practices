/*
 * Zephyr C++ Static Library Linking Demo
 *
 * Demonstrates linking the nmea_parser C++ library as a static lib
 * into a Zephyr application with full C++ support enabled.
 *
 * The application:
 *   1. Creates an nmea_parser with GGA and RMC callbacks registered.
 *   2. Feeds simulated NMEA sentences (realistic u-blox ZED-F9R output).
 *   3. Prints decoded position, speed, and satellite data via printk.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "nmea_parser.h"
#include "nmea_sentence_registry.h"
#include "nmea_errors.h"
#include "parsers/gga_parser.h"
#include "parsers/rmc_parser.h"
#include "parsers/gsv_parser.h"
#include "parsers/vtg_parser.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using namespace nmea::parser;

/* ── Helper: build a valid NMEA sentence with computed checksum ──────────── */
static std::vector<uint8_t> make_sentence(const std::string &body)
{
    uint8_t cs = 0U;
    for (char c : body) {
        cs ^= static_cast<uint8_t>(c);
    }

    char buf[8];
    std::snprintf(buf, sizeof(buf), "*%02X\r\n",
                  static_cast<unsigned>(cs));

    std::string full = "$" + body + buf;
    return std::vector<uint8_t>(full.begin(), full.end());
}

/* ── Zephyr application entry point ─────────────────────────────────────── */
int main(void)
{
    printk("=== Zephyr C++ Static Library Linking Demo ===\n");
    printk("    Library: nmea_parser (NMEA 0183 C++ parser)\n\n");

    /* ── 1. Create the sentence registry and register callbacks ────────── */
    nmea_sentence_registry registry;

    /* GGA — position fix */
    registry.register_parser(std::unique_ptr<gga_parser>(
        new gga_parser([](const nmea_gga &g) {
            if (!g.valid)
            {
                printk("[GGA] Invalid sentence\n");
                return;
            }
            printk("[GGA] talker=%s  lat=%d.%06d  lon=%d.%06d  "
                   "fix=%d  sats=%d  alt=%d.%01d m  hdop=%d.%02d\n",
                   g.talker.c_str(),
                   (int)g.latitude,
                   (int)((g.latitude - (int)g.latitude) * 1000000),
                   (int)g.longitude,
                   (int)((g.longitude - (int)g.longitude) * 1000000),
                   static_cast<int>(g.fix_quality),
                   g.num_satellites,
                   (int)g.altitude_msl,
                   (int)((g.altitude_msl - (int)g.altitude_msl) * 10),
                   (int)g.hdop,
                   (int)((g.hdop - (int)g.hdop) * 100));
        })));

    /* RMC — recommended minimum navigation data */
    registry.register_parser(std::unique_ptr<rmc_parser>(
        new rmc_parser([](const nmea_rmc &r) {
            if (!r.valid)
            {
                printk("[RMC] Invalid sentence\n");
                return;
            }
            printk("[RMC] status=%s  speed=%d.%03d kn  "
                   "course=%d.%01d deg  date=%u\n",
                   r.status_active ? "ACTIVE" : "VOID",
                   (int)r.speed_knots,
                   (int)((r.speed_knots - (int)r.speed_knots) * 1000),
                   (int)r.course_true,
                   (int)((r.course_true - (int)r.course_true) * 10),
                   r.date);
        })));

    /* VTG — course over ground and ground speed */
    registry.register_parser(std::unique_ptr<vtg_parser>(
        new vtg_parser([](const nmea_vtg &v) {
            if (!v.valid)
            {
                printk("[VTG] Invalid sentence\n");
                return;
            }
            printk("[VTG] course_true=%d.%01d deg  "
                   "speed_kmh=%d.%03d km/h\n",
                   (int)v.course_true,
                   (int)((v.course_true - (int)v.course_true) * 10),
                   (int)v.speed_kmh,
                   (int)((v.speed_kmh - (int)v.speed_kmh) * 1000));
        })));

    /* GSV — satellites in view */
    registry.register_parser(std::unique_ptr<gsv_parser>(
        new gsv_parser([](const nmea_gsv &sv) {
            if (!sv.valid)
            {
                printk("[GSV] Invalid sentence\n");
                return;
            }
            printk("[GSV] talker=%s",
                   sv.talker.c_str());
            printk("\n");
        })));

    /* ── 2. Create the parser (takes ownership of the registry) ────────── */
    nmea_parser parser(std::move(registry));

    /* Error callback — catch checksum failures, malformed fields, etc. */
    parser.set_error_callback([](const parse_error_info &e) {
        printk("[ERROR] type=%s  code=%d  desc=%s\n",
               e.sentence_type.c_str(),
               static_cast<int>(e.code),
               e.description.c_str());
    });

    /* ── 3. Feed simulated NMEA burst (u-blox ZED-F9R style) ───────────── */
    printk("--- Feeding NMEA epoch 1 ---\n");

    const auto gga = make_sentence(
        "GNGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,");
    const auto rmc = make_sentence(
        "GNRMC,092725.00,A,4717.11399,N,00833.91590,E,0.004,77.52,160223,,,A");
    const auto vtg = make_sentence(
        "GNVTG,77.52,T,,M,0.004,N,0.007,K,A");
    const auto gsv1 = make_sentence(
        "GPGSV,2,1,05,21,40,083,46,05,35,141,47,25,42,057,50,46,38,103,34");
    const auto gsv2 = make_sentence(
        "GPGSV,2,2,05,16,08,320,19");

    parser.feed(gga);
    parser.feed(rmc);
    parser.feed(vtg);
    parser.feed(gsv1);
    parser.feed(gsv2);

    /* ── 5. Print statistics ───────────────────────────────────────────── */
    printk("\n--- Statistics ---\n");
    printk("Sentences decoded:   %llu\n",
           (unsigned long long)parser.sentences_decoded());
    printk("Sentences discarded: %llu\n",
           (unsigned long long)parser.sentences_discarded());

    printk("\nDone.\n");
    return 0;
}
