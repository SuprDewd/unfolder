#ifndef UNFOLDER_MACHINE_LIST_H
#define UNFOLDER_MACHINE_LIST_H

#include "semaphore.h"
#include <list>
#include <mutex>

class MachineList;
#include "machine.h"

class MachineList {
private:
    semaphore avail_cnt;
    std::list<Machine*> avail_list;
    std::mutex upd;

public:
    void make_available(Machine *machine);
    Machine* get();
};

#endif
