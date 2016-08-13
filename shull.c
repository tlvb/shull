#include "shull.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

struct flipdata {
	unsigned int maxflips;
	bool flipped;
};

void *debug_print_triangle(void *a, void *b) { /*{{{*/
	sh_triangle *t = (sh_triangle *)a;
	FILE *fd = (FILE *)b;
	if (t == NULL) {
		fprintf( fd, "<T=(NULL)>");
	}
	else {
		fprintf(
			fd, "<T=%p: (%.0" PSHFLT ":%.0" PSHFLT ")->(%.0" PSHFLT ":%.0" PSHFLT ")->(%.0" PSHFLT ":%.0" PSHFLT ")>",
			t,
			t->p[0]->x, t->p[0]->y,
			t->p[1]->x, t->p[1]->y,
			t->p[2]->x, t->p[2]->y
		);
	}
	return a;
} /*}}}*/
void *debug_print_edge(void *a, void *b) { /*{{{*/
	sh_edge *e = (sh_edge *)a;
	FILE *fd = (FILE *)b;
	fprintf(
		fd, "[e=%p: (%.0" PSHFLT ":%.0" PSHFLT ")->(%.0" PSHFLT ":%.0" PSHFLT ")]",
		e,
		e->p[0]->x, e->p[0]->y,
		e->p[1]->x, e->p[1]->y
	);
	for (size_t i=0; i<2; ++i) {
		fprintf(fd, " ");
		debug_print_triangle(e->t[i], b);
	}
	fprintf(fd, "\n");
	return a;
} /*}}}*/

