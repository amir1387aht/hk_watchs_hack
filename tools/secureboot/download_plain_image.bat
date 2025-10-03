@echo Download HCPU image part 1 to flash id 4 (2 + coreid)
@python download.py img --eimg=image_plain.bin --bksize=512 --flashid=4 %*
@echo Press reset button to run images.

