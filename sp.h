#ifndef __SP_H__
#define __SP_H__

#define MAX_R		30
#define MAX_C		30
#define INFINITY	(1 << 14)

typedef int MAP[MAX_R][MAX_C];

typedef struct _ELIST {
     int      row, col;
     struct _ELIST * next;
} ELIST;

typedef struct _PLIST {
     int     wt;
     ELIST * elements;
     struct _PLIST * next;
} PLIST;

typedef struct {
     int    count;
     PLIST * priorities;
} PQUEUE;
     
int  map_read(char * fname);
void map_draw_path(void);
void build_path(void);
void limit_path(void);

PQUEUE * pqueue_new(void);
int pqueue_insert(PQUEUE * pq, int row, int col, int wt);
int pqueue_peekmin(PQUEUE * pq, int * row, int * col);
int pqueue_popmin(PQUEUE * pq, int * row, int * col);

#endif
