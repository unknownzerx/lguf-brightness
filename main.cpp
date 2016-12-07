#include <iostream>
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
        std::cout << "Failed to open the device" << std::endl;
        return EXIT_FAILURE;
    }

    libusb_device_handle *hdev = libusb_open_device_with_vid_pid(nullptr, vendor_id, product_id);
    if (hdev == nullptr) {
        std::cout << "Failed to open the device" << std::endl;
        return EXIT_FAILURE;
    }

    auto set_brightness = [&](uint16_t val) {
        u_char data[6] = {
                u_char(val & 0x00ff),
                u_char((val >> 8) & 0x00ff),
                0x00, 0x00, 0x00, 0x00
        };

        libusb_control_transfer(hdev,
                                LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                LIBUSB_REQUEST_SET_CONFIGURATION, 0x0300, 1, data, 6, 0);
    };
    auto get_brightness = [&] {
        u_char data[6];
        libusb_control_transfer(hdev,
                                LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                LIBUSB_REQUEST_CLEAR_FEATURE, 0x0300, 1, data, 6, 0);
        return uint16_t(data[0])
               + (uint16_t(data[1]) << 8);
    };

    set_brightness(get_brightness());

    libusb_close(hdev);
    libusb_exit(nullptr);

    return 0;
}
