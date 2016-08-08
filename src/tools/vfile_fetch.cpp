#include "log_store.h"
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace phxpaxos;

BytesBuffer g_oTmpBuffer;

void PrintState(AcceptorStateData & oState)
{
    printf("------------------------------------------------------\n");
    printf("instanceid %llu\n", oState.instanceid());
    printf("promiseid %llu\n", oState.promiseid());
    printf("promisenodid %llu\n", oState.promisenodeid());
    printf("acceptedid %llu\n", oState.acceptedid());
    printf("acceptednodeid %llu\n", oState.acceptednodeid());
    printf("acceptedvaluesize %zu\n", oState.acceptedvalue().size());
    printf("checksum %u\n", oState.checksum());
}

int VFileFetch(const std::string & sFilePath, const int iOffset, const int iFetchCount)
{
    int ret = access(sFilePath.c_str(), F_OK);
    if (ret == -1)
    {
        printf("file not exist, filepath %s\n", sFilePath.c_str());
        return 1;
    }

    int iFd = open(sFilePath.c_str(), O_CREAT | O_RDWR, S_IWRITE | S_IREAD);
    if (iFd == -1)
    {
        printf("open fail fail, filepath %s\n", sFilePath.c_str());
        return -1;
    }

    int iFileLen = lseek(iFd, 0, SEEK_END);
    if (iFileLen == -1)
    {
        close(iFd);
        return -1;
    }
    
    off_t iSeekPos = lseek(iFd, iOffset, SEEK_SET);
    if (iSeekPos == -1)
    {
        close(iFd);
        return -1;
    }

    int iNowFetchCount = 0;
    int iNowOffset = iOffset;
    while (true)
    {
        int iLen = 0;
        ssize_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
        if (iReadLen == 0)
        {
            printf("File End, offset %d\n", iNowOffset);
            break;
        }
        
        if (iReadLen != (ssize_t)sizeof(int))
        {
            printf("readlen %zd not qual to %zu\n", iReadLen, sizeof(int));
            break;
        }

        if (iLen == 0)
        {
            printf("File Data End, offset %d\n", iNowOffset);
            break;
        }

        if (iLen > iFileLen || iLen < (int)sizeof(uint64_t))
        {
            printf("File data len wrong, data len %d filelen %d\n",
                    iLen, iFileLen);
            ret = -1;
            break;
        }

        g_oTmpBuffer.Ready(iLen);
        iReadLen = read(iFd, g_oTmpBuffer.GetPtr(), iLen);
        if (iReadLen != iLen)
        {
            printf("readlen %zd not qual to %d\n", iReadLen, iLen);
            break;
        }


        uint64_t llInstanceID = 0;
        memcpy(&llInstanceID, g_oTmpBuffer.GetPtr(), sizeof(uint64_t));

        AcceptorStateData oState;
        bool bBufferValid = oState.ParseFromArray(g_oTmpBuffer.GetPtr() + sizeof(uint64_t), iLen - sizeof(uint64_t));
        if (!bBufferValid)
        {
            printf("This instance's buffer wrong, can't parse to acceptState, instanceid %lu bufferlen %d nowoffset %d\n",
                    llInstanceID, (int)(iLen - sizeof(uint64_t)), iNowOffset);
            break;
        }

        PrintState(oState);

        iNowOffset += sizeof(int) + iLen; 
        iNowFetchCount++;
        if (iNowFetchCount >= iFetchCount)
        {
            break;
        }
    }
    
    close(iFd);

    return ret;
}

int main(int argc, char ** argv)
{
    if (argc < 4)
    {
        printf("%s <filepath> <beginoffset> <fetch count>\n", argv[0]);
        return -2;
    }

    string sFilePath = argv[1];
    int iBeginOffset = atoi(argv[2]);
    int iFetchCount = atoi(argv[3]);

    return VFileFetch(sFilePath, iBeginOffset, iFetchCount);
}

