#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
    struct spinlock lock;
    struct shm_page {
        uint id;
        char *frame;
        int refcnt;
    } shm_pages[64];
} shm_table;

void shminit() {
    int i;
    initlock(&(shm_table.lock), "SHM lock");
    acquire(&(shm_table.lock));
    for (i = 0; i< 64; i++) {
        shm_table.shm_pages[i].id =0;
        shm_table.shm_pages[i].frame =0;
        shm_table.shm_pages[i].refcnt =0;
    }
    release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this
    struct proc* curproc = myproc();
    uint sz = PGROUNDUP(curproc->sz);
    int found_page = 0;
    int opened = 0;
    int i;
    acquire(&(shm_table.lock));
    for (i = 0; i < 64; i++) {
        if (shm_table.shm_pages[i].id == id) {
            found_page = 1;
            mappages(curproc->pgdir, (char*)sz, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W | PTE_U);
            shm_table.shm_pages[i].refcnt++;
            curproc->sz = sz + PGSIZE;
            *pointer = (char *)sz;
            opened = 1;
            break;
        }
    }
    if (found_page == 0) {
        int j;
        for (j = 0; j < 64; j++) {
            if (shm_table.shm_pages[j].id == 0) {
                shm_table.shm_pages[j].id = id;
                shm_table.shm_pages[j].frame = kalloc();
                memset(shm_table.shm_pages[j].frame, 0, PGSIZE);
                mappages(curproc->pgdir, (char*)sz, PGSIZE, V2P(shm_table.shm_pages[j].frame), PTE_W | PTE_U);
                shm_table.shm_pages[j].refcnt++;
                curproc->sz = sz + PGSIZE;
                *pointer = (char *)sz;
                opened = 1;
                break;
            }
        }
    }
    release(&(shm_table.lock));
    if (opened == 1) {
        return 0;
    } else {
        return 1;
    }
//    return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
    int closed = 0;
    struct proc* curproc = myproc();
    int i;
    acquire(&(shm_table.lock));
    for (i = 0; i < 64; i++) {
        if (shm_table.shm_pages[i].id == id) {
            if (shm_table.shm_pages[i].refcnt > 0) {
                shm_table.shm_pages[i].refcnt--;
            }
            closed = 1;
            if (shm_table.shm_pages[i].refcnt == 0) {
                cprintf("Cleared the shm entry");
                shm_table.shm_pages[i].id = 0;
//                kfree(shm_table.shm_pages[i].frame);
                shm_table.shm_pages[i].frame = 0;
                mappages(curproc->pgdir, )
                break;
            }
        }
    }
    release(&(shm_table.lock));
    cprintf("Closed value: %d", closed);
    if (closed == 1) {
        return 0;
    } else {
        return 1;
    }
//return 0; //added to remove compiler warning -- you should decide what to return
}
