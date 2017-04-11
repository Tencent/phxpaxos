<img align="right" src="http://mmbiz.qpic.cn/mmbiz/UqFrHRLeCAkOcYOjaX3oxIxWicXVJY0ODsbAyPybxk4DkPAaibgdm7trm1MNiatqJYRpF034J7PlfwCz33mbNUkew/640?wx_fmt=jpeg&wxfrom=5&wx_lazy=1" hspace="15" width="300px" style="float: right">

**PhxPaxos是腾讯公司微信后台团队自主研发的一套基于Paxos协议的多机状态拷贝类库。它以库函数的方式嵌入到开发者的代码当中，
使得一些单机状态服务可以扩展到多机器，从而获得强一致性的多副本以及自动容灾的特性。**
**这个类库在微信服务里面经过一系列的工程验证，并且我们对它进行过大量的恶劣环境下的测试，使其在一致性的保证上更为健壮。**

作者：Haochuan Cui, Ming Chen, Junchao Chen 和 Duokai Huang 

联系我们：phxteam@tencent.com

想了解更多, 以及更详细的编译手册，请进入[中文WIKI](https://github.com/tencent-wechat/phxpaxos/wiki)，和扫描右侧二维码关注我们的公众号

[关于实现的一些原理细节](http://mp.weixin.qq.com/s?__biz=MzI4NDMyNTU2Mw==&mid=2247483695&idx=1&sn=91ea422913fc62579e020e941d1d059e#rd)

# 特性
  * 基于Lamport的 [Paxos Made Simple](http://research.microsoft.com/en-us/um/people/lamport/pubs/paxos-simple.pdf) 进行工程化，不进行任何算法变种。
  * 使用基于消息传递机制的纯异步工程架构。
  * 每次写盘使用fsync严格保证正确性。
  * 一次Propose（写入数据）的Latency为一次RTT，均摊单机写盘次数为1次。
  * 使用点对点流式协议进行快速学习。
  * 支持Checkpoint以及对PaxosLog的自动清理。
  * 支持跨机器的Checkpoint自动拉取。
  * 一个PhxPaxos实例可以同时挂载多个状态机。
  * 可使用镜像状态机模式进行Checkpoint的自动生成。
  * 内置Master选举功能。
  * 线上数据的实时增量checksum校验。
  * 网络、存储、监控、日志模块插件化，可由开发者自定义。
  * 基于Paxos算法的安全的成员变更。
  * 基于Paxos算法的集群签名保护，隔离非法签名的错误机器。
  * 自适应的过载保护。
  
# 局限
  * 一个PhxPaxos实例任一时刻只允许运行在单一进程（容许多线程）。
  * 这个类库没有内建对client-server的支持，开发者必须将类库的代码嵌入到自己的服务器代码里面，以实现这个功能。
  * PhxPaxos只容许运行在64位的Linux平台。
  
# 性能
### 运行环境

    CPU：24 x Intel(R) Xeon(R) CPU E5-2420 0 @ 1.90GHz
    内存：32 GB
    硬盘：ssd raid5
    网卡：千兆网卡
    集群机器个数： 3个
    集群间机器PING值： 0.05ms
    请求写入并发：100个线程
    
### 性能测试结果(qps)
> 请求延时小于10ms.
###### 写入小数据(100B)
    1个Group： 1171
    20个Group： 11931
    50个Group： 13424
    100个Group： 13962
###### 写入大数据(100KB)
    1个Group： 280
    20个Group： 984
    50个Group： 1054
    100个Group： 1067
###### BatchPropose(2KB)
    100个Group: 150000

# 代码目录介绍
**include**目录包含了使用PhxPaxos所需要用到的所有头文件，您需要理解这些头文件的所有类函数的含义，才能正确的使用PhxPaxos。
>注：我们对外公共API都放在这个目录的头文件里，调用者切勿引用非此目录的头文件的一些内部API，
这些API我们有可能会随时的进行调整而不进行兼容。

**src**目录是PhxPaxos的源代码目录，如想深入研究PhxPaxos的工作原理，可详细阅读此目录的代码。
如果你只是想使用PhxPaxos，则暂时不需要理解此目录代码。 

**third_party**目录用于放置PhxPaxos所需要用到的一些第三方库；一般刚获得PhxPaxos源代码的时候，这是一个空目录。
如何放置第三方库在后面编译方面的章节会详细介绍。使用PhxPaxos需要依赖到两个第三方库，分别是protobuf和leveldb。

**plugin**目录提供日志插件模块。日志是调试程序的重要方式，但不同组织之间的日志功能通常有很大的区别，
因此PhxPaxos的src目录并未提供具体的日志功能，而是提供了日志的插件机制，使得日志功能可以被定制。为了方便大家快速尝试PhxPaxos库，我们基于目前比较流行的glog库实现了一个日志插件。如果你刚好也使用glog，那么plugin目录的代码可以为你减少一些开发时间。

**sample**目录提供了基于PhxPaxos实现的三个程序。这三个程序分别对应了使用PhxPaxos的由浅入深，从简单到入门到进阶的不同阶段。
 * PhxElection是一个极为简单的程序，它利用了PhxPaxos的内置Master选举功能，实现了一个选举Master的程序。
 * PhxEcho展示了如何编写一个状态机，并和Phxpaxos结合。
 * PhxKV则是一个更为完整的系统，他实现了一个KV的状态机，搭配PhxPaxos实现了分布式KV存储，并展示了如何实现Checkpoint来删除paxos log；
 另外它同时展示了怎么将这些代码整合到一个RPC框架（我们使用了grpc作为演示），最终实现一个完整的分布式后台存储系统。
 
# 公共头文件介绍
 * **include/node.h** PhxPaxos的主要API在这里，建议开发者从这里开始。
 * **include/options.h** 运行PhxPaxos所需的一些配置以及可定制的选项。
 * **include/sm.h** 状态机的基类。
 * **include/def.h** 返回值定义。
 * **include/network.h** 网络模块的抽象函数，如果您想使用自己的网络协议，通过重载这些函数实现网络模块的自定义。
 * **include/storage.h** 存储模块的抽象函数。
 * **include/log.h** 日志模块的抽象函数。
 * **include/breakpoint.h** 断点抽象函数，一般可用于实现自己的监控。
 
# 如何编译
### 编译前的第三方库准备
首先我们看一下各目录的依赖关系。如下：

| 目录               | 编译对象             | 内部依赖                           | 第三方库依赖     |
| ------------------ | -------------------- | ---------------------------------- | ---------------- |
| 根目录             | libphxpaxos.a        | 无                                 | protobuf,leveldb |
| plugin             | libphxpaxos_plugin.a | libphxpaxos.a                      | glog             |
| sample/phxelection | 可执行程序           | libphxpaxos.a,libphxpaxos_plugin.a | 无               |
| sample/phxecho     | 可执行程序           | libphxpaxos.a,libphxpaxos_plugin.a | 无               |
| sample/phxkv       | 可执行程序           | libphxpaxos.a,libphxpaxos_plugin.a | grpc             |
| src/ut             | 单元测试             | 无                                 | gtest,gmock      |

编译我们的Phxpaxo(libphxpaxos.a)类库，只需要protobuf和leveldb两个第三方库；而编译其他目录则需要glog和grpc这两个库。
在编译前，需要先准备好这些第三方库，放在我们的third_party目录，可以直接放置，也可以通过软链的形式，也可以git clone时加上--recursive参数获取third_party目录下所有的submodule。

### 编译环境
 * Linux。
 * GCC-4.8及以上版本。
 
### 编译安装方法
###### 编译libphxpaxos.a
 
在PhxPaxos根目录下
```Bash
./autoinstall.sh
make
make install
```

###### 编译libphxpaxos_plugin.a
 
在plugin目录下
```Bash
make
make install
```

# 如何嵌入PhxPaxos到自己的代码
### 选择一个单机服务
我们选用sample目录里面的PhxEcho来说明PhxPaxos的使用方法。Echo是我们编写RPC服务的常见测试函数，
我们尝试通过嵌入PhxPaxos的代码，使得我们的Echo可以扩展到多台机器。

假设我们现在已有的EchoServer代码头文件类定义如下：
```c++
class PhxEchoServer
{
public:
    PhxEchoServer();
    ~PhxEchoServer();

    int Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue);
};
```
接下来我们基于这个类来嵌入PhxPaxos的代码。

### 实现一个状态机
首先我们定义一个状态机叫PhxEchoSM，这个类继承自StateMachine类，如下：
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
因为一个PhxPaxos可以同时挂载多个状态机，所以需要SMID()这个函数返回这个状态机的唯一标识ID。

其中Execute为状态机状态转移函数，输入为sPaxosValue， PhxPaxos保证多台机器都会执行相同系列的Execute(sPaxosValue)，
从而获得强一致性。函数的实现如下：
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
我们仅仅print一下这个sPaxosValue，作为多机化的一个验证。

函数的参数出现了一个陌生的类型SMCtx，如下：
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
SMCtx类型参数作为一个上下文，由写入者提供（怎么提供后面会提到），并由PhxPaxos带到Execute函数，最终传递给用户使用。

m_iSMID与上文提到的SMID()函数相对应，PhxPaxos会将这个上下文带给SMID()等于m_iSMID的状态机。
m_pCtx则记录了用户自定义的上下文数据的所在地址。

>Execute函数的上下文参数仅在请求写入所在进程可以获得，在其他机器这个指针为nullptr，
所以Execute在处理这个参数的时候注意要进行空指针的判断。

下面展示了我们的Echo上下文数据类型定义：
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
通过iExecuteRet可以获得Execute的执行情况，通过sEchoRespValue可以获得Execute带入的sEchoReqValue。

最终由以上几个类，我们构建了自己的状态机以及状态转移函数。

>本小节要点：如果你想改造一个现有的服务模块使其多副本化，那么你要做的仅仅就是抽象你的服务逻辑，
使其变成一个Execute函数，仅此而已。

### 运行PhxPaxos
在编写好Echo状态机之后，接下来要做的就是运行PhxPaxos，并且挂载上状态机。

首先我们对原有的EchoServer类进行一下修改，如下：
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
构造函数增加了几个参数，oMyNode标识本机的IP/PORT信息，vecNodeList标识多副本集群的所有机器信息，
这些参数类型都是PhxPaxos的预设类型。

私有成员方面，除了m_oMyNode和m_vecNodeList之外，m_oEchoSM是我们刚刚编写的状态机类，m_poPaxosNode则代表
了本次我们需要运行的PhxPaxos实例指针。

我们通过调用RunPaxos函数来运行PhxPaxos实例。这个函数的实现如下：
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
Option类型变量包含了运行这个PhxPaxos实例的所有参数以及选项。

MakeLogStoragePath函数生成我们存放PhxPaxos产生的数据的目录路径，设置给oOptions.sLogStoragePath。
设置oOptions.iGroupCount为1，标识我们想同时运行多少个PhxPaxos实例。我们通过GroupIdx来标识实例，
其范围为[0,oOptions.iGroupCount)，我们支持并行运行多个实例，每个实例独立运作。不同实例直接无任何关联，
支持多实例的目的仅仅是为了让它们可以共享同一个IP/PORT。

然后设置好本机IP/PORT信息以及所有机器的信息。

接下来非常重要的一步，设置我们刚才实现的状态机。

oOptions.vecGroupSMInfoList描述了多个PhxPaxos实例对应的状态机列表。他是一个GroupSMInfo类的数组。

GroupSMInfo类型，用于描述一个PhxPaxos实例对应的状态机列表，GroupSMInfo.iGroupIdx标识这个实例，由于我们的GroupCount设置为1，
所以GroupIdx设置为0，vecSMList标识需要挂载的状态机列表，它是一个状态机类型指针的数组。

设置好我们的日志函数，这里我们用了plugin目录的glog日志方法。

最后，调用Node::RunNode(oOptions, m_poPaxosNode)，传入参数选项，如果运行成功，函数返回0，
并且m_poPaxosNode指向这个运行中的PhxPaxos实例。这样PhxPaxos实例就运行成功了。

# 发起请求
下面展示改造后的EchoServer的Echo函数，从而告诉大家如何使用PhxPaxos来发起写入请求，如下：
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
首先定义上下文类型变量oEchoSMCtx，然后将这个变量指针设置到状态机上下文oCtx.m_pCtx里面，
同时我们设置oCtx.m_iSMID为1，与我们刚刚编写的状态机的SMID()相对应，标识我们需要将这个请求送往SMID为1的状态机的Execute函数。

然后调用m_poPaxosNode->Propose，第一个参数GroupIdx填入0，代表我们希望对实例0进行写入请求，第二个参数填入我们的请求内容。
llInstanceID 是我们获得的回参，这个ID是一个全局递增的ID，最后一个参数传入我们的上下文。

如果执行成功则函数返回0，通过上下文即可获得sEchoRespValue。

经过以上几个步骤，我们就将一个单机的Echo函数，改造成了一个多机版本。

### 运行效果
下面展示我们编写的多副本Echo服务的运行效果。你可以照着上面的思路自己实现一遍，或者直接编译我们的sample/phxecho目录的程序。

我们运行一个拥有三副本的phxecho集群，其中第一台机器：
```c++
run paxos ok
echo server start, ip 127.0.0.1 port 11112

please input: <echo req value>
hello phxpaxos:)
[SM Execute] ok, smid 1 instanceid 0 value hello phxpaxos:)
echo resp value hello phxpaxos:)
```
IP为127.0.0.1，PORT为11112，可以看到我们给其输入一个EchoReqValue为”hello phxpaxos:)”，
然后我们看到Execute的printf打出来的[SM Execute] ok...，最后我们通过上下文获得EchoRespValue为相同的”hello phxpaxos:)”。

我们来看看其他副本机器的情况，这是第二台机器：
```c++
run paxos ok
echo server start, ip 127.0.0.1 port 11113

please input: <echo req value>
[SM Execute] ok, smid 1 instanceid 0 value hello phxpaxos:)
```
IP为127.0.0.1，PORT为11113， 可以看到他的Execute函数的信息也打印的了出来，value同样为”hello phxpaxos:)”。

