#include <zephyr/linker/iterable_sections.h>

ITERABLE_SECTION_RAM(err, 4)
ITERABLE_SECTION_ROM(err_callback, 4)
