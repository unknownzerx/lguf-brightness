#include <iostream>
#include <vector>
#include <curses.h>
#include <libusb.h>

const uint16_t vendor_id = 0x43e;
const uint16_t product_id = 0x9a40;
const uint16_t max_brightness = 0xd2f0;
const uint16_t min_brightness = 0x0190;
const std::vector<uint16_t> small_steps = {
        0x0190, 0x01af, 0x01d2, 0x01f7,
        0x021f, 0x024a, 0x0279, 0x02ac,
        0x02e2, 0x031d, 0x035c, 0x03a1,
        0x03eb, 0x043b, 0x0491, 0x04ee,
        0x0553, 0x05c0, 0x0635, 0x06b3,
        0x073c, 0x07d0, 0x086f, 0x091b,
        0x09d5, 0x0a9d, 0x0b76, 0x0c60,
        0x0d5c, 0x0e6c, 0x0f93, 0x10d0,
        0x1227, 0x1399, 0x1529, 0x16d9,
        0x18aa, 0x1aa2, 0x1cc1, 0x1f0b,
        0x2184, 0x2430, 0x2712, 0x2a2e,
        0x2d8b, 0x312b, 0x3516, 0x3951,
        0x3de2, 0x42cf, 0x4822, 0x4de1,
        0x5415, 0x5ac8, 0x6203, 0x69d2,
        0x7240, 0x7b5a, 0x852d, 0x8fc9,
        0x9b3d, 0xa79b, 0xb4f5, 0xc35f,
        0xd2f0,
};
const std::vector<uint16_t> big_steps = {
        0x0190, 0x021f, 0x02e2, 0x03eb,
        0x0553, 0x073c, 0x09d5, 0x0d5c,
        0x1227, 0x18aa, 0x2184, 0x2d8b,
        0x3de2, 0x5415, 0x7240, 0x9b3d,
        0xd2f0,
};

auto next_step = [](uint16_t val, auto &steps) {
    for (auto step : steps) {
        if (step > val) {
            return step;
        }
    }
    return steps.back();
};
auto prev_step = [](uint16_t val, auto &steps) {
    for (auto it = steps.rbegin(); it != steps.rend(); ++it) {
        if (*it < val) {
            return *it;
        }
    }
    return steps.front();
};

void print_fail_getch() {
    printw("Failed to open the device. Press any key to exit: ");
    getch();
}

int main(int argc, char **argv) {
    if (libusb_init(nullptr) < 0) {
        print_fail_getch();
        return EXIT_FAILURE;
    }

    libusb_device_handle *hdev = libusb_open_device_with_vid_pid(nullptr, vendor_id, product_id);
    if (hdev == nullptr) {
        libusb_exit(nullptr);
        print_fail_getch();
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
    
    if (argc > 1) {
        set_brightness(get_brightness()); // reset brightness for resolving a possible hardware bug
    } else {

        initscr();
        timeout(-1);
        noecho();

        auto brightness = get_brightness();
        printw("Press '-' or '=' to adjust brightness.\n");
        printw("Press '[' or: ']' to fine tune.\n");
        printw("Press 'q' or Enter to quit.\n");
        printw("Input: ");

        bool stop = false;
        while (not stop) {
            int c = getch();
            switch (c) {
                case '+':
                case '=':
                    brightness = next_step(brightness, big_steps);
                    set_brightness(brightness);
                    break;
                case '-':
                case '_':
                    brightness = prev_step(brightness, big_steps);
                    set_brightness(brightness);
                    break;
                case ']':
                    brightness = next_step(brightness, small_steps);
                    set_brightness(brightness);
                    break;
                case '[':
                    brightness = prev_step(brightness, small_steps);
                    set_brightness(brightness);
                    break;
                case 'q':
                case '\n':
                    stop = true;
                    break;
                default:
                    break;
            }
        }
    }
    
    libusb_close(hdev);
    libusb_exit(nullptr);

    return 0;
}
