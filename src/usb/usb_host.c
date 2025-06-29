/*
 * Copyright (c) 2015 STMicroelectronics.
 * Copyright (c) 2023 Finalmouse, LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "src/usb/usbh_core.h"
#include "usbh_hid.h"
#include "gfx_main.h"
#include "xlat.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
static char vidpid_string[128];
static char product_string[110] = {0};
static char manuf_string[110] = {0};

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostHS, USBH_HID_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostHS) != USBH_OK)
  {
    Error_Handler();
  }
}

void MX_USB_HOST_ReEnumeration(void)
{
  USBH_ReEnumerate(&hUsbHostHS);
}

/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            // Clear offsets
            xlat_clear_locations();

            gfx_send_event(GFX_EVENT_HID_DEVICE_DISCONNECTED, 0);
            break;

        case HOST_USER_CLASS_SELECTED:
        case HOST_USER_NO_SUPPORTED_CLASS: {
              // Compose vidpid string
              uint16_t vid = phost->device.DevDesc.idVendor;
              uint16_t pid = phost->device.DevDesc.idProduct;
              memset(vidpid_string, 0, sizeof(vidpid_string));
              snprintf(vidpid_string, sizeof(vidpid_string), "0x%04X:%04X", vid, pid);
              vidpid_string[sizeof(vidpid_string) - 1] = '\0';

              gfx_send_event(GFX_EVENT_HID_DEVICE_CONNECTED, 0);
              break;
        }

        case HOST_USER_CLASS_ACTIVE:
            printf("USB device ready\n");

            gfx_send_event(GFX_EVENT_HID_DEVICE_READY, 0);
            break;

        case HOST_USER_CONNECTION:
        default:
            break;
    }
}

void usb_host_set_product_string(const char * product)
{
    snprintf(product_string, sizeof(product_string), "%s", product);
    product_string[sizeof(product_string) - 1] = '\0';
}

char * usb_host_get_product_string(void)
{
    return product_string;
}

void usb_host_set_manuf_string(const char * manuf)
{
    snprintf(manuf_string, sizeof(manuf_string), "%s", manuf);
    manuf_string[sizeof(manuf_string) - 1] = '\0';
}


char * usb_host_get_manuf_string(void)
{
    return manuf_string;
}

char * usb_host_get_vidpid_string(void)
{
    return vidpid_string;
}

uint16_t usb_host_get_polling_time_in_micro_frames(void)
{
  uint8_t poll_intervall = USBH_HID_GetPollInterval(&hUsbHostHS);

  if (hUsbHostHS.device.speed != USBH_SPEED_HIGH)
  {
    poll_intervall *= 8;
  }

  return poll_intervall;
}
