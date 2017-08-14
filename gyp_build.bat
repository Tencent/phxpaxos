copy /y third_party\leveldb_win\port\port.h third_party\leveldb\port
python third_party\gyp\gyp_main.py --depth=. --include=common.gypi -Dtarget_arch=ia32 --generator-output=. -G msv_version=2017 -f msvs phxpaxos.gyp