# 使用PhxPaxos的Master，给你的Server提供一个选举功能
这里先解释一下Master的定义，Master是指在多台机器构建的集合里面，任一时刻，只有一台机器认为自己是Master或者没有任何机器认为自己是Master。

这个功能非常实用。假设有那么一个多台机器组成的集群，我希望任一时刻只有一台机器在提供服务，相信大家可能会遇到这样的场景，
而通常的做法可能是使用ZooKeeper来搭建分布式锁。那么使用我们的Master功能，只需编写短短的几十行代码，
即可跟你现有的服务无缝结合起来，而不用引入额外的一些庞大的模块。

下面展示如何嵌入Master到自己的代码里面。

首先我们构建一个选举类PhxElection，这个类供已有的模块代码使用，如下：
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
这个类提供两个功能函数，GetMaster获得当前集群的Master，IsIMMaster判断自己是否当前Master。

RunPaxos是运行PhxPaxos的函数，代码如下：

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
与Echo不一样的是，这次我们并不需要实现自己的状态机，而是通过将oSMInfo.bIsUseMaster设置为true，开启我们内置的一个Master状态机。
相同的，通过Node::RunNode即可获得PhxPaxos的实例指针。通过SetMasterLease可以随时修改Master的租约时间。
最后，我们通过这个指针获得集群的Master信息，代码如下：

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
通过这个简单的选举类，每台机器都可以获知当前的Master信息。

# 欢迎使用，欢迎反馈你们的建议:)
