#!/usr/bin/env python
# Copyright (c) 2025 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# You can use this script to convert an image to C code that can used with the RAMN SPI module.

from PIL import Image
import os

file_path = "<replace this with image file path>"
result = ""

image = Image.open(file_path).convert("RGB")

WIDTH = image.size[0]
HEIGHT = image.size[1]

result += "const uint8_t image_width = {};\r\n".format(WIDTH)

result += "const uint8_t image_height = {};\r\n".format(HEIGHT)

result += "const uint8_t image"+ "[{}] = ".format(WIDTH*HEIGHT*2) + "{"
for y in range(HEIGHT):
    for x in range(WIDTH):
        r,g,b = image.getpixel((x,y))

        R = r >> 3
        G = g >> 2
        B = b >> 3
        rgb565 = (R << 11) | (G << 5) | B
        
        #result += hex((rgb565 >> 8)&0xFF) + ", " + hex((rgb565)&0xFF) + ", " 
        result += hex((rgb565)&0xFF) + ", " + hex((rgb565 >> 8)&0xFF) + ", "
result += "};\n"
    
print(result)



    