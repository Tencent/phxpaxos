/*
Tencent is pleased to support the open source community by making 
PhxPaxos available.
Copyright (C) 2016 THL A29 Limited, a Tencent company. 
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may 
not use this file except in compliance with the License. You may 
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" basis, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
implied. See the License for the specific language governing 
permissions and limitations under the License.

See the AUTHORS file for names of contributors. 
*/

#include "util.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <thread>

namespace phxpaxos {

using namespace std;

const uint64_t Time :: GetTimestampMS() 
{
    auto now_time = chrono::system_clock::now();
    uint64_t now = (chrono::duration_cast<chrono::milliseconds>(now_time.time_since_epoch())).count();
    return now;
}

const uint64_t Time :: GetSteadyClockMS() 
{
    auto now_time = chrono::steady_clock::now();
    uint64_t now = (chrono::duration_cast<chrono::milliseconds>(now_time.time_since_epoch())).count();
    return now;
}

void Time :: MsSleep(const int iTimeMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(iTimeMs));
}

/////////////////////////////////////////////
int FileUtils :: IsDir(const std::string & sPath, bool & bIsDir)
{
    bIsDir = false;
    struct stat tStat;
    int ret = stat(sPath.c_str(), &tStat);
    if (ret != 0)
    {
        return ret;
    }

    if (tStat.st_mode & S_IFDIR)
    {
        bIsDir = true;
    }

    return 0;
}

int FileUtils :: DeleteDir(const std::string & sDirPath)
{
    DIR * dir = nullptr;
    struct dirent  * ptr;

    dir = opendir(sDirPath.c_str());
    if (dir == nullptr)
    {
        return 0;
    }


    int ret = 0;
    while ((ptr = readdir(dir)) != nullptr)
    {
        if (strcmp(ptr->d_name, ".") == 0
                || strcmp(ptr->d_name, "..") == 0)
        {
            continue;
        }

        char sChildPath[1024] = {0};
        snprintf(sChildPath, sizeof(sChildPath), "%s/%s", sDirPath.c_str(), ptr->d_name);

        bool bIsDir = false;
        ret = FileUtils::IsDir(sChildPath, bIsDir);
        if (ret != 0)
        {
            break;
        }

        if (bIsDir)
        {
            ret = DeleteDir(sChildPath);
            if (ret != 0)
            {
                break;
            }
        }
        else
        {
            ret = remove(sChildPath);
            if (ret != 0)
            {
                break;
            }
        }
    }

    closedir(dir);

    if (ret == 0)
    {
        ret = remove(sDirPath.c_str());
    }

    return ret;
}

int FileUtils :: IterDir(const std::string & sDirPath, std::vector<std::string> & vecFilePathList)
{
    DIR * dir = nullptr;
    struct dirent  * ptr;

    dir = opendir(sDirPath.c_str());
    if (dir == nullptr)
    {
        return 0;
    }


    int ret = 0;
    while ((ptr = readdir(dir)) != nullptr)
    {
        if (strcmp(ptr->d_name, ".") == 0
                || strcmp(ptr->d_name, "..") == 0)
        {
            continue;
        }

        char sChildPath[1024] = {0};
        snprintf(sChildPath, sizeof(sChildPath), "%s/%s", sDirPath.c_str(), ptr->d_name);

        bool bIsDir = false;
        ret = FileUtils::IsDir(sChildPath, bIsDir);
        if (ret != 0)
        {
            break;
        }

        if (bIsDir)
        {
            ret = IterDir(sChildPath, vecFilePathList);
            if (ret != 0)
            {
                break;
            }
        }
        else
        {
            vecFilePathList.push_back(sChildPath);
        }
    }

    closedir(dir);

    return ret;
}

////////////////////////////////

TimeStat :: TimeStat()
{
    m_llTime = Time::GetSteadyClockMS();
}

int TimeStat :: Point()
{
    uint64_t llNowTime = Time::GetSteadyClockMS();
    int llPassTime = 0;
    if (llNowTime > m_llTime)
    {
        llPassTime = llNowTime - m_llTime;
    }

    m_llTime = llNowTime;

    return llPassTime;
}

/////////////////////////////////

uint64_t OtherUtils :: GenGid(const uint64_t llNodeID)
{
    return (llNodeID ^ FastRand()) + FastRand();
}

//////////////////////////////////////////////////////////

