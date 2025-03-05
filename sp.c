/*************************************************************************
 * sp.c : Shortest Paths
 *
 * This is a small demo program which illustrates a modified Djisktra
 * algorithm for finding the shortest path between two points.	In fact,
 * the algorithm finds all shortest paths from some starting point to
 * all other points in the map and then we display the path with the
 * user requested end-point.
 *
 * The purpose of this program is not to be simply copied into your
 * game but is intended to illustrate and hopefully teach a little about
 * this algorithm.  As such, there is a very real possibility of having
 * integer overflows and other small bugs.
 *
 * For more information about the algorithm, see any graph theory book,
 * most data structures/algorithms books or
 * http://www.undergrad.math.uwaterloo.ca/~crpalmer/sp.html
 *
 *------------------------------------------------------------------------
 *
 * The code in this file is broken into two sections:
 *    wrapper code : code which deals with the input and output of the
 *		     algorithm
 *    build_paths  : The actual algorithm
 *
 * As such, there is only really 10 lines of interesting code in this
 * file.  To compile it, you will need a priority queue (eg: pq.c) and
 * the header file sp.h.
 *
 * The priority queue provided is not particularily general, it stores
 * elements by using the wt parameter (length of the shortest path) as
 * a priority and accepts/returns a row and column number for the data
 * elements.  The interface to the pqueue should be pretty much self
 * explanatory and I apologize for not bothering to provide more
 * more than this explanation of it.
 *
 * If you are unfamiliar as to what a priority queue is, I would suggest
 * consulting any algorithms book.
 *
 * -----------------------------------------------------------------------
 *
 * Map files are very simple.  They are ascii files where a single digit
 * represents the cost of stepping onto that square and any other character
 * indicates that the square is impassible.  So, a sample map file is:
 *
 * ########
 * #111111#
 * #222222#
 * #123456#
 * ########
 *
 * Note: rows and columns are numbered starting at (0,0).
 *
 * The command line usage is:
 * sp [-d<level>] <map_file>
 * where <level> is optional and sets the debug level:
 *       All debug levels display the squares being changed at each stage
 *       A debug level of 5 will display the squares not being changed
 *       A debug level of 10 will display the dist array after each square
 *       is considered.
 *       If the debug level is less than 10 (and greater than 0) then
 *       the dist array will be shown once it has been fully calculated.
 *
 * -------------------------------------------------------------------------
 *
 * After having participated in several arguments about shortest paths
 * in the newgroup rec.games.programmer, I've decided to add a simple
 * heuristic function to this algorithm.
 *
 * If you run the program with the options -a and -h you'll be essentially
 * running an A* algorithm which should find a shortest path very quickly.
 *
 * The heuristic is a grid distance where moving on a diagonal has the same
 * cost as moving in any other direction.  The function is the greater
 * of the x distance and the y distance.
 *
 * To change the last version of the code to support a heuristic, I had
 * include the function itself and then change 2 lines of code.
 * At the same time, I added the ability to stop processing when the
 * destination had been reached which required one if statement.  Thus,
 * to add the heuristic required about 4 lines of code.
 *
 * ------------------------------------------------------------------------
 *
 * Comments:  crpalmer@undergrad.math.uwaterloo.ca
 * Date    :  November 15, 1995.
 *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "sp.h"

/* The following two flags can be used to speed up the algorithm.  The
 * use_heuristic flag will estimate the distance from a node to the
 *   destination when it is added to the priority queue.
 * abort_at_dest will stop calculating shortest paths when it has found
 *   one to the destination node.
 *
 * By setting both of these two a non-zero value, you will see a very
 * fast way of generating a single path between two points on the map.
 * The search will be directed toward the destination by using the heuristic
 * and we'll stop once we've found a node.
 * Simply setting the use_heuristic and no other value won't do you much good.
 */
static int use_heuristic = 0;		/* don't use a heuristic */
static int abort_at_dest = 0;		/* don't abort at destination */

/* Normally you would allocate buffers for dist[][] and parent[][] and
 * limit the total length of the search to some reasonable distance.
 * This is just an example using small maps.  That means that i'm
 * making them global so that I don't have to turn the set of paths
 * into a object and can have several procedures deal with it.
 *
 * For the example, map is a 2d array of weights where the weight is
 * defined to the cost of stepping ONTO this square.  Normally, this would
 * be either information stored in a more complicated structure or more
 * likely you would want to implement a function which looks at the
 * square in question and tells you what the weight is.  For example:
 * floor = 1
 * door  = 2
 * acid  = 99
 * death field = INFINITY
 *
 * When we are building the paths, dist[row][col] represents the length
 * of the shortest path found so far from (startr,startc) to (row,col)
 * and the path is found by following the links recorded in parent.
 */

MAP map;
int dist[MAX_R][MAX_C];
int parent[MAX_R][MAX_C][2];
int startr, startc;
int endr, endc;
int rows_read, cols_read;
int pops, pushes;
int debug_level = 0;
extern int max_memory;

