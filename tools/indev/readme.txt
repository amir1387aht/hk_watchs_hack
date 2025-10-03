Use indev.py to capture and playback input via UART transport. To enable this feature, please eanble BSP_USING_LVGL_INPUT_AGENT in Kconfig.

There are 2 different transport connect to FPGA: console transport and indev capture/playback transport.

For capture:
1. In PC, 
   python indev.py record --logfile=<filename> --port=<capture transport in PC>
   to prepare input capture.
   
2. In console transport
   indev_agent start <capture transport in FPGA>
   each input will be sent to PC to be recorded as following format (totally 12 bytes):
   typedef struct
    {
        uint32_t tick_offset;
        lv_indev_state_t state;     /**< LV_INDEV_STATE_REL or LV_INDEV_STATE_PR*/
        lv_indev_type_t  type;      /**< type of input events*/
        uint8_t          count;     /**< count of same events*/
        uint8_t          interval;  /**< inteval of between last 2 same events*/
        union
        {
            lv_point_t point; /**< For LV_INDEV_TYPE_POINTER the currently pressed point*/
            uint32_t key;     /**< For LV_INDEV_TYPE_KEYPAD the currently pressed key*/
            uint32_t btn_id;  /**< For LV_INDEV_TYPE_BUTTON the currently pressed button*/
            int16_t enc_diff; /**< For LV_INDEV_TYPE_ENCODER number of steps since the previous read*/
        } data;
    } lv_indev_data_packed_t;

3. Normal handle FPGA to demo watch 
4. In console transport
   indev_agent stop 
   to stop recording. Firmware will send out all 0 (12 bytes) to PC.
5. PC will close log file and end logging.   
    Press CTRL+C to exit script
   
For playback:
1. In PC, 
   python indev.py play --logfile=<filename> --port=<capture transport in PC>
   to prepare play back.

2. In console transport,
    indev_agent play <capture transport in FPGA>
   Firmware will send required number of record to PC to ask input record.
   PC will send corresponding number of records to firmware.

3. After Firmware got input record, it will start playback.

4. In console transport
   indev_agent stop 
   to stop playback
   
5. In PC,
   Press CTRL+C to exit script

   
   