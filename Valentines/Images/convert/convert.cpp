#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <string>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

// Targets for the STM32
constexpr int TARGET_W = 50;
constexpr int TARGET_H = 50;

// This function now handles the Big-Endian swap for ST7789
uint16_t rgb888_to_rgb565_swapped(uint8_t r, uint8_t g, uint8_t b) {
    // 1. Calculate the 5-6-5 components
    uint8_t r5 = (r & 0xF8);      // Red top 5 bits
    uint8_t g6 = (g & 0xFC);      // Green top 6 bits
    uint8_t b5 = (b >> 3);        // Blue top 5 bits

    // 2. Assemble the two bytes for the ST7789 (Big-Endian)
    // Byte 0 (Sent first): RRRRR GGG
    // Byte 1 (Sent second): GGG BBBBB
    uint8_t byte0 = r5 | (g6 >> 5);
    uint8_t byte1 = (g6 << 3) | b5;

    // 3. Combine into a 16-bit word that, when cast to uint8_t* by the STM32,
    // will send byte0 then byte1.
    return (uint16_t)(byte0 | (byte1 << 8));
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.png> <output.h>" << std::endl;
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];

    int width, height, channels;
    uint8_t* src_img = stbi_load(input_file, &width, &height, &channels, 3);
    if (!src_img) {
        std::cerr << "Error: Could not load " << input_file << std::endl;
        return 1;
    }

    // Prepare buffer for the resized image
    std::vector<uint8_t> resized_img(TARGET_W * TARGET_H * 3);

    // Resize using high-quality linear interpolation
    stbir_resize_uint8_linear(src_img, width, height, 0,
                               resized_img.data(), TARGET_W, TARGET_H, 0,
                               STBIR_RGB);

    // Write the .h file
    std::ofstream out(output_file);
    if (!out) {
        std::cerr << "Error: Could not open output file" << std::endl;
        return 1;
    }

    // Create a clean Header Guard name
    std::string guard = output_file;
    std::replace_if(guard.begin(), guard.end(), [](char c){ return !std::isalnum(c); }, '_');
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);

    out << "#ifndef " << guard << "\n#define " << guard << "\n\n";
    out << "#include <stdint.h>\n\n";
    out << "// Scale:" << TARGET_W << "x" << TARGET_H << std::endl;
    out << "const uint16_t " << "image_data" << "[2500] = {\n    ";

    for (int i = 0; i < TARGET_W * TARGET_H; ++i) {
        uint8_t r = resized_img[i * 3 + 0];
        uint8_t g = resized_img[i * 3 + 1];
        uint8_t b = resized_img[i * 3 + 2];

        uint16_t color = rgb888_to_rgb565_swapped(r, g, b);

        out << "0x" << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << color;

        if (i < (TARGET_W * TARGET_H) - 1) {
            out << ", ";
            if ((i + 1) % 12 == 0) out << "\n    ";
        }
    }

    out << "\n};\n\n#endif\n";

    stbi_image_free(src_img);
    std::cout << "Successfully converted " << input_file << " to " << output_file << " (" << TARGET_W << "x" << TARGET_H << ", Byte-Swapped)" << std::endl;

    return 0;
}