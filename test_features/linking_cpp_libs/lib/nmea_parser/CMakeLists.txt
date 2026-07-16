# ──────────────────────────────────────────────────────────────────────────────
# nmea_parser — plain CMake static C++ library for Zephyr linkage
# ──────────────────────────────────────────────────────────────────────────────

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nmea_parser STATIC
    # ── Core ──────────────────────────────────────────────────────────────────
    src/nmea_parser.cpp
    src/nmea_dispatcher.cpp
    src/nmea_sentence_registry.cpp

    # ── Sentence parsers ──────────────────────────────────────────────────────
    src/parsers/gga_parser.cpp
    src/parsers/rmc_parser.cpp
    src/parsers/gsa_parser.cpp
    src/parsers/gsv_parser.cpp
    src/parsers/vtg_parser.cpp
    src/parsers/gll_parser.cpp
    src/parsers/gns_parser.cpp
    src/parsers/gbs_parser.cpp
    src/parsers/gst_parser.cpp
    src/parsers/zda_parser.cpp
    src/parsers/dtm_parser.cpp

    # ── Database layer ────────────────────────────────────────────────────────
    src/database/nmea_database.cpp
    src/database/nmea_snapshot.cpp
    src/database/nmea_database_adapter.cpp
    src/database/handlers/db_gga_handler.cpp
    src/database/handlers/db_rmc_handler.cpp
    src/database/handlers/db_gsa_handler.cpp
    src/database/handlers/db_gsv_handler.cpp
    src/database/handlers/db_vtg_handler.cpp

    # ── Generator layer ───────────────────────────────────────────────────────
    src/generator/nmea_generator_facade.cpp
    src/generator/nmea_formatter.cpp
    src/generator/builders/gga_builder.cpp
    src/generator/builders/rmc_builder.cpp
    src/generator/builders/gsa_builder.cpp
    src/generator/builders/gsv_builder.cpp
    src/generator/builders/vtg_builder.cpp
    src/generator/builders/gll_builder.cpp
    src/generator/builders/zda_builder.cpp
)

# Public headers — consumers only need the include/ directory
target_include_directories(nmea_parser PUBLIC
    include
)

# Private headers — internal implementation headers in src/
target_include_directories(nmea_parser PRIVATE
    src
)

target_link_libraries(nmea_parser PRIVATE zephyr_interface)
