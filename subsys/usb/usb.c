// glibc includes
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/usb/usbd.h>

LOG_MODULE_REGISTER(usb, CONFIG_USBD_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
static int init();

/* static variable -----------------------------------------------------------*/
static const struct device *const usb_device =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_usb_device));

USBD_DEVICE_DEFINE(usb_ctx, usb_device, 0x2fe3, 0x0004);

USBD_DESC_LANG_DEFINE(usb_desc_lang);
USBD_DESC_MANUFACTURER_DEFINE(usb_desc_manufacturer,
                              "National Taiwan University Racing Team");
USBD_DESC_PRODUCT_DEFINE(usb_desc_product, "Vechicle Control Unit");
#ifdef CONFIG_HWINFO
USBD_DESC_SERIAL_NUMBER_DEFINE(usb_desc_serial_number);
#endif
USBD_DESC_CONFIG_DEFINE(usb_desc_config, "USB full speed");

USBD_CONFIGURATION_DEFINE(usb_config, USB_SCD_SELF_POWERED, 0,
                          &usb_desc_config);

SYS_INIT(init, APPLICATION, CONFIG_NTURT_USB_INIT_PRIORITY);

#if CONFIG_NTURT_USB_MSC
USBD_DEFINE_MSC_LUN(sd, "SD", "Zephyr", "SD", "0.00");
#endif

/* static function definition ------------------------------------------------*/
static int init() {
  int ret;

  ret = usbd_add_descriptor(&usb_ctx, &usb_desc_lang);
  if (ret) {
    LOG_ERR("Failed to add language descriptor: %s", strerror(-ret));
    return ret;
  }

  ret = usbd_add_descriptor(&usb_ctx, &usb_desc_manufacturer);
  if (ret) {
    LOG_ERR("Failed to add manufacturer descriptor: %s", strerror(-ret));
    return ret;
  }

  ret = usbd_add_descriptor(&usb_ctx, &usb_desc_product);
  if (ret) {
    LOG_ERR("Failed to add product descriptor: %s", strerror(-ret));
    return ret;
  }

#ifdef CONFIG_HWINFO

  ret = usbd_add_descriptor(&usb_ctx, &usb_desc_serial_number);
  if (ret) {
    LOG_ERR("Failed to add serial number descriptor: %s", strerror(-ret));
    return ret;
  }

#endif  // CONFIG_HWINFO

  ret = usbd_add_configuration(&usb_ctx, USBD_SPEED_FS, &usb_config);
  if (ret) {
    LOG_ERR("Failed to add configuration: %s", strerror(-ret));
    return ret;
  }

  ret = usbd_register_all_classes(&usb_ctx, USBD_SPEED_FS, 1, NULL);
  if (ret) {
    LOG_ERR("Failed to register classes: %s", strerror(-ret));
    return ret;
  }

  ret = usbd_device_set_code_triple(&usb_ctx, USBD_SPEED_FS,
                                    USB_BCC_MISCELLANEOUS, 0x02, 0x01);
  if (ret) {
    LOG_ERR("Failed to set code triple: %s", strerror(-ret));
    return ret;
  }

  ret = usbd_init(&usb_ctx);
  if (ret) {
    LOG_ERR("Failed to initialize usb device: %s", strerror(-ret));
    return ret;
  }

  ret = usbd_enable(&usb_ctx);
  if (ret) {
    LOG_ERR("Failed to enable usb device: %s", strerror(-ret));
    return ret;
  }

  return 0;
}
