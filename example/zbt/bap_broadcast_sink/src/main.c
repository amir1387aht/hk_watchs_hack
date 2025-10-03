#include "bap_broadcast_sink_api.h"
#include "log.h"

#define DEFAULT_VOLUME 10

int main(void)
{
    int err;

    err = bt_enable(NULL);
    if (err)
    {
        printk("Bluetooth enable failed (err %d)\n", err);
        return err;
    }
    printk("Bluetooth initialized\n");

    //rt_thread_mdelay(1000);

    err = bap_broadcast_sink_init();
    if (err)
    {
        printk("Init failed (err %d)\n", err);
        return -1;
    }

    audio_server_set_private_volume(AUDIO_TYPE_BT_MUSIC, DEFAULT_VOLUME);
    while (1)
    {
        err = bap_broadcast_sink_start(0);
        if (err)
        {
            printk("start bap sink failed\n");
            rt_thread_mdelay(100);
            continue;
        }
        break;
    }

    return 0;
}

void audio_src(int argc, char **argv)
{
    if (argc < 2)
        return;

    if (argv[1][0] == '0')
    {
        bap_broadcast_sink_stop();
        printk("\r\ninput audio_src 1 to start\n\n");
    }
    else
    {
        bap_broadcast_sink_start(0);
        printk("\r\ninput audio_src 0 to stop\n\n");
    }
}
MSH_CMD_EXPORT(audio_src, audio_src command)

