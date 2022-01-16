#include "test/test.h"
#include "os/pcb.h"

int main() {
    ensure("test1 returns 1") {
        assert(test1() == 1);
    }
}
