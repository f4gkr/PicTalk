/*
 * Copyright (C) 2013 by Miroslav Slugen <thunder.m@email.cz>
 * Copyright (C) 2017 by Sylvain AZARIAN <sylvain.azarian@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
typedef struct mirisdr_device {
    uint16_t            vid;
    uint16_t            pid;
    const char          *name;
    const char          *manufacturer;
    const char          *product;
    mirisdr_hw_flavour_t hw_flavor ;
} mirisdr_device_t;
*/
// changed to have only RSP1 and RSP2
static mirisdr_device_t mirisdr_devices[] = {
    { 0x1df7, 0x2500, "RSP1", "SDRPlay", "RSP1", MIRISDR_HW_SDRPLAY_RSP1 },
    { 0x1df7, 0x3010, "RSP2", "SDRPlay", "RSP2", MIRISDR_HW_SDRPLAY_RSP2 }
};

static mirisdr_device_t *mirisdr_device_get (uint16_t vid, uint16_t pid) {
    size_t i;

    for (i = 0; i < sizeof(mirisdr_devices) / sizeof(mirisdr_device_t); i++) {
        if ((mirisdr_devices[i].vid == vid) && (mirisdr_devices[i].pid == pid))
            return &mirisdr_devices[i];
    }

    return NULL;
}

/* počet dostupných zařízení */
uint32_t mirisdr_get_device_count (void) {
    ssize_t i, i_max;
    uint32_t ret = 0;
    libusb_context *ctx;
    libusb_device **list;
    struct libusb_device_descriptor dd;

    libusb_init(&ctx);

    i_max = libusb_get_device_list(ctx, &list);

    for (i = 0; i < i_max; i++) {
        libusb_get_device_descriptor(list[i], &dd);

        if (mirisdr_device_get(dd.idVendor, dd.idProduct)) ret++;
    }

    libusb_free_device_list(list, 1);

    libusb_exit(ctx);

    return ret;
}

/* název zařízení */
const char *mirisdr_get_device_name (uint32_t index) {
    ssize_t i, i_max;
    size_t j = 0;
    libusb_context *ctx;
    libusb_device **list;
    struct libusb_device_descriptor dd;
    mirisdr_device_t *device = NULL;

    libusb_init(&ctx);
    i_max = libusb_get_device_list(ctx, &list);

    for (i = 0; i < i_max; i++) {
        libusb_get_device_descriptor(list[i], &dd);

        if ((device = mirisdr_device_get(dd.idVendor, dd.idProduct)) &&
            (j++ == index)) {
            libusb_free_device_list(list, 1);
            libusb_exit(ctx);
            return device->name;
        }
    }

    libusb_free_device_list(list, 1);
    libusb_exit(ctx);

    return "";
}

/* vlastní implementace */
int mirisdr_get_device_usb_strings (uint32_t index, char *manufact, char *product, char *serial) {
    ssize_t i, i_max;
    size_t j = 0;
    libusb_context *ctx;
    libusb_device **list;
    struct libusb_device_descriptor dd;
    mirisdr_device_t *device = NULL;

    libusb_init(&ctx);
    i_max = libusb_get_device_list(ctx, &list);

    for (i = 0; i < i_max; i++) {
        libusb_get_device_descriptor(list[i], &dd);

        if ((device = mirisdr_device_get(dd.idVendor, dd.idProduct)) &&
            (j++ == index)) {
            strcpy(manufact, device->manufacturer);
            strcpy(product, device->product);
            sprintf(serial, "%08u", index + 1);
            libusb_free_device_list(list, 1);
            libusb_exit(ctx);
            return 0;
        }
    }

    memset(manufact, 0, 256);
    memset(product, 0, 256);
    memset(serial, 0, 256);

    libusb_free_device_list(list, 1);
    libusb_exit(ctx);

    return -1;
}
