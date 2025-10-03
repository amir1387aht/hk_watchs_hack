@echo Download SEC BOOT patch image part 1 to flash id 5
@python download.py img --eimg=boot_rom_patch_sec.bin --bksize=512 --flashid=5 --verbose=2 --port=%1