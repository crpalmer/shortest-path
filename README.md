Dijkstra's Algorithm:
=====================

Notes:

* This algorithm is not presented in the same way that you'll find
  it in most texts because i'm ignored directed vs. undirected graphs
  and i'm ignoring the loop invariant that you'll see in any book
  which is planning on proving the correctness of the algorithm.
* If we are dealing with a di-graph the algorithm will be the same
  with the only difference being the meaning attached to each edge.
* The loop invariant is that at any stage we have partitioned the
  graph into three sets of vertices (S,Q,U), S which are vertices to 
  which we know their shortest paths, Q which are ones we have "queued"
  knowing that we may deal with them now and U which are the other
  vertices.  This information is important in proving the correctness and
  analyzing the running-time of the algorithm and not for understanding
  it (in my humble opinion).

REF: Cormen. Thomas H., Leiserson, Charles E., Rivest, Ronald L.,
     Introduction to Algorithms, MIT Press (1990), pages 527-531

Revision:
The current revision of this page was last modified on August 27, 1998.

Given a graph G with a weight function wt:E(G)->R which maps the edges
to a real valued weight and wt(e) > 0 for all e in E(G).  NOTE: wt(e) > 0
is a VERY important assumption.  As i've presented the algorith, a weight
of 0 is not actually a problem (it could be for some implementations) but
a negative weight could/would result in an infinite loop.  For example,

     *---- 1 ---*
      \        /
      -10     1
         \   /
          \ /
           *

will generate paths which cycle endlessly.

You need an origin vertex (where all the paths are starting from, or,
more typically in games, where the paths are ending).

Augment the labels of the vertices by a real value, initially infinity,
which is the shortest weighted path from the origin to this vertex (which
has been found so far).  Also, augment each vertex with a "pointer" to its
parent in the shortest weighted path found so far, initially have this
pointing nowhere.

You need a priority queue which is sorted based on the weight of the
shortest path from the origin to the vertex.  When an element is inserted
into the priority queue and it already exists, the previous copy must be
removed and the new one inserted into the right level [*].

----
   Take the origin vertex, set the weight of the shortest path to 0 and
   push it onto the priority queue.

   while the priority queue is not empty, pop an entry <v,w_v,p_v> where
     v is the vertex, w_v and p_v are the augmented labels of v.
     foreach edge e=(v,u) in G, where u has augmented labels w_u, p_u.
         if wt(e) + w_v < w_u then
             set p_u to v
             set w_u to wt(e) + w_v
             add <u, w_u, p_u> to the priority queue.
----

Comments:
=========

Technically, [*] must be true because otherwise it would be impossible to
prove the correctness of the algorithm.  I've played with an personally
think that you can get a better performance (your priority queue becomes
almost O(1) with the cost of extra elements building up in the queue)
by relaxing this restriction and allowing multiple copies of the same
vertex to accumulate in the queue.

The net effect will be that some sets of vertices may have to be cycled
through the queue several times (read "more cost").  It is my opinion
that this additional cost is considerably less than the O(log n) [n is
the size of the priority queue] time required to implement this algorithm
as described above.

However, I have not attempted to analyze the worst case effect of relaxing
[*] and would say to implement it in this way only if you trust my guess.

Also, note that if you were going to implement a heuristic for directing
the search, you wouldn't insert the weight of the shortest path from the
source node to this node.  Instead, you would insert the weight of the
shortest path from the source node to this node plus the heuristic's
estimate of the cost to the destination node.  I won't be discussing this
anymore until the very end of the document.

Tile/Grid Implementation:
=========================

Def: What I mean by "a game with a grid/tile map"

     A game where the playing area is dividing into
     squares with one or more "object" on the square.  These objects include
     walls, doors, etc which are assumed to occupy the entire square.

If you want to apply what i'm going to say where walls do not occupy the
entire square, you'll need a function wt({x,y}, {x',y'}) which gives the
cost of moving from (x,y) to (x',y') and otherwise it's the same.

In a game with a grid map, you need a function (or a table or whatever)
which i'll call wt(x,y) which gives you the "cost" of moving onto a
specified grid location (x,y).  Note: "moving *onto*".  If you are
writing a dungeon based game and you have a teleporter at (a,b) that you don't
want the monsters to hit, make wt(a,b) = infinity where infinity is
some arbitrarily large number.  The same applies to walls, if you have a
wall square at (a,b) then wt(a,b) = infinity.

For any square, there are at most 8 squares around it.  This is easy to
implement with an array that gives the offsets for each square.

We'll assume that you don't want the shortest paths for the entire world
(which could be arbitrarily large) but instead want shortest paths for
only a limited area, say of width and height 2*DELTA+1.

We will be storing elements in two arrays in positions [0..2*DELTA] which
correspond to the position on the map (o_x - x + DELTA - 1).  To make
the pseudo-code more readable, let:

---
    f_x(x) = x - o_x + DELTA
    f_y(y) = y - o_y + DELTA
---

be two functions which translate the coordinates.

The following two arrays are used to store the "augmented" information
discussed in the earlier explanation.  These store the following information:

---
    w[x'][y'] The weight of the current shortest path from (o_x, o_y) to
          (o_x + x' - DELTA, o_y + y' - DELTA).
    p[x'][y'] The parent node of that shortest path

    int w[2*DELTA+1][2*DELTA+1];
    int p[2*DELTA+1][2*DELTA+1][2];
