// canopennode includes
#include <canopennode.h>

/* static variable -----------------------------------------------------------*/
CANOPEN_STORAGE_ENTRY_DEFINE(OD_PERSIST_COMM, 0x02,
                             CO_storage_cmd | CO_storage_restore);

// CANOPEN_STORAGE_ENTRY_DEFINE(OD_EEPROM, 0x03, CO_storage_cmd |
// CO_storage_restore);
