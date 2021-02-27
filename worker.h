#ifndef __WORKER_H
#define __WORKER_H

#include <Windows.h>
#include <functional>
#include <exception>
#include <string>

/* WaitObject */

typedef std::function<bool(void*)> WaitFunc;

class WaitObject
{
public:
    enum : int { NoWait, WaitInterval, WaitFunction };

    WaitObject();
    WaitObject(long lInterval);
    WaitObject(WaitFunc func, void* data, long delay);

    int m_Type;
    long m_lInterval;
    WaitFunc m_Func;
    void* m_pData;
};

extern const WaitObject NoWait;
extern const WaitObject DefWait;

/* Worker */

class Worker;

/*typedef enum class {
    STATE_FAIL = -1,
    STATE_INIT,
    STATE_WAIT,
    STATE_RUNNING,
    STATE_FINISH,
} workstate;*/
/*enum class workstate {
    STATE_FAIL = -1,
    STATE_INIT,
    STATE_WAIT,
    STATE_RUNNING,
    STATE_FINISH
};*/

enum class workstate {
    FAIL = -1,
    INIT,
    WAIT,
    RUNNING,
    FINISH
};

typedef std::function<void(Worker*)> WorkerFunc;

class Worker
{
public:
    Worker();
    virtual ~Worker();

    virtual void SetName(const std::wstring& name);
    virtual std::wstring GetName() const;

    virtual void SetData(void* pData);
    virtual void* GetData() const;

    virtual void SetControlFunction(WorkerFunc func);
    virtual void SetWorkerFunction(WorkerFunc func);
    virtual void SetUpdateFunction(WorkerFunc func);

    virtual void SetState(workstate state, bool bRunning);

    virtual workstate GetState() const;
    virtual bool IsRunning() const;

    virtual std::string& GetExitMessage();
    virtual WaitObject& GetWait();

    virtual void SetStage(int stage);
    virtual int GetStage() const;

    virtual int GetProgress() const;

    virtual void EnableThink(bool bThink);
    virtual void Start();

    virtual void Fail(const std::string& msg);
    virtual void Wait(const WaitObject& obj);
    virtual void Continue(int nextStage = -1);

    virtual void Finish();

    virtual void OnException(const std::exception& exc);

    virtual void DoControl();
    virtual void DoWork();
protected:
    virtual void Think();
    void DoWait();
public:
    virtual void Update();
    virtual void Run();
private:
    std::wstring m_Name;

    void* m_pData;

    WorkerFunc m_Control;
    WorkerFunc m_Work;
    WorkerFunc m_Update;

    workstate m_State;
    bool m_bRunning;
    int m_iStage;
    std::string m_Msg;

    WaitObject m_Wait;

    bool m_bThink;
};

/* ThreadWorker */

class ThreadWorker : public Worker
{
public:
    ThreadWorker();
    virtual ~ThreadWorker();

    virtual unsigned long GetThreadId() const;

    virtual void Start();
    void JoinThread();
private:
    static DWORD WINAPI _ThreadWorker(LPVOID);

    unsigned long m_ulThreadId;
    HANDLE m_hThread;
};

#endif
