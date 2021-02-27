#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include <Windows.h>
#include <functional>
#include <vector>
#include "worker.h"

#define SCHEDULER_DELAY 100

typedef struct {
    Worker* m_pWorker;
    int m_iTimer;
} schedworker_t;

typedef std::function<Worker*(void*)> CreateFunc;

class Scheduler : public ThreadWorker
{
public:
    Scheduler(CreateFunc func, unsigned uMaxWorkers);
    ~Scheduler();

    void DoControl() override;
    void DoWork() override;
protected:
    void WorkerStart(Worker* wrk);
    void WorkerUpdate(Worker* wrk);
    void WorkerDestroy(Worker* wrk);
private:
    CreateFunc m_Create;
    unsigned m_uMaxWorkers;
    std::vector<schedworker_t> m_Workers;
};

#endif
