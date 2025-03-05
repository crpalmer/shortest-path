/* pq.c
 * This is a very simple priority queue which was built for use with the
 * shortest path demo program sp.  It should not be used for any other
 * application.  If you need information about building a good priority
 * queue, consult an algorithms book.
 *
 * The long and short of this queue is that it keeps a list of stacks
 * sorted by priority.  When you insert an element, a new stack will be
 * created (if needed) and inserted into the list and then the item will
 * be pushed onto the stack.
 */

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include "sp.h"

size_t max_memory = 0;
size_t current_memory = 0;

void *
my_malloc(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size)) == NULL) return(NULL);
    current_memory += size;
    if (current_memory > max_memory) max_memory = current_memory;
    return(ptr);
}

void
my_free(void * ptr, size_t size)
{
    free(ptr);
    current_memory -= size;
}

PQUEUE *
pqueue_new(void)
{
    PQUEUE *pq;

    if ((pq = (PQUEUE *) my_malloc(sizeof(PQUEUE))) == NULL) return (NULL);
    pq->count = 0;
    pq->priorities = NULL;
    return(pq);
}

int
pqueue_insert(PQUEUE * pq, int row, int col, int wt)
{
    ELIST *e;
    PLIST *p, *pp;

    if (pq == NULL) return(-1); 

    if ((e = (ELIST *) my_malloc(sizeof(ELIST))) == NULL) return(-1);
    e->row = row;
    e->col = col;
    e->next = NULL;

    /* Test empty queue or before the first element of the queue */
    if (pq->priorities == NULL || pq->priorities->wt > wt) {
	if ((p = (PLIST *) my_malloc(sizeof(PLIST))) == NULL) return(-1);
        p->wt = wt;
        p->elements = e;
        p->next = pq->priorities;
        pq->priorities = p;
	pq->count++;
        return(0);
    }

    pq->count++;

    for (p = pq->priorities; p->next && p->next->wt <= wt; p = p->next)
	;

    if (p->wt == wt) {
	e->next = p->elements;
	p->elements = e;
	return(0);
    }
    if ((pp = (PLIST *) my_malloc(sizeof(PLIST))) == NULL) return(-1);
    pp->wt = wt;
    pp->elements = e;
    pp->next = p->next;
    p->next = pp;
    pq->count++;
    return(0);
}

int
pqueue_peekmin(PQUEUE * pq, int * row, int * col)
{
    if (pq == NULL || pq->priorities == NULL || pq->priorities->elements == NULL)
	return(-1);
    *row = pq->priorities->elements->row;
    *col = pq->priorities->elements->col;
    return(0);
}

int
pqueue_popmin(PQUEUE * pq, int * row, int * col)
{
    ELIST *tmpe;
    PLIST *tmpp;

    if (pqueue_peekmin(pq, row, col) < 0) return(-1);
    pq->count--;

    tmpe = pq->priorities->elements;
    pq->priorities->elements = pq->priorities->elements->next;
    my_free(tmpe, sizeof(ELIST));

    if (pq->priorities->elements == NULL) {
	tmpp = pq->priorities;
	pq->priorities = pq->priorities->next;
	my_free(tmpp, sizeof(PLIST));
    }
	
    return (0);
}
