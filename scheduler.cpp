#include "scheduler.h"

Scheduler::Scheduler(CreateFunc func, unsigned uMaxWorkers)
{
    m_Create = func;
    m_uMaxWorkers = uMaxWorkers;
}

Scheduler::~Scheduler()
{
    for (auto& worker : m_Workers)
        WorkerDestroy(worker.m_pWorker);
}

void Scheduler::DoControl()
{
    if(GetState() == workstate::INIT)
        Continue(0);
    else Wait(WaitObject(SCHEDULER_DELAY));
}

void Scheduler::DoWork()
{
    for (auto it = m_Workers.begin(); it != m_Workers.end(); ++it)
    {
        Worker* wrk = it->m_pWorker;
        printf("wrk %p state %d timer %d\n", wrk,
                wrk->GetState(), it->m_iTimer);

        try {
            if (wrk->GetState() == workstate::INIT)
                wrk->SetState(workstate::INIT, true);

            if (wrk->IsRunning())
            {
                if (!it->m_iTimer)
                    wrk->DoControl();
            }
            else continue;

            if (wrk->GetState() == workstate::WAIT)
            {
                WaitObject& obj = wrk->GetWait();
                if (!it->m_iTimer)
                {
                    switch(obj.m_Type)
                    {
                    case WaitObject::NoWait:
                        wrk->SetState(workstate::RUNNING, true);
                        break;
                    case WaitObject::WaitInterval:
                        it->m_iTimer = (int)obj.m_lInterval;
                        break;
                    case WaitObject::WaitFunction:
                        printf("WaitFunction\n");
                        if (obj.m_Func(obj.m_pData))
                        {
                            printf("OK WaitFunction\n");
                            wrk->SetState(workstate::RUNNING, true);
                            it->m_iTimer = 0;
                        }
                        else it->m_iTimer = (int)obj.m_lInterval;
                        break;
                    }
                }
                else
                {
                    it->m_iTimer -= SCHEDULER_DELAY;
                    if (it->m_iTimer <= 0)
                    {
                        if (obj.m_Type != WaitObject::WaitFunction)
                            wrk->SetState(workstate::RUNNING, true);
                        it->m_iTimer = 0;
                    }
                }
            }

            if (wrk->IsRunning() && wrk->GetState() != workstate::WAIT)
                wrk->DoWork();
        } catch (const std::exception& exc) {
            wrk->OnException(exc);
        }
    }

    for (auto it = m_Workers.begin(); it != m_Workers.end();)
    {
        Worker* wrk = it->m_pWorker;
        if (!wrk->IsRunning())
        {
            WorkerDestroy(wrk);
            m_Workers.erase(it);
        }
        else ++it;
    }

    if (GetStage() == 0)
    {
        while(m_Workers.size() < m_uMaxWorkers)
        {
            Worker* wrk = m_Create(GetData());
            if (wrk)
                WorkerStart(wrk);
            else
            {
                Continue(1);
                break;
            }
        }
    }
    else if (GetStage() == 1 && m_Workers.empty())
        Finish();
}

void Scheduler::WorkerStart(Worker* wrk)
{
    printf("WorkerStart %p\n", wrk);
    wrk->EnableThink(false);

    schedworker_t sched = { wrk, 0 };
    m_Workers.push_back(sched);
}

void Scheduler::WorkerUpdate(Worker* wrk)
{
    printf("WorkerUpdate %p\n", wrk);
    wrk->Update();

}

void Scheduler::WorkerDestroy(Worker* wrk)
{
    printf("WorkerDestroy %p %s\n", wrk, wrk->GetExitMessage().c_str());
    delete wrk;
}
