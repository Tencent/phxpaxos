简单的[移植](https://github.com/boolking/phxpaxos/tree/win_port)了一下，已经可以支持windows了。
Unit test和echo，election都可以正常运行。

针对windows做了如下修改：
1. 使用gyp简化编译流程，不需要单独编译第三方库。linux下使用[gyp+ninja替代make](https://github.com/boolking/phxpaxos/blob/win_port/gyp_build.sh)，windows[使用vc2017](https://github.com/boolking/phxpaxos/blob/win_port/gyp_build.bat)；
2. 在windows使用select简单模拟了一下poll/epoll的行为，并修改了SocketBase等相关的一些实现来支持windows；使用Windows API模拟fsync/fdatasync/truncate；使用rmdir()替代remove()来删除空目录；由于windows下select只支持socket，使用socket pair替代pipe实现Notify；
3. 使用std::thread替代pthread，移除了一些没有使用的线程函数和ThreadAttr类，使用rand_s替代rand_r，移除了Unix domain socket的支持；
4. **由于[std::unique_lock跨线程使用不安全](https://stackoverflow.com/questions/18937564/unique-lock-across-threads)，使用std::condition_variable_any+std::mutex替代std::condition_variable+std::unique_lock+std::mutex，还能够节省一次函数调用；**
5. 使用__VA_ARGS__替代args...，使用%PRIu64替代%llu和%lu；
6. 添加了leveldb的windows支持，更新gtest/gmock到1.8.0，修正了glog在vc2017下的一些编译错误；
7. 修改[travis.ci](https://travis-ci.org/boolking/phxpaxos)的配置文件，添加了[appveyor](https://ci.appveyor.com/project/boolking/phxpaxos)的配置。windows和linux都可以正常编译。