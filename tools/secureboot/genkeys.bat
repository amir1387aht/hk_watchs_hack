@echo 1. Generate UID
@python imgtool.py uid --key=%1uid 
@echo 2. Generate Root key
@python imgtool.py root --rootkey=%1s01
@echo 3. Generate Signature key
@python imgtool.py gensig --sigkey=%1sig
@echo 4. Create Signature hash
@python imgtool.py dumpsig --sigkey=%1sig
@echo 5. Finished
