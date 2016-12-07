#include <iostream>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

const uint16_t vendor_id = 0x43e;
const uint16_t product_id = 0x9a40;
const uint16_t max_brightness = 0xd2f0;
const uint16_t min_brightness = 0x0190;
const uint16_t step0 = 0x0064; // total 536 steps
const uint16_t step1 = 0x0218; // total 100 steps
const uint16_t step2 = 0x14f0; // total 10 steps

const size_t LEN = 64;

int main() {
    int r = libusb_init(nullptr);
    if (r < 0) {
        throw "biu";
    }

    libusb_device_handle *hdev = libusb_open_device_with_vid_pid(nullptr, vendor_id, product_id);
    if (hdev == nullptr) {
        throw "biu";
    }

    u_char data[LEN] = {
            0x00, 0x13, 0x00, 0x00, 0x00, 0x00,
    };

    while (true) {
        uint16_t b;
        std::cin >> b;
        if (b > 100) { b = 100; }

        uint16_t val = b * step1 + min_brightness;

        data[0] = u_char(val & 0x00ff);
        data[1] = u_char((val >> 8) & 0x00ff);
        libusb_control_transfer(hdev,
                                LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                LIBUSB_REQUEST_SET_CONFIGURATION, 0x0300, 1, data, 6, 0);

    }

    libusb_close(hdev);

    libusb_exit(nullptr);

    return 0;
}