/* ok = map_read(file_name)
 *
 * Read the map file file_name and store the information into the global
 * map structure.  After reading the map file, initialize the dist array
 * to INFINITY and the parent array to -1.
 *
 * The number of rows read will be recorded in rows_read.
 * The maximum number of columns read will be recorded in cols_read
 *
 * This routine returns -1 on an error and 0 on success.  In case of an
 * error, the state of the global variables is undefined.
 */

int
map_read(char * fname)
{
     FILE *file;
     int row, col;
     char line[MAX_C*2+1], * line_ptr;

    if ((file = fopen(fname, "r")) == NULL) return(-1);

    cols_read = 0;
    for (row = 0; row < MAX_R && ! feof(file); row++) {
	if (fgets(line, MAX_C*2, file) == NULL) break;
	printf("%s",line);
	for (col = 0, line_ptr = line; col < MAX_C && *line_ptr; col++, line_ptr++)
	     if (isdigit(*line_ptr))
		map[row][col] = *line_ptr - '0';
	     else
		map[row][col] = INFINITY;
	if (col > cols_read) cols_read = col - 1;
	for (; col < MAX_C; col++) map[row][col] = INFINITY;
    }
    rows_read = row;
    for ( ; row < MAX_R; row++)
	for (col = 0; col < MAX_C; col++)
	    map[row][col] = INFINITY;
    fclose(file);

    for (row = 0; row < MAX_R; row++)
	for (col = 0; col < MAX_C; col++) {
	    dist[row][col] = INFINITY;
            parent[row][col][0] = -1;
            parent[row][col][1] = -1;
        }
    startr = startc = endr = endc = -1;
    return(0);
}

/* (void) map_draw_path()
 *
 * This routine should be called after limit_path() has been called or
 * else the user will be shown every path from the starting point which
 * doesn't show very much.
 *
 * map_draw_path will display the starting point as '^', the ending point
 * as '$', the intermediate squares in the path '+', the walkable squares
 * in the map '.' and the impassible squares '#'.
 *
 * A square is considered to be in the path if it has a parent.
 */

void
map_draw_path(void)
{
    int row, col;
    int wt;

    wt = 0;
    for (row = 0; row < rows_read; row++) {
	for (col = 0; col < cols_read; col++) {
	    if (startr == row && startc == col)
		putc('^', stdout);
	    else if (endr == row && endc == col)
		putc('$', stdout);
	    else if (parent[row][col][0] >= 0 && parent[row][col][1] >= 0) {
		putc('+', stdout);
		wt += map[row][col];
	    } else if (map[row][col] < INFINITY)
		putc('.', stdout);
	    else
		putc('#', stdout);
	}
	printf("\n");
    }
    printf ("\nThe weight of this path is: %d\n", wt);
}
		

/* Two helpful macros to make the code more readable.  drow(row,i) and
 * dcol(col,i) will apply the delta table to the indicated row/column
 * to return a square around the one specified.
 * The delta table lists row/column offsets of the 8 squares which
 * surround a square (ie: up-left, up, up-right, left, etc).
 */
#define drow(row,i) (row+delta[i][0])
#define dcol(col,i) (col+delta[i][1])

/* delta is defined with the diagonals first and the "natural" directions
 * second which corresponds with our priority queue to give us "nicer"
 * looking paths.  It would probably be more appropriate to make the
 * algorithm simply pick a natural direction when possible.  I would
 * recommend that if actually using this algorithm with a real priority
 * queue since you will not be able to predict order in which the elements
 * will be returned.
 */
int delta[8][2] = { -1, -1, -1, 1, 1, -1, 1, 1, -1, 0, 0, 1, 1, 0, 0, -1 };

/* (void) build_illustrate();
 *
 * Show the weights of all the shortest paths.
 */

void
build_illustrate(void)
{
    int i, j;
    char line[256];

    for (i = 0; i < rows_read; i++) {
	for (j = 0; j < cols_read; j++)
	    if (dist[i][j] < INFINITY)
		printf("%2d ", dist[i][j]);
	    else
		printf(" I ");
	printf("\n");
    }
    printf("\nHIT ENTER:"); fflush(stdout);
    fgets(line, sizeof(line), stdin);
}

#ifdef max
#undef max
#endif
#ifdef abs
#undef abs
#endif

#define max(a,b) ((a) > (b) ? (a) : (b))
#define abs(a)   ((a) < 0 ? -(a) : (a))

static int
h(int r1, int c1, int r2, int c2)
{
    if (use_heuristic) return(max(abs(r2 - r1), abs(c2 - c1)));
    return(0);
}

/**************************************************************************
 *
 * (void) build_path()
 *
 * This routine is entire algorithm:
 *
 * push the starting point onto the queue
 * while there is an element of least weight in the queue, say (r,c)
 *    foreach square around (r,c), say (r',c') where
 *	  dist[to (r,c)] + weight(r',c') < dist[to (r',c')]
 *    do
 *	  dist[to (r',c')] = dist[to (r,c)] + weight(r',c')
 *	  push (r',c') onto the queue
 **************************************************************************/


