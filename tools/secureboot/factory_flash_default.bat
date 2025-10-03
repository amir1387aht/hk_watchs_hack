@echo 1. Format Ftab and write magic number
@python download.py reset_ftab --port=%2
@echo 2. Generate Flash tables
@armcc ftab/ftab_flash_default.json -E -P > ftab/ftab_default.json
@python imgtool.py enctab --key=%1s01  --sigkey=%1sig --table=ftab/ftab_default
@echo 3. Download Signature key
@python download.py sigkey --key=%1sig --port=%2
@echo 4. Download Flash table 
@echo Core ID: LCPU=0, BCPU=1, HCPU=2, BOOT=3, flashid=coreid+2
@python download.py ftab --table=ftab/ftab_default --port=%2
@echo 5. Finished
