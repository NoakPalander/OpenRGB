/*-----------------------------------------*\
|  HyperXAlloyOriginsController.cpp         |
|                                           |
|  Driver for HyperX Alloy Origins RGB      |
|  Keyboard lighting controller             |
|                                           |
|  Adam Honse (CalcProgrammer1) 7/11/2020   |
\*-----------------------------------------*/

#include "HyperXAlloyOriginsController.h"

#include <cstring>

// Skip these indices in the color output
static unsigned int skip_idx[] = { 6, 23, 29, 41, 47, 59, 70, 71, 75, 76, 87, 88, 93, 99, 100, 102, 108, 113, 114, 120, 123, 124 };

HyperXAlloyOriginsController::HyperXAlloyOriginsController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;
}

HyperXAlloyOriginsController::~HyperXAlloyOriginsController()
{
    hid_close(dev);
}

std::string HyperXAlloyOriginsController::GetDeviceLocation()
{
    return("HID " + location);
}

std::string HyperXAlloyOriginsController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    std::wstring return_wstring = serial_string;
    std::string return_string(return_wstring.begin(), return_wstring.end());

    return(return_string);
}

void HyperXAlloyOriginsController::SetLEDsDirect(std::vector<RGBColor> colors)
{
    /*-----------------------------------------------------*\
    | Insert color data for unused positions                |
    \*-----------------------------------------------------*/
    for(unsigned int skip_cnt = 0; skip_cnt < (sizeof(skip_idx) / sizeof(skip_idx[0])); skip_cnt++)
    {
        colors.insert(colors.begin() + skip_idx[skip_cnt], 0x00000000);
    }

    /*-----------------------------------------------------*\
    | Set up variables to track progress of color transmit  |
    | Do this after inserting blanks                        |
    \*-----------------------------------------------------*/
    int colors_to_send = colors.size();
    int colors_sent    = 0;

    SendDirectInitialization();

    for(int pkt_idx = 0; pkt_idx < 9; pkt_idx++)
    {
        if(colors_to_send > 16)
        {
            SendDirectColorPacket(&colors[colors_sent], 16);
            colors_sent    += 16;
            colors_to_send -= 16;
        }
        else if(colors_to_send > 0)
        {
            SendDirectColorPacket(&colors[colors_sent], colors_to_send);
            colors_sent    += colors_to_send;
            colors_to_send -= colors_to_send;
        }
        else
        {
            RGBColor temp = 0x00000000;
            SendDirectColorPacket(&temp, 1);
        }
    }
}

void HyperXAlloyOriginsController::SendDirectInitialization()
{
    unsigned char buf[65];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(buf, 0x00, sizeof(buf));

    /*-----------------------------------------------------*\
    | Set up Direct Initialization packet                   |
    \*-----------------------------------------------------*/
    buf[0x00]   = 0x00;
    buf[0x01]   = 0x04;
    buf[0x02]   = 0xF2;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_send_feature_report(dev, buf, 65);
}

void HyperXAlloyOriginsController::SendDirectColorPacket
    (
    RGBColor*       color_data,
    unsigned int    color_count
    )
{
    unsigned char buf[65];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(buf, 0x00, sizeof(buf));

    /*-----------------------------------------------------*\
    | Set up Direct Initialization packet                   |
    \*-----------------------------------------------------*/
    buf[0x00]   = 0x00;

    /*-----------------------------------------------------*\
    | The maximum number of colors per packet is 16         |
    \*-----------------------------------------------------*/
    if(color_count > 16)
    {
        color_count = 16;
    }

    /*-----------------------------------------------------*\
    | Copy in color data                                    |
    \*-----------------------------------------------------*/
    for(unsigned int color_idx = 0; color_idx < color_count; color_idx++)
    {
        buf[(color_idx * 4) + 1] = 0x81;
        buf[(color_idx * 4) + 2] = RGBGetRValue(color_data[color_idx]);
        buf[(color_idx * 4) + 3] = RGBGetGValue(color_data[color_idx]);
        buf[(color_idx * 4) + 4] = RGBGetBValue(color_data[color_idx]);
    }

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_send_feature_report(dev, buf, 65);
}
