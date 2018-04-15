#include "machine_list.h"

void MachineList::make_available(Machine *machine) {
    {
        std::lock_guard<std::mutex> lock(this->upd);
        this->avail_list.push_back(machine);
    }

    this->avail_cnt.notify();
}

Machine* MachineList::get() {
    this->avail_cnt.wait();

    {
        std::lock_guard<std::mutex> lock(this->upd);
        Machine *res = this->avail_list.front();
        this->avail_list.pop_front();
        return res;
    }
}