__inline__  int64_t rdtsc() {

#if defined(__i386__)
  int64_t ret;
  __asm__ volatile("rdtsc" : "=A"(ret));
  return ret;
#elif defined(__x86_64__) || defined(__amd64__)
  uint64_t low, high;
  __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
  return (high << 32) | low;
#elif defined(__powerpc__) || defined(__ppc__)
  // This returns a time-base, which is not always precisely a cycle-count.
#if defined(__powerpc64__) || defined(__ppc64__)
  int64_t tb;
  asm volatile("mfspr %0, 268" : "=r"(tb));
  return tb;
#else
  uint32_t tbl, tbu0, tbu1;
  asm volatile(
      "mftbu %0\n"
      "mftbl %1\n"
      "mftbu %2"
      : "=r"(tbu0), "=r"(tbl), "=r"(tbu1));
  tbl &= -static_cast<int32_t>(tbu0 == tbu1);
  // high 32 bits in tbu1; low 32 bits in tbl  (tbu0 is no longer needed)
  return (static_cast<uint64_t>(tbu1) << 32) | tbl;
#endif
#elif defined(__sparc__)
  int64_t tick;
  asm(".byte 0x83, 0x41, 0x00, 0x00");
  asm("mov   %%g1, %0" : "=r"(tick));
  return tick;
#elif defined(__ia64__)
  int64_t itc;
  asm("mov %0 = ar.itc" : "=r"(itc));
  return itc;
#elif defined(COMPILER_MSVC) && defined(_M_IX86)
  // Older MSVC compilers (like 7.x) don't seem to support the
  // __rdtsc intrinsic properly, so I prefer to use _asm instead
  // when I know it will work.  Otherwise, I'll use __rdtsc and hope
  // the code is being compiled with a non-ancient compiler.
  _asm rdtsc
#elif defined(COMPILER_MSVC)
  return __rdtsc();
#elif defined(BENCHMARK_OS_NACL)
  // Native Client validator on x86/x86-64 allows RDTSC instructions,
  // and this case is handled above. Native Client validator on ARM
  // rejects MRC instructions (used in the ARM-specific sequence below),
  // so we handle it here. Portable Native Client compiles to
  // architecture-agnostic bytecode, which doesn't provide any
  // cycle counter access mnemonics.

  // Native Client does not provide any API to access cycle counter.
  // Use clock_gettime(CLOCK_MONOTONIC, ...) instead of gettimeofday
  // because is provides nanosecond resolution (which is noticable at
  // least for PNaCl modules running on x86 Mac & Linux).
  // Initialize to always return 0 if clock_gettime fails.
  struct timespec ts = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<int64_t>(ts.tv_sec) * 1000000000 + ts.tv_nsec;
#elif defined(__aarch64__)
  // System timer of ARMv8 runs at a different frequency than the CPU's.
  // The frequency is fixed, typically in the range 1-50MHz.  It can be
  // read at CNTFRQ special register.  We assume the OS has set up
  // the virtual timer properly.
  int64_t virtual_timer_value;
  asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
  return virtual_timer_value;
#elif defined(__ARM_ARCH)
  // V6 is the earliest arch that has a standard cyclecount
  // Native Client validator doesn't allow MRC instructions.
#if (__ARM_ARCH >= 6)
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  // Read the user mode perf monitor counter access permissions.
  asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {  // Is it counting?
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      // The counter is set up to count every 64th cycle
      return static_cast<int64_t>(pmccntr) * 64;  // Should optimize to << 6
    }
  }
#endif
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#elif defined(__mips__)
  // mips apparently only allows rdtsc for superusers, so we fall
  // back to gettimeofday.  It's possible clock_gettime would be better.
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#elif defined(__s390__)  // Covers both s390 and s390x.
  // Return the CPU clock.
  uint64_t tsc;
  asm("stck %0" : "=Q"(tsc) : : "cc");
  return tsc;
#elif defined(__riscv) // RISC-V
  // Use RDCYCLE (and RDCYCLEH on riscv32)
#if __riscv_xlen == 32
  uint32_t cycles_lo, cycles_hi0, cycles_hi1;
  // This asm also includes the PowerPC overflow handling strategy, as above.
  // Implemented in assembly because Clang insisted on branching.
  asm volatile(
      "rdcycleh %0\n"
      "rdcycle %1\n"
      "rdcycleh %2\n"
      "sub %0, %0, %2\n"
      "seqz %0, %0\n"
      "sub %0, zero, %0\n"
      "and %1, %1, %0\n"
      : "=r"(cycles_hi0), "=r"(cycles_lo), "=r"(cycles_hi1));
  return (static_cast<uint64_t>(cycles_hi1) << 32) | cycles_lo;
#else
  uint64_t cycles;
  asm volatile("rdcycle %0" : "=r"(cycles));
  return cycles;
#endif
#else
// The soft failover to a generic implementation is automatic only for ARM.
// For other platforms the developer is expected to make an attempt to create
// a fast implementation and use generic version if nothing better is available.
#error You need to define CycleTimer for your OS and CPU
#endif
}
struct FastRandomSeed {
    bool init;
    unsigned int seed;
};

static __thread FastRandomSeed seed_thread_safe = { false, 0 };

static void ResetFastRandomSeed()
{
    seed_thread_safe.seed = rdtsc();
    seed_thread_safe.init = true; 
}

static void InitFastRandomSeedAtFork()
{
    pthread_atfork(ResetFastRandomSeed, ResetFastRandomSeed, ResetFastRandomSeed);
}

static void InitFastRandomSeed()
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, InitFastRandomSeedAtFork);

    ResetFastRandomSeed();
}

const uint32_t OtherUtils :: FastRand()
{
    if (!seed_thread_safe.init)
    {
        InitFastRandomSeed();
    }

    return rand_r(&seed_thread_safe.seed);
}

}


