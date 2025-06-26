#include <zephyr/linker/iterable_sections.h>

ITERABLE_SECTION_RAM(tm_data, Z_LINK_ITERABLE_SUBALIGN)
ITERABLE_SECTION_RAM(tm_group, Z_LINK_ITERABLE_SUBALIGN)