---

Algorithm: (origin at (o_x, o_y))
---------------------------------

---
   set each w[i][j] = infinity
   set each p[i][j] = (infinity, infinity)

   set w[f_x(o_x)][f_y(o_y)] = 0
   push (o_x, o_y) onto the priority queue, say pq.                     [**]

   while (pop (x,y) from pq is possible)
                                                                        [***]
       foreach each square (x',y') adjacent to (x,y) and where
           |x-o_x| <= DELTA and |y-o_y| <= DELTA
           if w[f_x(x)][f_y(y)] + wt(x',y') < w[f_x(x')][f_y(y')] then
              { we have found a new shortest path }
              w[f_x(x')][f_y(y')] = w[f_x(x)][f_y(y)] + wt(x',y')
              p[f_x(x')][f_y(y')] = (x,y)
              push (x',y') onto the priority queue                      [**]
---

And you now have the shortest paths from (o_x,o_y) to every square in
the range { o_x - DELTA, o_y - DELTA, o_x + DELTA, o_y + DELTA } and
this will print the path from some point (x,y) to (o_x, o_y):

---
    do
       print (x,y)
       (x,y) = p[f_x(x)][f_(y)]
    while (x,y) != (infinity, infinity)
---

and any square, (x,y), which is not reachable from (o_x, o_y) will have:

---
     p[f_x(x)][f_y(y)] = (infinity, infinity)
     w[f_x(x)][f_y(y)] = infinity
---

Sample Code:
============

I have written a simple example of implementing this algorithm for a grid
map game.  The source code is zipped (pkzip).  The code itself is quite
simple.

I have been able to compile and run the code on several UNIX boxes and DOS
with both the Microsoft C compiler (version 7, I think) and GCC (2.6.3 with
the version of DJGPP current as of June 1, 1995).  It should be quite easy
to port to any system.

Included in the source code is a single map which should give you the gist
of what sort of input it expects and you should read the file sp.c which
explains how to use the demo.

This source code is release with the understanding that I retain ownership
of the code.  You are free to make use of it in any way you see fit and
by transferring the code, you accept all responsibility for any damages
which result from using it.

Having accepted that, you may get the source code:
[this repository]

NOTE:  I've also got a modified version of the same code with a link
a little further down into the document.  The second version can use a simple
heuristic to find the path quickly.

Adding a Heuristic
==================

If you are more interested in finding the shortest path between two nodes
(and not all shortest paths to a node), you may be interested in applying
a heuristic function to the search.

Author's Note: My apologies on this section.  It is not overly
clear and is a little too brief.  If you don't follow what I'm saying,
you may wish to look at the source code I've supplied and then compare
the algorith in this page to the actual source code.  It should be
fairly clear.

A /heuristic function/ is a fancy way of saying a function which guesses
at an answer.  In this case, our heurisitic function will be guessing
at the length of the shortest path from any node to the destination node.

IMPORTANT>: It is important that your function always be at most the
real shortest distance.  If you are using a real valued cartesian playing
field, you would want to use sqrt(dist(x)^2 + dist(y)^2) distance function.
Using a heurtistic function of 0 will get you back to the original algorithm.

The effect of adding a good heuristic function would be to direct the search
so that very few bad paths will be considered.  I'm not going to bother
much with the theory that this is a shortest path or even why it works.
If you're avidly reading this part, you probably just want to get a
short path as quickly as possible and are willing to trust a little magic.

To add the heuristic to my algorithm, let h((x,y), (dest_x, dest_y))
be a function which estimates the shortest path betwen two points.

In the algorith, in each line that has ** beside it, add the heuristic
function applied to the current node.  At the point marked with ***
add the line:

if (x,y) = (dest_x, dest_y) then done.

And voila!  You have a function which can rapidly find the shortest path
between any two points.

The following source code uses exactly the same algorith with a heuristic
function that corresponds to the shortest distance on a grid.

---
  H(x_1, y_1, x_2, y_2) = max( |x_2 - x_1|, |y_2 - y_1| )
---

which is the shortest possible distance on a grid between the points
(x_1, y_1) and (x_2, y_2).  The only changes to the original source code
are to add this value to the priority when entries are added to the
priority queue and a simple if statement which stops processing when we
have found the node for which we are searching.

To compare the different techniques, you can use two command line options.
-a will request that processing stop when the destination has been
located and -h will request that the heuristic be applied.  Thus
running the program with -a and then with -a -h will give you a good
comparison between Djisktra's and Djisktra's with the heuristic.

Disclaimer I have not compiled this code on any machine except
a DEC Alpha (OSF).  The zip file was created using that system's
zip command.  I don't guarantee that anyone else will be able to get and
compile this code.  For this reason, I'm continuing to provide the original
source code (just in case).

A* vs. Dijkstra
===============

If you are familiar with the A* algorithm, you might notice that adding
the heuristic to Dijkstra's algorithm gives you something very similar.
Basically, Dijkstra's algorithm with a heuristic is equivalent to A*
except for a couple of technical facts:

  * A* will generally use less memory
  * You must know the entire search space to use Dijkstra's algorithm

For people that are writing tile based games, I would actually
recommend that they use Dijkstra's algorithm because it is easier to
code and will be about as efficient as A*.

Feel free to send any comments, criticism et al to me.
Chris Palmer
(no longer valid) crpalmer+@cs.cmu.edu

