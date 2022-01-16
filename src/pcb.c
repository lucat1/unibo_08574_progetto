#include "pcb.h"

// TODO: Remove the following include statements 
// (I need those for vscode)
#include "pandos_const.h"
#include "pandos_types.h"


#define FALSE 0
#define TRUE 1

// TODO: Change these define position 
// (Maybe also change the names of pcbFree_table and pcbFree_h)
#define MAX_PROC 20
static pcb_t pcbFree_table[MAX_PROC];
static struct list_head *pcbFree_h;


// This function should be called only once during the initialization phase
void initPcbs()
{
    // Initialize the list
    INIT_LIST_HEAD(pcbFree_h);

    // Add pcbFree_table elements to the list
    for(int i = 0; i < MAX_PROC; i++){
        // TODO: check what happens when the element of the array is undefined
        list_add(&(pcbFree_table[i].p_list), pcbFree_h);
    }
}

void freePcb(pcb_t *p)
{
    // TODO: Check if the element p is already contained in the list
    // (I don't know if it supposed to be already checked or not, so I'll just leave it like this)
    list_add(p, pcbFree_h);
}

/** 
 * null_state - returns a state whose value is null
 * 
 * Returns the null state
 * 
 * TODO:    Test this function
 */
state_t null_state()
{
    // TODO: Search on the documentation if 
    // there are constants rappresenting these values
    state_t null_value;
    null_value.entry_hi = 0;
    null_value.cause = 0;
    null_value.status = UNINSTALLED;
    null_value.pc_epc = 0;
    null_value.hi = 0;
    null_value.lo = 0;
    for(int i = 0; i < STATE_GPR_LEN; i++){
        null_value.gpr[i] = 0;
    }
    return null_value;
}

pcb_t* allocPcb()
{
    if(list_empty(pcbFree_h)){
        return NULL;
    }else{
        pcb_t *first = pcbFree_h->next;
        list_del(pcbFree_h->next);
        INIT_LIST_HEAD(&(first->p_list));
        INIT_LIST_HEAD(&(first->p_child));
        INIT_LIST_HEAD(&(first->p_sib));
        first->p_parent = NULL;
        first->p_s = null_state();
        first->p_time = 0;
        first->p_semAdd = NULL;
        return first;
    }
}

void mkEmptyProcQ(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *head)
{
    return list_empty(head);
}

void insertProcQ(struct list_head *head, pcb_t *p)
{
    list_add_tail(&(p->p_list), head);
}

pcb_t* headProcQ(struct list_head *head)
{
    if(list_empty(head)){
        return NULL;
    }else{
        return container_of(head->next, pcb_t, p_list);
    }
}


pcb_t* removeProcQ(struct list_head* head){
    
    // check if list is empty
    if(list_empty(head)) return NULL;

    // get the first element of the list
    struct list_head *to_remove = list_next(head);

    // delete element from list
    list_del(to_remove);

    // return the pcb pointed by the deleted element
    return container_of(to_remove, pcb_t , p_list); 
}


pcb_t* outProcQ(struct list_head* head, pcb_t *p){
    struct list_head* iter = (head)->next;
    
    // looking for p element 
    for (; container_of(iter, pcb_t , p_list) != (p) && iter != (head); iter = iter->next);
    
    // completed a circle without finding p element
    if (iter == head) {
        return NULL;
    }
    
    // remove p element from list
    list_del(iter);
    
    return container_of(iter, pcb_t , p_list);
}


int emptyChild(pcb_t *p){
    if (p->p_child.next == NULL) {
        return TRUE;
    }

    return FALSE;
}


void insertChild(pcb_t *prnt, pcb_t *p){
    list_add(p, &(prnt->p_child));
}


pcb_t* removeChild(pcb_t *p){
    if(emptyChild(p)) return NULL;
    pcb_t *ret = list_next(&(p->p_child));
    list_del(ret);
    return ret;
}

pcb_t *outChild(pcb_t* p){
    if (p->p_parent == NULL) return NULL; 

    // get the first element of p_child inside p_parent of p
    struct list_head* iter = ((p->p_parent)->p_child).next;

    // assume that p exists in p_child of p->p_parent
    for (; container_of(iter, pcb_t , p_child) != (p); iter = iter->next);
    
    list_del(iter);
    
    return container_of(iter, pcb_t , p_child);
}

