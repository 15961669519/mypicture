#!/bin/bash

arduino-cli compile -b esp32:esp32:esp32s3 $1 -j 0 -v --warnings all --board-options "FlashSize=16M" --board-options "PartitionScheme=custom" --board-options "PSRAM=opi" --build-property "build.extra_flags=-Wall -Werror -DPRINT_HEAP_INFO_INTERVAL=1000 -DCLOGGER_SEVERITY=0"