void
build_path(void)
{
    PQUEUE *pq;
    int row, col;
    int	i;

    pq = pqueue_new();
    if (startr < 0 || startr >= rows_read || startc < 0 || startc >= cols_read)
	return;
    dist[startr][startc] = 0;
    parent[startr][startc][0] = -1;
    parent[startr][startc][1] = -1;
    pqueue_insert(pq, startr, startc, 0 + h(startr,startc,endr,endc));
    pushes++;

    while (pqueue_popmin(pq, &row, &col) == 0) {
	/* DEBUG INFORMATION */
	pops++;
	if (debug_level > 0)
    	    printf("\nSelected (row,col)=(%d,%d).  Adding:\n", row, col);

	/* See if we only wanted the one path */
	if (abort_at_dest && row == endr && col == endc) {
		/* STOP HERE */
		break;
	}
	/* FOREACH square adjacent to (row,col), call it drow(row,i), dcol(col,i) */
	for (i = 0; i < 8; i++) {
	    /* Check to make sure the square is on the map */
	    if (drow(row,i) < 0 || drow(row,i) >= rows_read ||
		dcol(col,i) < 0 || dcol(col,i) >= cols_read)
		continue;

	    /* Check to see if we've found a shorter path */
	    if (dist[row][col] + map[drow(row,i)][dcol(col,i)] <
		dist[drow(row,i)][dcol(col,i)]) {
		/* DEBUG INFORMATION */
		if (debug_level > 0) {
		    printf("\t\t(%d,%d) Weight was/is: ", drow(row,i), dcol(col,i));
                    printf("%d/%d\n", dist[drow(row,i)][dcol(col,i)], map[drow(row,i)][dcol(col,i)] + dist[row][col]);
		}

		/* We have a shorter path, update the information */
		dist[drow(row,i)][dcol(col,i)] = map[drow(row,i)][dcol(col,i)] + dist[row][col];
		parent[drow(row,i)][dcol(col,i)][0] = row;
		parent[drow(row,i)][dcol(col,i)][1] = col;
		/* Push the modified square onto the priority queue */
		pqueue_insert(pq, drow(row,i), dcol(col,i), dist[drow(row,i)][dcol(col,i)] + h(drow(row, i), dcol(col, i), endr, endc));
		pushes++;
	    } else if (debug_level >= 5) {
		printf("\t\t(%d,%d) Weight Stays: %d (would have been %d)\n", drow(row,i), dcol(col,i), dist[drow(row,i)][dcol(col,i)], dist[row][col] + map[drow(row,i)][dcol(col,i)]);
	    }
	}
	if (debug_level >= 10) build_illustrate();
    }
    return;
}

/* (void) limit_path(void)
 * This routine will set the parent of every square which is not on the
 * shortest path we selected between the starting and ending points.
 * It does this by tracing back the path and multiplying the distance of
 * the shortest path to that point by -1 to identify the squares which
 * lie on the path.
 * Then, the entire set of squares is run through to return the distance
 * values to their original values and clear the parent information for
 * all squares in the path.
 * This is not a pretty way of getting the path but it's the easiest way
 * to do it in place in such a way that we can easily dump the map to
 * the screen.
 */

void
limit_path(void)
{
    int row, col;
    int tmp;

    if (endr < 0 || endr >= rows_read || endc < 0 || endc >= cols_read) return;
    row = endr;
    col = endc;
    do {
	dist[row][col] = -dist[row][col];
	tmp = parent[row][col][0];
	col = parent[row][col][1];
	row = tmp;
    } while(row >= 0 && col >= 0);

    for (row = 0; row < rows_read; row++)
	for (col = 0; col < cols_read; col++)
	    if (dist[row][col] < 0)
		dist[row][col] = -dist[row][col];
	    else
		parent[row][col][0] = -1;
    return;
}
    
int
main(int argc, char ** argv)
{
    char tmp_str[128];
    int  i;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') break;
	switch(argv[i][1]) {
	case 'a':
		abort_at_dest++;
		break;

	case 'd':
		debug_level = atoi(&argv[1][2]);
		if (debug_level <= 0) debug_level = 1;
		break;
	case 'h':
		use_heuristic++;
		break;
	default:
usage:
		printf("USAGE: sp [-a] [-d<level>] [-h] [map filename]\n");
		printf("       -a       : Stop when we have a path to the destination.\n");
		printf("       -d<level>: Set the debug level.\n");
		printf("       -h       : Apply a heuristic to direct the search.\n");
		return(2);
	}
    }

    if (i >= argc) goto usage;

    map_read(argv[i]);
    printf("Read a %d x %d map.\n", rows_read, cols_read);
    printf("Enter 4 integers: starting row, starting column, ending row, ending column\n");
    if (scanf("%d %d %d %d", &startr, &startc, &endr, &endc) != 4) return(0);
    fgets(tmp_str, sizeof(tmp_str), stdin);
    pops = 0;
    pushes = 0;
    build_path();
    if (debug_level < 10 && debug_level > 0) build_illustrate();
    limit_path();
    map_draw_path();
    printf ("We required 2x %d/%d (pushes/pops) queue operations\n", pushes, pops);
    printf ("N = rows x cols = %d x %d = %d\n", rows_read, cols_read, rows_read * cols_read);
    printf ("Maximum memory consummed in the priority queue = %d\n", max_memory);
    return(0);
}
