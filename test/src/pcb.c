/** \file
 * \brief Tests concerning \ref pcb.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/pcb.h"
#include "os/util.h"
#include "test/test.h"

bool pcb_free_contains(pcb_t *p)
{
    if (p == NULL || &p->p_list == NULL) {
        return false;
    }
    return list_contains(&p->p_list, get_pcb_free());
}

int main()
{
    pcb_t *pcb1, *pcb2, *pcb5;
    pcb_t *pcb_child1, *pcb_child2, *pcb_child3;
    init_pcbs();
    it("correctly allocated MAX_PROC PCBs")
    {
        for (int i = 0; i < MAX_PROC; i++) {
            pcb2 = pcb1;
            assert((pcb1 = alloc_pcb()) != NULL);

            /* Check if the pcb is null */
            assert(list_empty(&pcb1->p_list));
            assert(list_empty(&pcb1->p_child));
            assert(list_empty(&pcb1->p_sib));
            assert(pcb1->p_parent == NULL);
            assert(pcb1->p_semAdd == NULL);
            assert(pcb1->p_time == 0);
            assert(pcb1->p_s.entry_hi == 0);
            assert(pcb1->p_s.cause == 0);
            assert(pcb1->p_s.status == UNINSTALLED);
            assert(pcb1->p_s.pc_epc == 0);
            assert(pcb1->p_s.hi == 0);
            assert(pcb1->p_s.lo == 0);
            for (int i = 0; i < STATE_GPR_LEN; i++) {
                assert(pcb1->p_s.gpr[i] == 0);
            }

            /* useful for next testing */
            if (i == 4)
                pcb5 = pcb1;
            if (i == 5)
                pcb_child1 = pcb1;
            if (i == 6)
                pcb_child2 = pcb1;
            if (i == 7)
                pcb_child3 = pcb1;
        }
        assert(pcb_child1 != pcb_child2);
    }
    ensure("when pcb_free is empty alloc_pcb should return NULL")
    {
        assert(alloc_pcb() == NULL);
    }
    it("correctly add a PCB to the pcb_free list")
    {
        free_pcb(pcb2);
        assert(pcb_free_contains(pcb2));
        assert((alloc_pcb()) != NULL);
        assert((alloc_pcb()) == NULL);
    }

    ensure("free_pcb does not crash when the user input is NULL or already "
           "contained")
    {
        /* add pcb2 to pcb_free */
        free_pcb(pcb2);
        /* pcb_size should be 1 */
        int pcb_size = list_size(get_pcb_free());
        free_pcb(NULL);
        assert(list_contains(&pcb2->p_list, get_pcb_free()));
        /* this should be useless */
        free_pcb(pcb2);
        assert(pcb_size == list_size(get_pcb_free()));
        assert((alloc_pcb()) != NULL);
    }

    it("returns NULL if head in head_proc_q is NULL")
    {
        assert(head_proc_q(NULL) == NULL);
    }
    it("correctly created an empty PCBs list")
    {
        mk_empty_proc_q(&pcb1->p_list);
        /* check that p_list is initialized */
        assert(&pcb1->p_list != NULL);
        /* check that p_list is empty */
        assert(empty_proc_q(&pcb1->p_list));
        /* check that p_list is empty */
        assert(head_proc_q(&pcb1->p_list) == NULL);
        assert(list_empty(&pcb1->p_list));
    }

    it("correctly added PCB to p_list")
    {
        insert_proc_q(&pcb1->p_list, pcb2);
        assert(list_contains(&pcb1->p_list, &pcb2->p_list));
        insert_proc_q(&pcb1->p_list, pcb5);
        assert(list_contains(&pcb1->p_list, &pcb5->p_list));
        assert(list_size(&pcb1->p_list) == 2);
        /* check that p_list is not empty */
        assert(!empty_proc_q(&pcb1->p_list));
        /* check that p_list is not empty */
        assert(head_proc_q(&pcb1->p_list) != NULL);
        /* check that pcb2 is successfully added */
        assert(head_proc_q(&pcb1->p_list) == pcb2);
    }
    ensure("insert_proc_q does not crash when the user input is NULL")
    {
        int len = list_size(&pcb1->p_list);
        insert_proc_q(NULL, pcb1);
        insert_proc_q(&pcb1->p_list, NULL);
        assert(len == list_size(&pcb1->p_list));
    }

    ensure("remove_proc_q returns NULL when the head parameter is null")
    {
        assert(remove_proc_q(NULL) == NULL);
    }
    it("correctly removed first PCB from p_list")
    {
        /* assert(pcb2 != NULL && remove_proc_q(&pcb1->p_list)) == pcb5); */
        assert(remove_proc_q(&pcb1->p_list) == pcb2);
        assert(list_size(&pcb1->p_list) == 1);
        assert(remove_proc_q(&pcb1->p_list) == pcb5);
        assert(list_size(&pcb1->p_list) == 0);
        assert(!list_contains(&pcb2->p_list, &pcb1->p_list));
        assert(!list_contains(&pcb5->p_list, &pcb1->p_list));
    }
    ensure("fails on removing first PCB from p_list if empty")
    {
        assert(remove_proc_q(&pcb1->p_list) == NULL);
    }

    it("correctly removed arbitrary PCB from p_list")
    {
        insert_proc_q(&pcb1->p_list, pcb2);
        insert_proc_q(&pcb1->p_list, pcb5);

        assert(list_contains(&pcb5->p_list, &pcb1->p_list));
        assert(out_proc_q(&pcb1->p_list, pcb5) == pcb5);
        assert(!list_contains(&pcb5->p_list, &pcb1->p_list));

        assert(list_contains(&pcb2->p_list, &pcb1->p_list));
        assert(out_proc_q(&pcb1->p_list, pcb2) == pcb2);
        assert(!list_contains(&pcb2->p_list, &pcb1->p_list));
    }
    ensure("out_proc_q fails on removing PCB that doesn't exist from list_head")
    {
        assert(out_proc_q(&pcb1->p_list, pcb5) == NULL);
        assert(out_proc_q(&pcb1->p_list, pcb2) == NULL);
    }
    ensure(
        "out_proc_q returns NULL if the element or the head parameter is NULL")
    {
        assert(out_proc_q(NULL, pcb1) == NULL);
        assert(out_proc_q(&pcb1->p_list, NULL) == NULL);
    }

    ensure("that new PCB has not children") { assert(empty_child(pcb1)); }
    ensure("empty_child returns true if the parameter p is NULL")
    {
        assert(empty_child(NULL));
    }

    it("correctly added PCB as child and empty_child returns false when the "
       "children list is not empty")
    {
        insert_child(pcb1, pcb_child1);
        assert(list_size(&pcb1->p_child) == 1);

        insert_child(pcb1, pcb_child2);
        assert(list_size(&pcb1->p_child) == 2);
        /* check that parent is correctly set */
        assert(pcb_child1->p_parent == pcb1);
        assert(pcb_child2->p_parent == pcb1);
        /* check that children list is not empty */
        assert(!empty_child(pcb1));

        /* check updated children list */
        assert(list_contains(&pcb_child1->p_sib, &pcb1->p_child));
        assert(list_contains(&pcb_child2->p_sib, &pcb1->p_child));

        /* check siblings list */
        assert(list_contains(&pcb_child2->p_sib, &pcb_child1->p_sib));
        assert(list_contains(&pcb_child1->p_sib, &pcb_child2->p_sib));
    }
    ensure("insert_child does not crash when the user input is NULL")
    {
        int len = list_size(&pcb1->p_child);
        insert_child(NULL, pcb_child1);
        insert_proc_q(&pcb1->p_child, NULL);
        assert(len == list_size(&pcb1->p_child));
    }
    ensure("insert_child does not add the element when it is already contained "
           "in the children list")
    {
        int len = list_size(&pcb1->p_child);
        insert_child(pcb1, pcb_child1);
        assert(list_size(&pcb1->p_child) == len);
        insert_child(pcb1, pcb_child2);
        insert_child(pcb1, pcb_child3);

        /* verifying order is same as previous */
        /* now should be 1 -> 2 -> 3 */
        list_head *tmp = list_next(&pcb1->p_child);
        assert(tmp == &pcb_child1->p_sib);
        tmp = list_next(tmp);
        assert(tmp == &pcb_child2->p_sib);
        tmp = list_next(tmp);
        assert(tmp == &pcb_child3->p_sib);
        assert(remove_child(pcb1) == pcb_child1);
        /* now should be 2 -> 3 */
        insert_child(pcb1, pcb_child1);
        /* now should be 2 -> 3 -> 1 */
        tmp = list_next(&pcb1->p_child);
        assert(tmp == &pcb_child2->p_sib);
        tmp = list_next(tmp);
        assert(tmp == &pcb_child3->p_sib);
        tmp = list_next(tmp);
        assert(tmp == &pcb_child1->p_sib);
    }

    it("correctly removed first child from PCB")
    {
        assert(remove_child(pcb1) == pcb_child2);
        assert(!list_contains(&pcb_child2->p_sib, &pcb1->p_child));
        assert(pcb_child2->p_parent == NULL);

        assert(remove_child(pcb1) == pcb_child3);
        assert(!list_contains(&pcb_child3->p_sib, &pcb1->p_child));
        assert(pcb_child3->p_parent == NULL);

        assert(remove_child(pcb1) == pcb_child1);
        assert(!list_contains(&pcb_child1->p_sib, &pcb1->p_child));
        assert(pcb_child1->p_parent == NULL);

        assert(list_empty(&pcb1->p_child));
    }

    ensure("fails on removing child if p_child is empty"){
        assert(remove_child(pcb1) == NULL)}

    it("correctly remove myself from p_child of p_parent")
    {

        insert_child(pcb1, pcb_child1);
        insert_child(pcb1, pcb_child2);

        assert(out_child(pcb_child1) == pcb_child1);
        assert(!list_empty(&pcb1->p_child));

        assert(out_child(pcb_child2) == pcb_child2);
        assert(pcb_child2->p_parent == NULL);
        assert(list_empty(&pcb1->p_child));
    }
    return 0;
}
