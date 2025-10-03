usage: log2monkey log_name xxx.bin

for PC: 
1. system running. in console windows,  input command "indev_agent play_random" to start random-monkey test, and input command "indev_agent stop" to stop test. save log to log_name.
2. use tool, in dos command window, input command "log2monkey log_name xxx.bin" to extract agent monkey data.
3. in console windows,  input command "indev_agent play_record xxx.bin" to playback.

for borad:
1. system running.  in console windows,  input command "indev_agent play_random" to start random-monkey test, and input command "indev_agent stop" to stop test. use uart tool, save log to log_name.
2. using  tool, in dos command window, input command "log2monkey log_name xxx.bin" to extract agent monkey data.
3. in jlink window, using command "loadbin xxx.bin 0x20100000 0x----" to load xxx.bin to board-mem. (0x---- is total bytes of xxx.bin)
4. system running.  in console windows,  input command "indev_agent play_record ----" to playback. (---- is  number of monkey command, unit: dec )
