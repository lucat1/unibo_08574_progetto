#include "os/pcb.h"
#include "os/scheduler.h"
#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"

int main()
{
    mock_init();
    it("sanitizes the input parameter")
    {
        active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(GETPROCESSID, (bool)-2, 0, 0);
        assert(active_process->p_pid == NULL_PID);

        active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(GETPROCESSID, 2, 0, 0);
        assert(active_process->p_pid == NULL_PID);

        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, false, 0, 0);
        assert(active_process->p_pid != NULL_PID);

        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, true, 0, 0);
        assert(active_process->p_pid != NULL_PID);
    }
    it("returns its pid")
    {
        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, false, 0, 0);
        assert(active_process->p_s.reg_v0 == active_process->p_pid);
        kill_progeny(active_process);
    }
    it("returns its parent's pid")
    {
        pcb_t *parent = spawn_process(false);
        active_process = spawn_process(false);
        insert_child(parent, active_process);
        SYSCALL(GETPROCESSID, true, 0, 0);
        assert(active_process->p_s.reg_v0 == parent->p_pid);
        kill_progeny(parent);
    }
    it("returns zero if it has no parent")
    {
        active_process = spawn_process(false);
        SYSCALL(GETPROCESSID, true, 0, 0);
        assert(active_process->p_s.reg_v0 == 0);
        kill_progeny(active_process);
    }
    return 0;
}
