#ifndef PANDOS_P2TEST_H
#define PANDOS_P2TEST_H

/* just to be clear */
#define NOLEAVES 4 /* number of leaves of p8 process tree */
#define MAXSEM 20

extern int sem_term_mut,     /* for mutual exclusion on terminal */
    s[MAXSEM + 1],           /* semaphore array */
    sem_testsem,             /* for a simple test */
    sem_startp2,             /* used to start p2 */
    sem_endp2,               /* used to signal p2's demise */
    sem_endp3,               /* used to signal p3's demise */
    sem_blkp4,               /* used to block second incaration of p4 */
    sem_synp4,               /* used to allow p4 incarnations to synhronize */
    sem_endp4,               /* to signal demise of p4 */
    sem_endp5,               /* to signal demise of p5 */
    sem_endp8,               /* to signal demise of p8 */
    sem_endcreate[NOLEAVES], /* for a p8 leaf to signal its creation */
    sem_blkp8,               /* to block p8 */
    sem_blkp9;               /* to block p9 */

void uTLB_RefillHandler();
void test();

#endif /* PANDOS_P2TEST_H */
