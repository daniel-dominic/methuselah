#pragma once

#include <tuple>

using Color = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;

// pos in range [0, 1]
//
// Thanks to:
// https://stackoverflow.com/questions/5960979/using-c-vectorinsert-to-add-to-end-of-vector
Color gradient(double pos) {
    //we want to normalize ratio so that it fits in to 6 regions
    //where each region is 256 units long
    int normalized = int(pos * 256 * 6);

    //find the distance to the start of the closest region
    uint8_t x = normalized % 256;

    uint8_t red = 0, grn = 0, blu = 0;
    switch(normalized / 256) {
    case 0: red = 255;      grn = x;        blu = 0;       break;//red
    case 1: red = 255 - x;  grn = 255;      blu = 0;       break;//yellow
    case 2: red = 0;        grn = 255;      blu = x;       break;//green
    case 3: red = 0;        grn = 255 - x;  blu = 255;     break;//cyan
    case 4: red = x;        grn = 0;        blu = 255;     break;//blue
    case 5: red = 255;      grn = 0;        blu = 255 - x; break;//magenta
    }

    return {red, grn, blu, 255};
}