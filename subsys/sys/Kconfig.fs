config NTURT_SYS_FS
    bool "File system module"
    depends on DISK_ACCESS && FILE_SYSTEM
    help
      Automatically mount SD card to file system of choise.

if NTURT_SYS_FS

config NTURT_SYS_FS_MOUNT_POINT
    string "Mount point of the SD card"
    default "/SD:"

choice NTURT_SYS_FS_TYPE
    prompt "Which file system to use"
    default NTURT_SYS_FS_LITTLE_FS

config NTURT_SYS_FS_FAT_FS
    bool "Uses FatFs as the file system"
    depends on FAT_FILESYSTEM_ELM

config NTURT_SYS_FS_LITTLE_FS
    bool "Uses LittleFs as the file system"
    depends on FILE_SYSTEM_LITTLEFS
    select FS_LITTLEFS_BLK_DEV

endchoice

config NTURT_SD_DISK_NAME
    string "SD card disk name to mount"
    default "SD" if NTURT_SYS_FS_FAT_FS
    default "SDMMC"

endif # NTURT_SYS_FS
