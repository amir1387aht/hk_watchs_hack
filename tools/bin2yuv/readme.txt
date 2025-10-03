Supported color format:
a8
rgb565
rgb888
argb8888
rgba8888



# bin2bmp.py example:
# Show rgb565 454x454 framebuffer in hcpu_ram.bin at offset 0x2000
python bin2bmp.py hcpu_ram.bin rgb565 454 454 2000





# jlinkbin2bmp.py example:
# Connect target device£¬and show rgb565 454x454 framebuffer at 0x20027610
python jlinkbin2bmp.py CORTEX-M33 rgb565 454 454 20027610