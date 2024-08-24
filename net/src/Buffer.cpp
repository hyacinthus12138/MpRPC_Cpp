#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>


/*
    从指定的fd上接收数据到 buffer
    值得注意的是
    buffer缓冲区的大小是有限的， 但是从fd上读取数据却是无法确定的
*/
ssize_t Buffer::readFd(int fd, int* saveErrno){
    char extrabuf[65536] = {0}; //这是一个开辟在栈上的内存空间， 64k
    const size_t writable = writableBytes();

    /*
        下面这段代码就是readv函数的应用， 把fd上的数据读取到 buffer 和 extrabuff 这两个缓冲区上

    */
    struct iovec vec[2];
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if(n < 0){
        *saveErrno = errno;
    }
    else if(n <= writable){//说明单纯的buffer已经够用了， 不需要extrabuff，只有修改指针就可以
        writerIndex_ += n;
    }
    else{ //这表示buffer不够用， extrabuff 里面也有数据
        writerIndex_ = buffer_.size();
        //这里就是把extrabuff里里面的数据写入buffer， append会把buffer扩容
        append(extrabuf, n - writable);
    }
    return n;

}

/*
    把buffer里面的数据写入对应的fd
*/
ssize_t Buffer::writeFd(int fd, int *saveErrno){
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0){
        *saveErrno = errno;
    }
    return n;
}