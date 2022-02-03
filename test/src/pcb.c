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

int main()
{

    pcb_t *pcb1, *pcb2, *pcb5;
    pcb_t *pcb_child1, *pcb_child2;
    init_pcbs();

    it("correctly allocated MAX_PROC PCBs")
    {
        for(int i = 0; i < MAX_PROC; i++) {
            pcb2 = pcb1;
            assert((pcb1 = alloc_pcb()) != NULL);

            // Check if the pcb is null
            assert(list_empty(&(pcb1->p_list)));
            assert(list_empty(&(pcb1->p_child)));
            assert(list_empty(&(pcb1->p_sib)));
            assert(pcb1->p_parent == NULL);
            assert(pcb1->p_semAdd == NULL);
            assert(pcb1->p_time == 0);
            assert(pcb1->p_s.entry_hi == 0);
            assert(pcb1->p_s.cause == 0);
            assert(pcb1->p_s.status == UNINSTALLED);
            assert(pcb1->p_s.pc_epc == 0);
            assert(pcb1->p_s.hi == 0);
            assert(pcb1->p_s.lo == 0);
            for(int i=0; i < STATE_GPR_LEN; i++){
                assert(pcb1->p_s.gpr[i] == 0);
            }

            // useful for next testing
            if (i == 4) pcb5 = pcb1;
            if (i == 5) pcb_child1 = pcb1;
            if (i == 6) pcb_child2 = pcb1;
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
    }
    
    it("correctly created an empty PCBs list") {
        mk_empty_proc_q(&(pcb1->p_list));
        // check that p_list is initialized
        assert(&(pcb1->p_list) != NULL)
        // check that p_list is empty
        assert(empty_proc_q(&(pcb1->p_list)) == 1);
        // check that p_list is empty 
        assert(head_proc_q(&(pcb1->p_list)) == NULL);
    }

    it("correctly added PCB to list_head") {
        insert_proc_q(&(pcb1->p_list), pcb2);
        assert(list_contains(&(pcb1->p_list), &(pcb2->p_list)));
        // check that p_list is not empty
        assert(empty_proc_q(&(pcb1->p_list)) == 0);
        // check that p_list is not empty
        assert(head_proc_q(&(pcb1->p_list)) != NULL);
        // check that pcb2 is successfully added
        assert(head_proc_q(&(pcb1->p_list)) == pcb2);
    }

    it("correctly removed first PCB from list_head") {
        assert(pcb2 != NULL && remove_proc_q(&(pcb1->p_list)) == pcb2);
        assert(!list_contains(&(pcb2->p_list), &(pcb1->p_list)));
    }
    ensure("fails on removing first PCB from list_head if empty"){
        assert(remove_proc_q(&(pcb1->p_list)) == NULL);
    }

    it("correctly removed arbitrary PCB from list_head") {
        insert_proc_q(&(pcb1->p_list), pcb2);
        insert_proc_q(&(pcb1->p_list), pcb5);
        assert(pcb5 != NULL);
        assert(list_contains(&(pcb5->p_list), &(pcb1->p_list)));
        assert(out_proc_q(&(pcb1->p_list), pcb5) == pcb5);
        assert(!list_contains(&(pcb5->p_list), &(pcb1->p_list)));
    }
    ensure("fails on removing PCB that doesn't exist from list_head") {
        assert(out_proc_q(&(pcb1->p_list), pcb5) == NULL);
    }

    ensure("Check that new PCB has not child") {
        assert(empty_child(pcb1));
    }

    it("correctly added PCB as child") {
        insert_child(pcb1, pcb_child1);
        assert(list_size(&(pcb1->p_child)) == 1);
        //insert_child(pcb1, pcb_child2);
        //assert(list_size(&(pcb1->p_child)) == 2);
        // check that parent is correctly set
        assert(pcb_child1->p_parent == pcb1); 
        //assert(pcb_child2->p_parent == pcb1);  
        // check that child list is not empty
        assert(!empty_child(pcb1));
        assert(list_contains(&(pcb_child1->p_list), &(pcb1->p_child)))
        //assert(list_contains(&(pcb_child2->p_list), &(pcb1->p_child)))
    }

    it ("correctly removed first child from PCB"){
        assert(remove_child(pcb1) == pcb_child1)
        assert(!list_contains(&(pcb_child1->p_list), &(pcb1->p_child)))
        assert(pcb_child1->p_parent == NULL);   
    }

    ensure("fails on removing child if p_child is empty"){
        assert(remove_child(pcb1) == NULL)
    }

    it ("correctly remove myself from p_child of p_parent"){
        insert_child(pcb1, pcb_child1);
        insert_child(pcb1, pcb_child2);
        //assert(list_size(&(pcb1->p_child)) == 2);
        //assert(list_contains(&(pcb_child1->p_list), &(pcb1->p_child)))
        //assert(list_contains(&(pcb_child2->p_list), &(pcb1->p_child)))
        
        //assert(list_contains(&(pcb_child1->p_list), &(pcb_child2->p_sib)))
        //assert(list_contains(&(pcb_child2->p_list), &(pcb_child1->p_sib)))
        
        //assert(out_child(pcb_child1) == pcb_child1);
        //assert(pcb_child1->p_parent == NULL);
        //assert(!list_contains(&(pcb_child1->p_list), &(pcb1->p_child)))
        // check that pcb_child1 is not a sibling of pcb_child2
        //assert(!list_contains(&(pcb_child1->p_list), &(pcb_child2->p_sib)))
        // and viceversa
        //assert(!list_contains(&(pcb_child2->p_list), &(pcb_child1->p_sib)))
    }
}
