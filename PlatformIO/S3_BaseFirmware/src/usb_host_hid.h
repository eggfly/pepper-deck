#pragma once
#include <hid_usage_mouse.h>


void usb_hid_setup();

typedef struct {
    hid_mouse_input_report_boot_t report;
    int32_t pos_x;
    int32_t pos_y;
} my_hid_mouse_report_t;

extern my_hid_mouse_report_t mouse_hid_report;
