/*********************************P1TEST.C*******************************
 *
 *	Test program for the modules ASL and pcbQueues (phase 1).
 *
 *	Produces progress messages on terminal 0 in addition
 *		to the array ``okbuf[]''
 *		Error messages will also appear on terminal 0 in
 *		addition to the array ``errbuf[]''.
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 */

#include "os/asl.h"
#include "os/const.h"
#include "os/pcb.h"
#include "os/types.h"
#include <umps/libumps.h>

#define MAXPROC 20
#define MAXSEM MAXPROC

char okbuf[2048]; /* sequence of progress messages */
char errbuf[128]; /* contains reason for failing */
char msgbuf[128]; /* nonrecoverable error message before shut down */
int sem[MAXSEM];
int onesem;
pcb_t *procp[MAXPROC], *p, *q, *firstproc, *lastproc, *midproc;
char *mp = okbuf;

#define TRANSMITTED 5
#define ACK 1
#define PRINTCHR 2
#define CHAROFFSET 8
#define STATUSMASK 0xFF
#define TERM0ADDR 0x10000254

/* Ported from phase2 to compile */
inline void null_state(state_t *s)
{
    s->entry_hi = 0;
    s->cause = 0;
    s->status = UNINSTALLED;
    s->pc_epc = 0;
    s->hi = 0;
    s->lo = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        s->gpr[i] = 0;
}

typedef unsigned int devreg;

/* This function returns the terminal transmitter status value given its address
 */
devreg termstat(memaddr *stataddr) { return ((*stataddr) & STATUSMASK); }

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint(char *str, unsigned int term)
{
    memaddr *statusp;
    memaddr *commandp;
    devreg stat;
    devreg cmd;
    unsigned int error = FALSE;

    if (term < DEVPERINT) {
        /* terminal is correct */
        /* compute device register field addresses */
        statusp = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) +
                             (TRANSTATUS * DEVREGLEN));
        commandp = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) +
                              (TRANCOMMAND * DEVREGLEN));

        /* test device status */
        stat = termstat(statusp);
        if (stat == READY || stat == TRANSMITTED) {
            /* device is available */

            /* print cycle */
            while (*str != EOS && !error) {
                cmd = (*str << CHAROFFSET) | PRINTCHR;
                *commandp = cmd;

                /* busy waiting */
                stat = termstat(statusp);
                while (stat == BUSY)
                    stat = termstat(statusp);

                /* end of wait */
                if (stat != TRANSMITTED)
                    error = TRUE;
                else
                    /* move to next char */
                    str++;
            }
        } else
            /* device is not available */
            error = TRUE;
    } else
        /* wrong terminal device number */
        error = TRUE;

    return (!error);
}

/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void addokbuf(char *strp)
{
    char *tstrp = strp;
    while ((*mp++ = *strp++) != '\0')
        ;
    mp--;
    termprint(tstrp, 0);
}

/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void adderrbuf(char *strp)
{
    char *ep = errbuf;
    char *tstrp = strp;

    while ((*ep++ = *strp++) != '\0')
        ;

    termprint(tstrp, 0);

    PANIC();
}

