[简体中文README](https://github.com/tencent-wechat/phxpaxos/blob/master/README.zh_CN.md)

**PhxPaxos is a state-synchronization lib based on Paxos protocol, it is totally designed by Wechat independently. It can help your services in synchronizating the state from a single node to another nodes to make services into a multi-copy cluster and fail-over handing automatically by calling functions in our lib.**

**This lib has been used in Wechat production environment, we also test it in a large number of harsh environments to guarantee a stable consistency.**

Authors: Haochuan Cui, Ming Chen, Junchao Chen and Duokai Huang 

Contact us: phxteam@tencent.com

[Principle details(Chinese)](http://mp.weixin.qq.com/s?__biz=MzI4NDMyNTU2Mw==&mid=2247483695&idx=1&sn=91ea422913fc62579e020e941d1d059e#rd)

PhxPaxos [![Build Status](https://travis-ci.org/tencent-wechat/phxpaxos.png)](https://travis-ci.org/tencent-wechat/phxpaxos)

# Features
  * Purely based on [Paxos Made Simple](http://research.microsoft.com/en-us/um/people/lamport/pubs/paxos-simple.pdf) by Lamport. 
  * Transfering message in a async mechanism architecture.
  * Using `fsync` to guarantee the correctness in every IO write operations.
  * Latency of a successful `Propose` is one RTT and one IO write. 
  * Using P2P stream protocol to learn paxos log.
  * Cleaning `Checkpoint` and `PaxosLog` automatically.
  * Pulling `Checkpoint` across nodes automatically.
  * Supporting more than one state-machines in a single PhxPaxos Instance.
  * Supporting recover checkpoint by snapshot+paxoslog automatically.
  * Implementing Master election as a state-machine embedded in PhxPaxos
  * Implementing reconfiguration as a state-machine embedded in PhxPaxos
  * Using signature algorithm embedded in PhxPaxos to recognise invalid hosts.
  * Using checksum to verifying the data consistency of increment data in realtime.
  * Implementing Network, Stroage, Monitor, Logging module as a plugin, they can be implemented customly 
  * Supporting overload protection in a self-adaption way.
  
# Limitations
  * Only a single process (possibly multi-threaded) can run a PhxPaxos instance at a time.
  * There is no client-server support builtin to the library. An application that needs such support will have to wrap their own server around the library.
  * PhxPaxos was only tested on Linux 64bit platform so far.
  
# Performance
### Setup

    CPU: 24 x Intel(R) Xeon(R) CPU E5-2420 0 @ 1.90GHz
    Memory: 2 GB
    Disk: ssd raid5
    Network: Gigabit Ethernet
    Cluster Nodes: 3
    Ping: 0.05ms
    Parallel client: 100 Threads
    
### Performance Test Result(QPS)
> Request latency small than 10ms.
###### Data set with small size(100B)
    1 Group: 1171
    20 Groups: 11931
    50 Groups: 13424
    100 Groups: 13962
###### Data set with larse size(100KB)
    1 Group: 280
    20 Groups: 984
    50 Groups: 1054
    100 Groups: 1067
###### BatchPropose(2KB)
    100 Groups: 150000

# Code Directory Introduction
**include:** This directory includes all head files while using PhxPaxos. You may make some mistakes if you don't understand all the functions in this directory completely.
>NOTE: The public interface is in include/*.h. Callers should not include or rely on the details of any other header files in this package. Those internal APIs may be changed without warning.

**src:** This directory includes all implementation of Phapaxos, You can figure out the working principle of PhxPaoxs by reading this directorys. No neccessary to read it if you are only using PhxPaxos.

**third_party:** This directory is designed to place all third party libs for compiling and running PhxPaxos. You can get a detail in the following compilation section. We have only two libs requirement: Protobuf and LevelDB.

**plugin:** This directory provides the plugin of Logging module. Loggins is an important way to debug a program. But different organazations always implement it in a totally different way. So PhxPaxos does not provide a specific implementation of Logging, instead, it provides the mechanism of Logging so everyone can implement it customly. We also implement a specific Logging by using GLOG for you. If you are using GLOG this may help you in saving your development time.

**sample:** This directory provides 3 samples based on PhxPaoxs, They representive different depth of using PhxPaxos, from easy to hard.
 * PhxElection: This is a very simple program. It implements a Master Election Program by a Master Election state-machine which is embedded in PhxPaxos.
 * PhxEcho: This shows how to program a status-machine and combine it with PhxPaxos.
 * PhxKV: This is a more complete system. which implements a KV state-machine. It shows how to implement a distributed KV storage system by PhxPaxos and how to delete PaxosLog by implementing the Checkpoint API. It also shows how to combine this code into a RPC(etc: GRPC) framework to get a complete distributed KV storage system.

# Guide to Header Files:
 * **include/node.h** The beginning of PhxPaxos. We strongly suggest you to begin here.
 * **include/options.h** Some configurations and options needed while running PhxPaxos.
 * **include/sm.h** A base class of state-machine.
 * **include/def.h** Sets of return code.
 * **include/network.h** Abstract function of Network Module. You can use your own network protocol by overloading these functions.
 * **include/storage.h** Abstract function of Storage Module.
 * **include/log.h** Abstract function of Logging Module.
 * **include/breakpoint.h** Abstract function of breaking points, add your ownMonitor Module here to monitor these break pints. 
 
# How to Compile
### Third party libs preparation
Following is a dependency relationship tablet of all directories.

| Directories             | compilation target        | inner dependency       | third party dependency     |
| ------------------ | -------------------- | ---------------------------------- | ---------------- |
| root             | libphxpaxos.a        | None                                 | protobuf,leveldb |
| plugin             | libphxpaxos_plugin.a | libphxpaxos.a                      | glog             |
| sample/phxelection | Executable program           | libphxpaxos.a,libphxpaxos_plugin.a | None               |
| sample/phxecho     | Executable program           | libphxpaxos.a,libphxpaxos_plugin.a | None               |
| sample/phxkv       | Executable program           | libphxpaxos.a,libphxpaxos_plugin.a | grpc             |
| src/ut             | Unit tests             | None                                 | gtest,gmock      |

We only need 2 third party libs: Protobuf and Leveldb while compiling PhxPaxos library(target is libphxpaoxs.a).But we need GLOG and GRPC while compiling other directories. All these third party libs should be prepared and palced in our `third_party` directory before PhxPaxos compilation. You can `git clone` them by adding `--recurse-submodules` on just download and link them.

### Compilation Enviroment
 * Linux
 * GCC-4.8 or above
 
### Compilation and Installation

###### How to Complie libphxpaxos.a.
 
Execute following shell commands in root directory of PhxPaxos
```Bash
./autoinstall.sh
make
make install
```

###### How to Complie libphxpaxos_plugin.a.
 
Execute following shell commands in plugin directory.
```Bash
make
make install
```

# How to Wrap Your Own Code Around PhxPaxos.
### First choose a single node service.
We will show you this by a PhxEcho service in our `sample` directory, Echo is a common test functions while writing an RPC service. We will wrap this service's code around PhxPaxos to make Echo into a multi-node service.

First, Assume following is the definition fo PhxEchoServer
```c++
class PhxEchoServer
{
public:
    PhxEchoServer();
    ~PhxEchoServer();

    int Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue);
};
```
Let's wrap this code around PhxPaxos.

### Second, implement a state-machine
We now define a PhxEchoSM state-machine which inherit from class `StateMachine` as the following
```c++
class PhxEchoSM : public phxpaxos::StateMachine
{
public:
    PhxEchoSM();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, phxpaxos::SMCtx * poSMCtx);

    const int SMID() const { return 1; }
};
```
`SMID()` functions should return an unique identifier since PhxPaxos support more than 1 state-machines at the same time.

`Execute()` is a state transition function of this state-machine. PhxPaxos guarantees all nodes will execute `sPaxosValue` in the same order to achieve strong consistency (`sPaxosValue` is one of input arguments of `Execute()`).  Following is the implementation of this functions:

```c++
bool PhxEchoSM :: Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue, SMCtx * poSMCtx)
{
    printf("[SM Execute] ok, smid %d instanceid %lu value %s\n", 
            SMID(), llInstanceID, sPaxosValue.c_str());

    //only commiter node have SMCtx.
    if (poSMCtx != nullptr && poSMCtx->m_pCtx != nullptr)
    {   
        PhxEchoSMCtx * poPhxEchoSMCtx = (PhxEchoSMCtx *)poSMCtx->m_pCtx;
        poPhxEchoSMCtx->iExecuteRet = 0;
        poPhxEchoSMCtx->sEchoRespValue = sPaxosValue;
    }   

    return true;
}
```
We only print it on the screen as a prove of broadcasting this Echo message.

Here we got a strange class `SMCtx`, following is the definition.
```c++
class SMCtx
{
public:
    SMCtx();
    SMCtx(const int iSMID, void * pCtx);

    int m_iSMID;
    void * m_pCtx;
};
```
`SMCtx` is a context argument which is provide by proposer(we will offer more details in the following introduction), transmitted to `Execute()` function by PhxPaxos and finally callback to proposer.

`m_iSMID` is related to `SMID()` function mentioned above.PhxPaxos will transmit this to a specific state-machine which owns the same id.

`m_pCtx` is a customly context point address provided by proposer.

>The context arguments of `Execute()` functions is a nullptr in all nodes except the one which propose it. 
Developer should judge if it is NULL while implementing, otherwise it will cause a segment fault.

Following shows the context definition of Echo:
```c++
class PhxEchoSMCtx
{
public:
    int iExecuteRet;
    std::string sEchoRespValue;

    PhxEchoSMCtx()
    {   
        iExecuteRet = -1; 
    }   
};
```
`iExecuteRet` represents the return code of `Execute()` execution. 

`sEchoRespValue` represents the `sEchoReqValue` transmit by `Execute()`.

We finally construct a state-machine and a state transmittion function by classes above.

>HINT: What you should do to make a service into a replicated service is to abstract the logic of it. And then implement it in a `Execute()` fuction. That's all:)

### Running PhxPaxos
After the implementation, We will try to run PhxPaxos with `PhxEchoSM` loaded.

We will do some modifications for `EchoServer class` first:
```c++
class PhxEchoServer
{
public:
    PhxEchoServer(const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList);
    ~PhxEchoServer();

    int RunPaxos();
    int Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue);

    phxpaxos::NodeInfo m_oMyNode;
    phxpaxos::NodeInfoList m_vecNodeList;

    phxpaxos::Node * m_poPaxosNode;
    PhxEchoSM m_oEchoSM;
};
```

We add some arguments in construction function, `oMyNode` represents the information(IP, Port, etc...) of this node.
`vecNodeList` represents informations of all nodes in this cluster.
These 2 arguments is pre-defined by PhxPaxos.

Except `m_oMyNode` and `m_vecNodeList`, `m_oEchoSM` is a state-machine we just finished, `m_poPaxosNode` represents the instance pointer of PhxPaxos running this time.

PhxPaxos instance is actived by executing `RunPaxos()` function, following is the implementation of this function:

```c++
int PhxEchoServer :: RunPaxos()
{
    Options oOptions;

    int ret = MakeLogStoragePath(oOptions.sLogStoragePath);
    if (ret != 0)
    {   
        return ret;
    }   

    //this groupcount means run paxos group count.
    //every paxos group is independent, there are no any communicate between any 2 paxos group.
    oOptions.iGroupCount = 1;

    oOptions.oMyNode = m_oMyNode;
    oOptions.vecNodeInfoList = m_vecNodeList;

    GroupSMInfo oSMInfo;
    oSMInfo.iGroupIdx = 0;
    //one paxos group can have multi state machine.
    oSMInfo.vecSMList.push_back(&m_oEchoSM);
    oOptions.vecGroupSMInfoList.push_back(oSMInfo);

    //use logger_google to print log
    LogFunc pLogFunc;
    ret = LoggerGoogle :: GetLogger("phxecho", "./log", 3, pLogFunc);
    if (ret != 0)
    {   
        printf("get logger_google fail, ret %d\n", ret);
        return ret;
    }   

    //set logger
    oOptions.pLogFunc = pLogFunc;

    ret = Node::RunNode(oOptions, m_poPaxosNode);
    if (ret != 0)
    {   
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }   

    printf("run paxos ok\n");
    return 0;
}
```
All arguments and options have been included in `Option` variable while running PhxPaxos instance.

`MakeLogStoragePath()` function genereate the path where to storage paxos data and it will be set to oOptions.sLogStoragePath.
`oOptions.iGroupCount` represents how many groups running at the same time. They are identified by `GroupIdx`
 (the range is [0, oOptions.iGroupCount) ). Different groups run in a independent space and have no connection with the others groups. Why we put them into a single instance is to make them share the IP/Port.

After configuring all IP/Port informations we will configure the state-machine we just finished.

`oOptions.vecGroupSMInfoList` is a array of `GroupSMInfo` class, it represents a list of status-machines corresponding to every specific group. 

`GroupSMInfo` is a class which is used to describe a state-machine info for a specific group. `GroupSMInfo.iGroupIdx` is the identifier of this group. It will set to be 0 since we have only 1 group.

`vecSMList` is an array of state-machine class. It represents a list of state-machines which we want to load into PhxPaxos. 

Config the Logging functions next, we used GLOG here.

At last, Execute `Node::RunNode(oOptions, m_poPaxosNode)` to run PhxPaxos. Return code 0 means we have active it successfully and `m_poPaxosNode` point to this PhxPaxos instance.

# Proposing Requests
The following codes shows how to reform `Echo()` in `EchoServer` to propose a value in PhxPaxos:
```c++
int PhxEchoServer :: Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue)
{
    SMCtx oCtx;
    PhxEchoSMCtx oEchoSMCtx;
    //smid must same to PhxEchoSM.SMID().
    oCtx.m_iSMID = 1;
    oCtx.m_pCtx = (void *)&oEchoSMCtx;

    uint64_t llInstanceID = 0;
    int ret = m_poPaxosNode->Propose(0, sEchoReqValue, llInstanceID, &oCtx);
    if (ret != 0)
    {   
        printf("paxos propose fail, ret %d\n", ret);
        return ret;
    }   

    if (oEchoSMCtx.iExecuteRet != 0)
    {   
        printf("echo sm excute fail, excuteret %d\n", oEchoSMCtx.iExecuteRet);
        return oEchoSMCtx.iExecuteRet;
    }   

    sEchoRespValue = oEchoSMCtx.sEchoRespValue.c_str();

    return 0;
}
```
First we define a context variable `oEchoSMCtx` and assigned to `oCtx.m_pCtx`.

`oCtx.m_iSMID` set to 1 which corresponding to `SMID()` above, indicate this request will be executed in the state-machine which SMID is 1.

Then call `m_poPaxosNode->Propose` with following arguments:

`GroupIdx`: it indicates which group we will propose this request. The value is 0 here.
`sEchoReqValue`: indicates what we want to propose.
`llInsanceID`: A return arugments we got after proposed successfully, it is global incremented.
`oCtx`: The context variable which will be transmitted to `Execute()` function.

The propose will return 0 if it was proposed successfully, You can get `sEchoRespValue` in the context variable.

After all the steps above, we enhanced a Echo service from a single node into a cluster:)

### The Running Perfomance
The output of Echo cluster is shown in the following. You can implement it by yourself or just compile `sample/phxeco` directory to get this program.

We have 3 nodes in this cluster. Following output came from The node which proposed the value:
```c++
run paxos ok
echo server start, ip 127.0.0.1 port 11112

please input: <echo req value>
hello phxpaxos:)
[SM Execute] ok, smid 1 instanceid 0 value hello phxpaxos:)
echo resp value hello phxpaxos:)
```

We listened on 127.0.0.1:11112. Add we sent "hello phxpaxos" as an input. Then `Execute()` funtion printed `[SM Execute] ok...` as the code we write above, we also got the same `EchoRespValue` at the context variable at the same time.

Let's see what happend in the other nodes:
```c++
run paxos ok
echo server start, ip 127.0.0.1 port 11113

please input: <echo req value>
[SM Execute] ok, smid 1 instanceid 0 value hello phxpaxos:)
```

This one listend on 127.0.0.1:11113, The `Execute()` function also printed "hello phxpaxos".

# Got a election feature for your service by using Master Election in PhxPaxos.

Here we want to explain the exact meaning of Master: Master is a special role in a cluster.At any given moment, there is only one node that considers itself as master at most (remember no master exists is legal.).

This is a very pratical. Assume there is a cluster of multi machines and we wish only one machines to serve at any given moment. The common way to achive this is to build up a Zookeeper cluster and implement a distributed lock service. But now
dozens of lines of code will help you to implement this feature by using our Master Election feature. You don't any extra big modules now.

Following will show you how to embed Master Election into your own service.

First we construct a election class `PhxElection` for existing modules.
```c++
class PhxElection
{
public:
    PhxElection(const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList);
    ~PhxElection();

    int RunPaxos();
    const phxpaxos::NodeInfo GetMaster();
    const bool IsIMMaster();

private:
    phxpaxos::NodeInfo m_oMyNode;
    phxpaxos::NodeInfoList m_vecNodeList;
    phxpaxos::Node * m_poPaxosNode;
};
```
It has two APIs: `GetMaster` to get current master of this cluster and `IsIMMaster` indicate whether I(the node) am master now.

Then implement `RunPaxos()` function:
```c++
int PhxElection :: RunPaxos()
{
    Options oOptions;

    int ret = MakeLogStoragePath(oOptions.sLogStoragePath);
    if (ret != 0)
    {   
        return ret;
    }   

    oOptions.iGroupCount = 1;

    oOptions.oMyNode = m_oMyNode;
    oOptions.vecNodeInfoList = m_vecNodeList;

    //open inside master state machine
    GroupSMInfo oSMInfo;
    oSMInfo.iGroupIdx = 0;
    oSMInfo.bIsUseMaster = true;

    oOptions.vecGroupSMInfoList.push_back(oSMInfo);

    ret = Node::RunNode(oOptions, m_poPaxosNode);
    if (ret != 0)
    {   
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }   

    //you can change master lease in real-time.
    m_poPaxosNode->SetMasterLease(0, 3000);

    printf("run paxos ok\n");
    return 0;
}
```

The difference between `MasterElection` and `Echo` is there is no neccessary to implement your own state-machine this time, instead, you can set `oSMInfo.bIsUseMaster` to `true` to enable our embedded MasterElection state-machine.

Then, run `Node::RunNode()` to get the pointer of PhxPaxos instance.
You can set the lease length of Master by using `SetMasterLease` API at any moment.

Finally, We can get master information from this pointer like following shows:

```c++
const phxpaxos::NodeInfo PhxElection :: GetMaster()
{
    //only one group, so groupidx is 0.
    return m_poPaxosNode->GetMaster(0);
}

const bool PhxElection :: IsIMMaster()
{
    return m_poPaxosNode->IsIMMaster(0);
}
```
Now, every node in the cluster can get master information now by the codes above.

# Welcome to have a try and give us your suggestion :)
