@echo generate encrypted ftab
@python imgtool.py enc_static --sigkey=%1sig --table=%3 --key=%1s01 --uid=%1uid --img=%2 --eimg=image_sec.bin --bksize=512 --flags=3