


### Ghetto Shell Scripts for quick testing
PRODUCT_COPY_FILES += \
    device/archos/archos_g9/releasetools/flash-ramdisk.sh:flash-ramdisk.sh
    device/archos/archos_g9/releasetools/flash-ramdisk.sh:flash-recovery.sh

PRODUCT_PACKAGES += \
    static_busybox \
    utility_kdflasher \
    utility_make_ext4fs