int main(void)
{
    int i;

    init_pcbs();
    addokbuf("Initialized process control blocks   \n");

    /* Check allocProc */
    for (i = 0; i < MAXPROC; i++) {
        if ((procp[i] = alloc_pcb()) == NULL)
            adderrbuf("alloc_pcb: unexpected NULL   ");
    }
    if (alloc_pcb() != NULL) {
        adderrbuf("alloc_pcb: allocated more than MAXPROC entries   ");
    }
    addokbuf("alloc_pcb ok   \n");

    /* return the last 10 entries back to free list */
    for (i = 10; i < MAXPROC; i++)
        free_pcb(procp[i]);
    addokbuf("freed 10 entries   \n");

    /* create a 10-element process queue */
    LIST_HEAD(qa);
    if (!empty_proc_q(&qa))
        adderrbuf("empty_proc_q: unexpected FALSE   ");
    addokbuf("Inserting...   \n");
    for (i = 0; i < 10; i++) {
        if ((q = alloc_pcb()) == NULL)
            adderrbuf("alloc_pcb: unexpected NULL while insert   ");
        switch (i) {
            case 0:
                firstproc = q;
                break;
            case 5:
                midproc = q;
                break;
            case 9:
                lastproc = q;
                break;
            default:
                break;
        }
        insert_proc_q(&qa, q);
    }
    addokbuf("inserted 10 elements   \n");

    if (empty_proc_q(&qa))
        adderrbuf("empty_proc_q: unexpected TRUE");

    /* Check outProc and headProc */
    if (head_proc_q(&qa) != firstproc)
        adderrbuf("head_proc_q failed   ");
    q = out_proc_q(&qa, firstproc);
    if (q == NULL || q != firstproc)
        adderrbuf("out_proc_q failed on first entry   ");
    free_pcb(q);
    q = out_proc_q(&qa, midproc);
    if (q == NULL || q != midproc)
        adderrbuf("out_proc_q failed on middle entry   ");
    free_pcb(q);
    if (out_proc_q(&qa, procp[0]) != NULL)
        adderrbuf("out_proc_q failed on nonexistent entry   ");
    addokbuf("out_proc_q ok   \n");

    /* Check if removeProc and insertProc remove in the correct order */
    addokbuf("Removing...   \n");
    for (i = 0; i < 8; i++) {
        if ((q = remove_proc_q(&qa)) == NULL)
            adderrbuf("remove_proc_q: unexpected NULL   ");
        free_pcb(q);
    }
    if (q != lastproc)
        adderrbuf("remove_proc_q: failed on last entry   ");
    if (remove_proc_q(&qa) != NULL)
        adderrbuf("remove_proc_q: removes too many entries   ");

    if (!empty_proc_q(&qa))
        adderrbuf("empty_proc_q: unexpected FALSE   ");

    addokbuf("insert_proc_q, remove_proc_q and empty_proc_q ok   \n");
    addokbuf("process queues module ok      \n");

    addokbuf("checking process trees...\n");

    if (!empty_child(procp[2]))
        adderrbuf("empty_child: unexpected FALSE   ");

    /* make procp[1] through procp[9] children of procp[0] */
    addokbuf("Inserting...   \n");
    for (i = 1; i < 10; i++) {
        insert_child(procp[0], procp[i]);
    }
    addokbuf("Inserted 9 children   \n");

    if (empty_child(procp[0]))
        adderrbuf("empty_child: unexpected TRUE   ");

    /* Check out_child */
    q = out_child(procp[1]);
    if (q == NULL || q != procp[1])
        adderrbuf("out_child failed on first child   ");
    q = out_child(procp[4]);
    if (q == NULL || q != procp[4])
        adderrbuf("out_child failed on middle child   ");
    if (out_child(procp[0]) != NULL)
        adderrbuf("out_child failed on nonexistent child   ");
    addokbuf("out_child ok   \n");

    /* Check remove_child */
    addokbuf("Removing...   \n");
    for (i = 0; i < 7; i++) {
        if ((q = remove_child(procp[0])) == NULL)
            adderrbuf("remove_child: unexpected NULL   ");
    }
    if (remove_child(procp[0]) != NULL)
        adderrbuf("remove_child: removes too many children   ");

    if (!empty_child(procp[0]))
        adderrbuf("empty_child: unexpected FALSE   ");

    addokbuf("insert_child, remove_child and empty_child ok   \n");
    addokbuf("process tree module ok      \n");

    for (i = 0; i < 10; i++)
        free_pcb(procp[i]);

    /* check ASL */
    init_asl();
    addokbuf("Initialized active semaphore list   \n");

    /* check remove_blocked and insert_blocked */
    addokbuf("insert_blocked test #1 started  \n");
    for (i = 10; i < MAXPROC; i++) {
        procp[i] = alloc_pcb();
        if (insert_blocked(&sem[i], procp[i]))
            adderrbuf("insert_blocked(1): unexpected TRUE   ");
    }
    addokbuf("insert_blocked test #2 started  \n");
    for (i = 0; i < 10; i++) {
        procp[i] = alloc_pcb();
        if (insert_blocked(&sem[i], procp[i]))
            adderrbuf("insert_blocked(2): unexpected TRUE   ");
    }

    /* check if semaphore descriptors are returned to free list */
    p = remove_blocked(&sem[11]);
    if (insert_blocked(&sem[11], p))
        adderrbuf("remove_blocked: fails to return to free list   ");

    if (insert_blocked(&onesem, procp[9]) == FALSE)
        adderrbuf("insert_blocked: inserted more than MAXPROC   ");

    addokbuf("remove_blocked test started   \n");
    for (i = 10; i < MAXPROC; i++) {
        q = remove_blocked(&sem[i]);
        if (q == NULL)
            adderrbuf("remove_blocked: wouldn't remove   ");
        if (q != procp[i])
            adderrbuf("remove_blocked: removed wrong element   ");
        if (insert_blocked(&sem[i - 10], q))
            adderrbuf("insert_blocked(3): unexpected TRUE   ");
    }
    if (remove_blocked(&sem[11]) != NULL)
        adderrbuf("remove_blocked: removed nonexistent blocked proc   ");
    addokbuf("insert_blocked and remove_blocked ok   \n");

    if (head_blocked(&sem[11]) != NULL)
        adderrbuf("head_blocked: nonNULL for a nonexistent queue   ");
    if ((q = head_blocked(&sem[9])) == NULL)
        adderrbuf("head_blocked(1): NULL for an existent queue   ");
    if (q != procp[9])
        adderrbuf("head_blocked(1): wrong process returned   ");
    p = out_blocked(q);
    if (p != q)
        adderrbuf("outBlocked(1): couldn't remove from valid queue   ");
    q = head_blocked(&sem[9]);
    if (q == NULL)
        adderrbuf("head_blocked(2): NULL for an existent queue   ");
    if (q != procp[19])
        adderrbuf("head_blocked(2): wrong process returned   ");
    p = out_blocked(q);
    if (p != q)
        adderrbuf("outBlocked(2): couldn't remove from valid queue   ");
    p = out_blocked(q);
    if (p != NULL)
        adderrbuf("outBlocked: removed same process twice.");
    if (head_blocked(&sem[9]) != NULL)
        adderrbuf("out/head_blocked: unexpected nonempty queue   ");
    addokbuf("head_blocked and outBlocked ok   \n");
    addokbuf("ASL module ok   \n");
    addokbuf("So Long and Thanks for All the Fish\n");
    return 0;
}
