#include"rpccontroller.h"


rpccontroller::rpccontroller()
{
    m_failed = false;
    m_errText = "";
}

void rpccontroller::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool rpccontroller::Failed() const
{
    return m_failed;
}

std::string rpccontroller::ErrorText() const
{
    return m_errText;
}

void rpccontroller::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}

// 目前未实现具体的功能
void rpccontroller::StartCancel(){}
bool rpccontroller::IsCanceled() const {return false;}
void rpccontroller::NotifyOnCancel(google::protobuf::Closure* callback) {}