SHFLT sqdist(const sh_point *p, const sh_point *q) { /*{{{*/
	return (p->x - q->x) * (p->x - q->x) + (p->y - q->y) * (p->y - q->y);
} /*}}}*/
int radialcompare(const void *a, const void *b, void *c) { /*{{{*/
	int32_t rr = sqdist((const sh_point *)a, (const sh_point *)c);
	int32_t ss = sqdist((const sh_point *)b, (const sh_point *)c);
	if (rr < ss) {
		return -1;
	}
	else if (rr > ss) {
		return 1;
	}
	else {
		return 0;
	}
} /*}}}*/
void radialsort(sh_point *ps, size_t n, sh_point *q) { /*{{{*/
	qsort_r(ps, n, sizeof(sh_point), radialcompare, q);
} /*}}}*/
void swap_points(sh_point *p, sh_point *q) { /*{{{*/
	sh_point tmp;
	memcpy(&tmp, p,    sizeof(sh_point));
	memcpy(p,    q,    sizeof(sh_point));
	memcpy(q,    &tmp, sizeof(sh_point));
} /*}}}*/
SHFLT plane_cross(const sh_point *a, const sh_point *b, const sh_point *c) { /*{{{*/
	return (b->x-a->x) * (c->y-a->y) - (b->y-a->y) * (c->x-a->x);
} /*}}}*/
SHFLT sqcircumradius(const sh_point *a, const sh_point *b, const sh_point *c) { /*{{{*/
	sh_point p = { .x = b->x-a->x, .y = b->y-a->y };
	sh_point q = { .x = c->x-a->x, .y = c->y-a->y };
	SHFLT p2 = p.x*p.x + p.y*p.y;
	SHFLT q2 = q.x*q.x + q.y*q.y;
	SHFLT d = 2*(p.x*q.y - p.y*q.x);
	if (d == 0) {
		return -1;
	}
	SHFLT x = (q.y*p2 - p.y*q2)/d;
	SHFLT y = (p.x*q2 - q.x*p2)/d;
	return x*x+y*y;
} /*}}}*/
SHFLT circumcircle(sh_point *r, const sh_point *a, const sh_point *b, const sh_point *c) { /*{{{*/
	sh_point p = { .x = b->x-a->x, .y = b->y-a->y };
	sh_point q = { .x = c->x-a->x, .y = c->y-a->y };
	SHFLT p2 = p.x*p.x + p.y*p.y;
	SHFLT q2 = q.x*q.x + q.y*q.y;
	SHFLT d = 2*(p.x*q.y - p.y*q.x);
	if (d == 0) {
		return -1;
	}
	SHFLT x = (q.y*p2 - p.y*q2)/d;
	SHFLT y = (p.x*q2 - q.x*p2)/d;
	if (r != NULL) {
		r->x = a->x + x;
		r->y = a->y + y;
	}
	return x*x+y*y;
} /*}}}*/
sh_triangle *create_triangle(sh_point *p, sh_point *q, sh_point *r) { /*{{{*/
	sh_triangle *t = malloc(sizeof(sh_triangle));
	assert(t != NULL);
	t->p[0] = p;
	t->p[1] = q;
	t->p[2] = r;
	t->ccr2 = circumcircle(&t->cc, p, q, r);
	t->e[0] = NULL;
	t->e[1] = NULL;
	t->e[2] = NULL;
	return t;
} /*}}}*/
sh_edge *create_edge(sh_point *p, sh_point *q, sh_triangle *t, sh_triangle *u) { /*{{{*/
	sh_edge *e = malloc(sizeof(sh_edge));
	assert(e != NULL);
	e->p[0] = p;
	e->p[1] = q;
	e->t[0] = t;
	e->t[1] = u;
	e->flipcount = 0;
	return e;
} /*}}}*/
void seed_triangulation(sh_triangulation_data *td, sh_point *ps) { /*{{{*/
	sh_triangle *tri = create_triangle(&ps[0], &ps[1], &ps[2]);
	sh_edge *e[3] = {
		create_edge(&ps[2], &ps[0], tri, NULL),
		create_edge(&ps[1], &ps[2], tri, NULL),
		create_edge(&ps[0], &ps[1], tri, NULL)
	};
	assert(e[0] != NULL);
	assert(e[1] != NULL);
	assert(e[2] != NULL);
	tri->e[0] = e[1];
	tri->e[1] = e[0];
	tri->e[2] = e[2];

	td->triangles = ll_insert_after(NULL, tri);

	td->hull_edges = ll_insert_after(NULL, e[0]);
	ll_glue(td->hull_edges, td->hull_edges); /* make circular */
	ll_insert_after(td->hull_edges, e[1]);
	ll_insert_after(td->hull_edges, e[2]);
	td->internal_edges = NULL;
} /*}}}*/
bool is_visible_to_point(void *a, void *b) { /*{{{*/
	sh_edge *e = (sh_edge *)a;
	sh_point *p = (sh_point *)b;
	SHFLT cross = plane_cross(p, e->p[0], e->p[1]);
	if (cross > 0) {
		return true;
	}
	return false;
} /*}}}*/
bool is_not_visible_to_point(void *a, void *b) { /*{{{*/
	sh_edge *e = (sh_edge *)a;
	sh_point *p = (sh_point *)b;
	SHFLT cross = plane_cross(p, e->p[0], e->p[1]);
	if (cross <= 0) {
		return true;
	}
	return false;
} /*}}}*/
void add_point_to_hull(sh_triangulation_data *td, sh_point *p) { /*{{{*/
	/*
	 * Find the first hull edge that is visible from the point p
	 * and the last hull edge that is visible from point p.
	 * Those edges, and the edges between, in the hull will be
	 * replaced by two edges, one leading to the point, and one
	 * leading from it.
	 * New triangles will be created from the combination of
	 * each visible edge and the point.
	 */

	ll_node *first_vis;
	ll_node *last_vis;
	ll_node *first_hid;
	ll_node *last_hid;

	if (is_visible_to_point(DATA(td->hull_edges), p)) { /*{{{*/
		first_hid = ll_cfind_r(td->hull_edges, is_not_visible_to_point, p);
		last_hid = ll_crfind_r(td->hull_edges, is_not_visible_to_point, p);
		first_vis = ll_cut_after(last_hid);
		last_vis = ll_cut_before(first_hid);
	}
	else {
		first_vis = ll_cfind_r(td->hull_edges, is_visible_to_point, p);
		last_vis = ll_crfind_r(td->hull_edges, is_visible_to_point, p);
		first_hid = ll_cut_after(last_vis);
		last_hid = ll_cut_before(first_vis);
	} /*}}}*/
	sh_edge *e0 = NULL;
	sh_edge *e1 = NULL;
	for (ll_node *n=first_vis; n!=NULL; n=NEXT(n)) { /*{{{*/
		sh_edge *e = (sh_edge *)DATA(n);
		sh_triangle *t = create_triangle(e->p[0], p, e->p[1]);
		td->triangles = ll_insert_before(td->triangles, t);

		e->t[1] = t;
		if (n == first_vis) {
			e0 = create_edge(e->p[0], p, t, NULL);
			last_hid = ll_insert_after(last_hid, e0);
			/* printf("visible edge %p is first_vis, and remains in hull\n", e); */
		}
		else {
			e0 = e1;
			e0->t[1] = t;
			/* printf("adding t=%p to e=%p\n", t, e0); */
			td->internal_edges = ll_insert_before(td->internal_edges, e0);
		}
		e1 = create_edge(p, e->p[1], t, NULL);

		/* the edge is given the same index as the point it is opposite of in the triangle */
		t->e[0] = e1;
		t->e[1] = e;
		t->e[2] = e0;

		if (n == last_vis) {
			last_hid = ll_insert_after(last_hid, e1);
		}
	} /*}}}*/
	ll_glue(last_vis, td->internal_edges);
	td->internal_edges = first_vis;
	ll_glue(last_hid, first_hid);
	td->hull_edges = first_hid;
} /*}}}*/
int triangulate(sh_triangulation_data *td, sh_point *ps, size_t n) { /*{{{*/

	int p0 = 0;
	delaunay_restart:
	if (p0 != 0) { /*{{{*/
		if (p0 == n) {
			fprintf(stderr, "can not triangulate this pointset\n");
			return false;
		}
		swap_points(&ps[0], &ps[p0]);
	} /*}}}*/
	/*
	 * {{{
	 * 1 Select a seed point p from the set of points.
	 * 2 Sort set of points according to distance to the seed point.
	 * 3 Find the point q closest to p.
	 * 4 Find the point r which yields the smallest circumcircle c for the triangle pqr.
	 * 5 Order the points pqr so that the system is right handed; this is the initial hull h.
	 * 6 Resort the rest of the set of points based on the distance to to the centre of c.
	 * 7 Sequentially add the points s of the set, based on distance, growing h. As points are added
	 *   to h, triangles are created containing s and edges of h visible to s.
	 * 8 When h contains all points, a non-overlapping triangulation has been created.
	 * -- to get Delaunay triangulation:
	 * 9 Adjacent pairs of triangles may require flipping to create a proper Delaunay triangulation
	 *   from the triangulation
	 * }}}
	 */
	/* find starting points */ /*{{{*/
	/*
	 * "Randomly" select the first point, then find the the point closest to it
	 * and put it on index 1.
	 * Then find the point which together with the other two creates the smallest
	 * circumcircle, and put that on index 2.
	 * Order the points so that they become a clockwise ordered triangle, and then
	 * sort all the other points based on closeness to the circumcenter.
	 */

	size_t i_best = 1;
	SHFLT r_best = sqdist(&ps[1], &ps[0]);
	for (size_t i=2; i<n; ++i) { /*{{{*/
		SHFLT r = sqdist(&ps[i], &ps[0]);
		if (r_best > r) {
			r_best = r;
			i_best = i;
		}
	} /*}}}*/
	swap_points(&ps[1], &ps[i_best]);

	i_best = 2;
	r_best = sqcircumradius(&ps[2], &ps[1], &ps[0]);
	if (r_best == -1) { /*{{{*/
		fprintf(stderr, "circumradius degenerate case\n");
		++p0;
		goto delaunay_restart;
	} /*}}}*/
	for (size_t i=3; i<n; ++i) { /*{{{*/
		SHFLT r = sqcircumradius(&ps[i], &ps[1], &ps[0]);
		if (r > -1 && r_best > r) {
			r_best = r;
			i_best = i;
		}
	} /*}}}*/
	swap_points(&ps[2], &ps[i_best]);
	/*}}}*/
	/* ensure positively winded starting triangle */ /*{{{*/
	SHFLT cross = plane_cross(&ps[0], &ps[1], &ps[2]);
	if (cross > 0) {
		swap_points(&ps[1], &ps[2]);
	}
	else if (cross == 0) {
		++p0;
		fprintf(stderr, "cross product degenerate case\n");
		goto delaunay_restart;
	}
	/*}}}*/
	/* calculate circumcircle centre and sort points based on distance */ /*{{{*/
	sh_point cc;
	SHFLT radius = circumcircle(&cc, &ps[0], &ps[1], &ps[2]);
	if (radius < 0) {
		++p0;
		goto delaunay_restart;
	}
	if (n > 4) {
		radialsort(ps+3, n-3, &cc);
	}
	/*}}}*/
	seed_triangulation(td, ps);
	/* iteratively add points to the hull */ /*{{{*/
	for (size_t i=3; i<n; ++i) {
		add_point_to_hull(td, &ps[i]);
	} /*}}}*/

	return 0;

} /*}}}*/
int find_common_index(const sh_triangle *t, const sh_point *p) { /*{{{*/
	for (size_t i=0; i<3; ++i) {
		if (p == t->p[i]) {
			return i;
		}
	}
	return -1;
} /*}}}*/
void *flip_if_necessary(void *a, void *b) { /*{{{*/
	sh_edge *e = (sh_edge *)a;
	struct flipdata *fd = (struct flipdata *)b;

	if (e->t[0] != NULL && e->t[1] != NULL) {

		/*
		 *                b0  1         0  a1
		 *  c *------------* *           * *------------* b1
		 *    |           / /             \ \           |
		 *    |   T0     / / * b1      c * \ \     T1   |
		 *    |         / / /|           |\ \ \         |
		 *    |        / / / |           | \ \ \        |
		 *    |       / / /  |           |  \ \ \       |
		 *    |      / / /   |           |   \ \ \      |
		 *    |     / / /    |           |    \ \ \     |
		 *    |    / / /     |           |     \ \ \    |
		 *    |   / / /      |           |      \ \ \   |
		 *    |  / / /       |           |       \ \ \  |
		 *    | / / /        |           |        \ \ \ |
		 *    |/ / /         |           |         \ \ \|
		 * a0 * / /     T1   |           |   T0     \ \ * d
		 *     / /           |           |           \ \
		 *    * *------------* d      a0 *------------* *
		 *   0  a1                                   b0  1
		 */

		const int a0 = find_common_index(e->t[0], e->p[0]);
		const int a1 = find_common_index(e->t[1], e->p[0]);
		const int b0 = find_common_index(e->t[0], e->p[1]);
		const int b1 = find_common_index(e->t[1], e->p[1]);
		/* if (e->t[0]->p[a0] != e->t[1]->p[a1]) { */
		/* 	printf("point a error\n"); */
		/* } */
		/* if (e->t[0]->p[b0] != e->t[1]->p[b1]) { */
		/* 	printf("point a error\n"); */
		/* } */
		const int c = 3 ^ a0 ^ b0;
		const int d = 3 ^ a1 ^ b1;

		if (a0==-1 || a1==-1 || b0==-1 || b1==-1) { /*{{{*/
			fprintf(stderr, "a0=%d a1=%d b0=%d b1=%d\n", a0, a1, b0, b1);
			debug_print_edge((void *)e, stderr);
			fprintf(stderr, "\n");
			assert(false);
		} /*}}}*/
		/* printf("---\n"); */
		/* debug_print_edge(e, stdout); */
		if (
#ifdef SH_MAXRADIUS
			e->t[0]->ccr2 > SH_MAXRADIUS || e->t[1]->ccr2 > SH_MAXRADIUS ||
#endif
			e->t[0]->ccr2 > sqdist(&e->t[0]->cc, e->t[1]->p[d]) ||
			e->t[1]->ccr2 > sqdist(&e->t[1]->cc, e->t[0]->p[c])) {
			++e->flipcount;
			if (e->flipcount > fd->maxflips) {
				debug_print_edge(e, stdout);
				SHFLT sqd;
				printf("\n");
				sqd = sqdist(&e->t[0]->cc, e->t[1]->p[d]);
				printf("%" PSHFLT " %" PSHFLT " %" PSHFLT "\n", e->t[0]->ccr2, sqd, e->t[0]->ccr2-sqd);
				sqd = sqdist(&e->t[1]->cc, e->t[0]->p[c]);
				printf("%" PSHFLT " %" PSHFLT " %" PSHFLT "\n", e->t[0]->ccr2, sqd, e->t[0]->ccr2-sqd);
				return a;
			}
			fd->flipped = true;
			/*
			 * --- ------------------------------------------------------------
			 * E:  p[0]  is changed so that it points at T0:p[c]
			 *     p[1]  is changed so that it points at T1:p[d]
			 * --- ------------------------------------------------------------
			 * T0: p[a0] stays the same
			 *     p[b0] is changed to point at T1:p[d]
			 *     p[c]  stays the same
			 *
			 *     e[a0] is changed so that it points at the common edge
			 *     e[b0] stays the same
			 *     e[c]  is changed so that it points the pointed at the edge T1:e[b1] and the edge is made to reciprocate
			 * --- ------------------------------------------------------------
			 * T1: p[a1] is changed so that it points at T0:p[c]
			 *     p[b1] stays the same
			 *     p[d]  stays the same
			 *
			 *     e[a1] stays the same
			 *     e[b1] is changed so that it points at the common edge
			 *     e[d]  is changed so that it points at the edge T0:e[a0] and the edge is made to reciprocate
			 * --- ------------------------------------------------------------
			 */
			sh_edge *t1eb1 = e->t[1]->e[b1];
			sh_edge *t0ea0 = e->t[0]->e[a0];

			/* --- ------------------------------------------------------------ */
			e->p[0] = e->t[0]->p[c];
			e->p[1] = e->t[1]->p[d];
			/* --- ------------------------------------------------------------ */
			e->t[0]->p[b0] = e->t[1]->p[d];

			e->t[0]->e[a0] = e;
			e->t[0]->e[c] = t1eb1;
			/* --- ------------------------------------------------------------ */
			e->t[1]->p[a1] = e->t[0]->p[c];

			e->t[1]->e[b1] = e;
			e->t[1]->e[d] = t0ea0;
			/* --- ------------------------------------------------------------ */

			if (t1eb1->t[0] == e->t[1]) { t1eb1->t[0] = e->t[0]; } else { t1eb1->t[1] = e->t[0]; }
			if (t0ea0->t[0] == e->t[0]) { t0ea0->t[0] = e->t[1]; } else { t0ea0->t[1] = e->t[1]; }

			e->t[0]->ccr2 = circumcircle(&e->t[0]->cc, e->t[0]->p[0], e->t[0]->p[1], e->t[0]->p[2]);
			e->t[1]->ccr2 = circumcircle(&e->t[1]->cc, e->t[1]->p[0], e->t[1]->p[1], e->t[1]->p[2]);
		}
	}
	return a;
} /*}}}*/
void *find_highest_flipcount(void *a, void *b) { /*{{{*/
	sh_edge *e = (sh_edge *)a;
	int *flipcount = (int *)b;
	if (*flipcount < e->flipcount) {
		*flipcount = e->flipcount;
	}
	return a;
} /*}}}*/
int make_delaunay(sh_triangulation_data *td) { /*{{{*/
	struct flipdata fd;
	fd.maxflips = ll_length(td->internal_edges);
	fd.maxflips *= fd.maxflips;
	fd.flipped = true;
	while (fd.flipped) {
		fd.flipped = false;
		ll_map_r(td->internal_edges, flip_if_necessary, &fd);
	}
	int flipcount = 0;
	ll_map_r(td->internal_edges, find_highest_flipcount, &flipcount);
	return flipcount;
}/*}}}*/
int delaunay(sh_triangulation_data *td, sh_point *ps, size_t n) { /*{{{*/
	if (triangulate(td, ps, n) == 0) {
		return make_delaunay(td);
	}
	return -1;
} /*}}}*/
