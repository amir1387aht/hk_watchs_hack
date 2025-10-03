import string

def SifliEnv():

    try:
        f = open('../../../../.version', 'r')
        for line in f:
            line=line[:-1]
            fields=line.split('=')
            if (fields[0]=='MAJOR'):
                MAJOR=int(fields[1])
            elif (fields[0]=='MINOR'):
                MINOR=int(fields[1])
            elif (fields[0]=='REV'):
                REV=int(fields[1])
        f.close()
        sifli_version=(MAJOR<<24)+(MINOR<<16)+REV
    except:
        print("Cannot get SDK version")
        exit()

    str_version = str(sifli_version)
    print(str_version)
    f = open('./src/flashdrv_version.h', 'w')
    f.write('#define FLASH_JLINK_VERSION ')
    f.write(str_version)
    f.write('\n')
    f.close()

SifliEnv()
