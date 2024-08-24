/*
    buffer和其它模块是解耦合的
*/
#pragma once

#include <vector>
#include <string>
#include <algorithm>

/*
    网络库底层的缓冲器类型定义
    cheapPrepend ---- read ---------write
        8          |              |         
                readIndex         writeIndex
*/
class Buffer{

public:

    //设置了一个字节的prepend， 缓冲区初始设置为 1024bit 的大小
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    
    explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + initialSize)
            , readerIndex_(kCheapPrepend)
            , writerIndex_(kCheapPrepend)
    {}

    //还有多少数据可以读
    size_t readableBytes() const {return writerIndex_ - readerIndex_;};

    //还有多少缓冲区keyixie
    size_t writableBytes() const {return buffer_.size() - writerIndex_;};

    //prepend有多大
    size_t prependableBytes() const {return readerIndex_;};

    //返回缓冲区的可读数据的起始地址
    const char* peek() const { return begin() + readerIndex_;};

    /*
        读取了缓冲区的长度为len的数据
        如果只读了 部分数据没有全部读完， 只需要修改readerIndex即可
        如果把数据全部读完了， 那么调用retrieveAll, 重置所有指针
    */
    void retrieve(size_t len){
        if(len < readableBytes()){
            readerIndex_ += len;
        }
        else{ // len == readableBytes()
            retrieveAll();
        }
    }
    void retrieveAll() {readerIndex_ = writerIndex_ = kCheapPrepend;};

    //把可读数据中全部数据转换为string类型， 并且重置指针
    std::string retrieveAllAsString() {return retrieveAsString(readableBytes());};
    //把可读数据中长度为len的数据转换为string类型， 并且重置指针
    std::string retrieveAsString(size_t len){
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    //确认buffer中是否还能写入长度为len的数据， 不然扩容vector
    void ensureWriteableBytes(size_t len){
        if(writableBytes() < len){
            makeSpace(len);
        }
    }

    // 把[data, data+len]内存上的数据，添加到writable缓冲区当中
    void append(const char *data, size_t len){
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    // 从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);
    // 通过fd发送数据
    ssize_t writeFd(int fd, int* saveErrno);
private:

    //就是堆vector数组的vector.begin() 的封装
    char* begin() { return &*buffer_.begin();};
    const char* begin() const {return &*buffer_.begin();};

    /*
        扩容vector-》buffer 的函数
    */
    void makeSpace(size_t len){
        if(writableBytes() + prependableBytes() < len + kCheapPrepend){
            /*
                writableBytes() + prependableBytes() 是 buffer中除了未被读取
                的数据之外的所有空间
                如果这还是小于 len + kCheapPrepend，
                说明 vector 一定要扩容，
                扩容到 writerIndex+len, 正好可以继续添加len长度的数据
                否则， 说明 现有的buffer含可以填入长度len的数据， 只要把
                未读取的数据前移就可以
            */
            buffer_.resize(writerIndex_ + len);
        }
        else{
            size_t readalbe = readableBytes();
            std::copy(begin() + readerIndex_, 
                    begin() + writerIndex_,
                    begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readalbe;
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;//还可以读的数据的开始
    size_t writerIndex_;//还可以写的缓冲区的开始
};