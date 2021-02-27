#include "worker.h"

/* WaitObject */

WaitObject::WaitObject()
    : m_Type(WaitObject::NoWait)
{
    m_lInterval = 0;
    m_Func = NULL;
    m_pData = NULL;
}

WaitObject::WaitObject(long lInterval)
    : m_Type(WaitObject::WaitInterval)
{
    m_lInterval = lInterval > 0 ? lInterval : 0;
    m_Func = NULL;
    m_pData = NULL;
}

WaitObject::WaitObject(WaitFunc func, void* data, long delay)
    : m_Type(WaitObject::WaitFunction)
{
    m_lInterval = delay > 0 ? delay : 0;
    m_Func = func;
    m_pData = data;
}

const WaitObject NoWait = WaitObject();
const WaitObject DefWait = WaitObject(100);

/* Worker */

Worker::Worker()
{
    m_pData = NULL;
    m_bThink = false;
    m_iStage = 0;
    m_State = workstate::INIT;
    m_bRunning = false;
}

Worker::~Worker()
{
}

void Worker::SetName(const std::wstring& name)
{
    m_Name = name;
}

std::wstring Worker::GetName() const
{
    return m_Name;
}

void Worker::SetData(void* data)
{
    m_pData = data;
}

void* Worker::GetData() const
{
    return m_pData;
}

void Worker::SetControlFunction(WorkerFunc func)
{
    m_Control = func;
}

void Worker::SetWorkerFunction(WorkerFunc func)
{
    m_Work = func;
}

void Worker::SetUpdateFunction(WorkerFunc func)
{
    m_Update = func;
}

void Worker::SetState(workstate state, bool bRunning)
{
    if (m_State == workstate::WAIT && state != m_State)
        m_Wait = NoWait;

    m_State = state;
    m_bRunning = bRunning;
}

workstate Worker::GetState() const
{
    return m_State;
}

bool Worker::IsRunning() const
{
    return m_bRunning;
}

std::string& Worker::GetExitMessage()
{
    return m_Msg;
}

WaitObject& Worker::GetWait()
{
    return m_Wait;
}

void Worker::SetStage(int stage)
{
    m_iStage = stage;
}

int Worker::GetStage() const
{
    return m_iStage;
}

int Worker::GetProgress() const
{
    return -1;
}

void Worker::EnableThink(bool bThink)
{
    m_bThink = bThink;
}

void Worker::Start()
{
    SetState(workstate::INIT, true);

    Update();
}

void Worker::Fail(const std::string& msg)
{
    SetState(workstate::FAIL, false);
    m_Msg = msg;
}

void Worker::Wait(const WaitObject& obj)
{
    SetState(workstate::WAIT, true);
    m_Wait = obj;
}

void Worker::Continue(int nextStage)
{
    SetState(workstate::RUNNING, true);
    if (nextStage != -1)
        SetStage(nextStage);
}

void Worker::Finish()
{
    SetState(workstate::FINISH, false);
}

void Worker::OnException(const std::exception& exc)
{
    fprintf(stderr, "%s\n", exc.what());
    Fail(exc.what());
}

void Worker::DoControl()
{
    if (m_Control)
        m_Control(this);
}

void Worker::DoWork()
{
    if (m_Work)
        m_Work(this);
}

void Worker::Update()
{
    try {
        if (m_Update)
            m_Update(this);

        DoControl();
        if (m_bThink)
            Think();

        if (IsRunning() && GetState() != workstate::WAIT)
            DoWork();
    } catch (const std::exception& exc) {
        OnException(exc);
    }
}

void Worker::Think()
{
    switch(GetState())
    {
    case workstate::WAIT: DoWait(); break;
    default: return;
    }
}

void Worker::DoWait()
{
    WaitObject& obj = GetWait();
    switch(obj.m_Type)
    {
    case WaitObject::NoWait:
        SetState(workstate::RUNNING, true);
        break;
    case WaitObject::WaitInterval:
        Sleep(obj.m_lInterval);
        SetState(workstate::RUNNING, true);
        break;
    case WaitObject::WaitFunction:
        if (obj.m_Func(obj.m_pData))
            SetState(workstate::RUNNING, true);
        else Sleep(obj.m_lInterval);
        break;
    }
}

void Worker::Run()
{
    do {
        Update();
    } while (IsRunning());
}

/* ThreadWorker */

ThreadWorker::ThreadWorker()
{
    m_hThread = INVALID_HANDLE_VALUE;
    m_ulThreadId = 0;
}

ThreadWorker::~ThreadWorker()
{
}

unsigned long ThreadWorker::GetThreadId() const
{
    return m_ulThreadId;
}

DWORD WINAPI ThreadWorker::_ThreadWorker(LPVOID lpArg)
{
    ThreadWorker* wrk = (ThreadWorker*)lpArg;

    wrk->SetState(workstate::INIT, false);
    wrk->EnableThink(true);
    wrk->Update();

    wrk->Run();
    return !(wrk->GetState() == workstate::FINISH);
}

void ThreadWorker::Start()
{
    DWORD dwTid;
    m_hThread = CreateThread(NULL, 0, _ThreadWorker, this, 0, &dwTid);
    m_ulThreadId = dwTid;
}

void ThreadWorker::JoinThread()
{
    WaitForSingleObject(m_hThread, INFINITE);
}
