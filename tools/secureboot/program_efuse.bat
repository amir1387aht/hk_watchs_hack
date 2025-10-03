rem Usage: program_efuse.bat sifli01/ COM3
@echo 1. Enable efuse
@python download.py enable_secure --secure=165  --port=%2 --verbose=2
@echo 2. Download uid
@python download.py uid --key=%1uid --port=%2 --verbose=2
@echo 3. Download Root key
@python download.py root --key=%1s01 --port=%2 --verbose=2
@echo 4. Download signature hash
@python download.py sighash --key=%1sig --port=%2 --verbose=2

