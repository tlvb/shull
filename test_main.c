#include "shull.h"
#include "ll.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define NPOINTS 25

sh_point *random_unique_points(size_t n) { /*{{{*/ /*{{{*/
	sh_point *ps = malloc(sizeof(sh_point)*n);
	assert(ps != NULL);
	for (size_t i=0; i<n; ++i) {
		bool dupe = true;

		/* make sure points are unique */
		while (dupe) {
			ps[i].x = (int) (((SHFLT)(rand()%1000)/10));
			ps[i].y = (int) (((SHFLT)(rand()%1000)/10));
			dupe = false;
			for (size_t j=0; j<i; ++j) {
				if (ps[i].x == ps[j].x && ps[i].y == ps[j].y) {
					dupe = true;
					break;
				}
			}
		}

	}
	return ps;
} /*}}}*/ /*}}}*/
void scale_points(sh_point *ps, size_t n) { /*{{{*/
	/* scale */
	SHFLT minx = 101;
	SHFLT maxx = -1;
	SHFLT miny = 101;
	SHFLT maxy = -1;
	for (size_t i=0; i<n; ++i) {
		if (minx > ps[i].x) { minx = ps[i].x; }
		if (maxx < ps[i].x) { maxx = ps[i].x; }
		if (miny > ps[i].y) { miny = ps[i].y; }
		if (maxy < ps[i].y) { maxy = ps[i].y; }
	}
	SHFLT scalex = 100 / (maxx-minx);
	SHFLT scaley = 100 / (maxy-miny);
	for (size_t i=0; i<n; ++i) {
		ps[i].x = (ps[i].x-minx)*scalex;
		ps[i].y = (ps[i].y-miny)*scaley;
	}
}/*}}}*/
sh_point *random_framed_unique_points(size_t n) { /*{{{*/
	sh_point *ps = random_unique_points(n);
	ps[0].x =   0; ps[0].y =   0;
	ps[1].x =   0; ps[1].y = 100;
	ps[2].x = 100; ps[2].y =   0;
	ps[3].x = 100; ps[3].y = 100;
	return ps;
} /*}}}*/
sh_point *random_scaled_unique_points(size_t n) { /*{{{*/
	sh_point *ps = random_unique_points(n);
	scale_points(ps, n);
	return ps;
}/*}}}*/
void write_seed(const char *filename, unsigned int seed) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	fprintf(fd, "%u\n", seed);
	fclose(fd);
} /*}}}*/
void write_points(const char *filename, const sh_point *ps, size_t n) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	for (size_t i=0; i<n; ++i) {
		fprintf(fd, "%" PSHFLT ",%" PSHFLT "\n", ps[i].x, ps[i].y);
	}
	fclose(fd);
} /*}}}*/
void *print_edge(void *a, void *b) { /*{{{*/
	sh_edge *e = (sh_edge *)a;
	FILE *fd = (FILE *)b;
	fprintf(
		fd, "%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT "\n",
		e->p[0]->x, e->p[0]->y,
		e->p[1]->x, e->p[1]->y
	);
	return a;
} /*}}}*/
void *print_triangle(void *a, void *b) { /*{{{*/
	sh_triangle *t = (sh_triangle *)a;
	FILE *fd = (FILE *)b;
	fprintf(
		fd, "%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT "\n",
		t->p[0]->x, t->p[0]->y,
		t->p[1]->x, t->p[1]->y,
		t->p[2]->x, t->p[2]->y
	);
	return a;
} /*}}}*/
void *print_triangle_circumcircle(void *a, void *b) { /*{{{*/
	sh_triangle *t = (sh_triangle *)a;
	FILE *fd = (FILE *)b;
	fprintf(
		fd, "%" PSHFLT ",%" PSHFLT ",%" PSHFLT "\n",
		t->cc.x, t->cc.y, t->ccr2
	);
	return a;
} /*}}}*/
void write_triangles(const char *filename, const sh_triangle *ts, size_t n) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	for (size_t i=0; i<n; ++i) {
		fprintf(
			fd, "%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT ",%" PSHFLT "\n",
			ts[i].p[0]->x, ts[i].p[0]->y,
			ts[i].p[1]->x, ts[i].p[1]->y,
			ts[i].p[2]->x, ts[i].p[2]->y
		);
	}
	fclose(fd);
} /*}}}*/
void write_edge_list(const char *filename, ll_node *first) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	ll_cmap_r(first, print_edge, fd);
	fclose(fd);
} /*}}}*/
void write_triangle_list(const char *filename, ll_node *first) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	ll_map_r(first, print_triangle, fd);
	fclose(fd);
} /*}}}*/
void write_triangle_circumcircle_list(const char *filename, ll_node *first) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	ll_map_r(first, print_triangle_circumcircle, fd);
	fclose(fd);
} /*}}}*/
void write_circle(const char *filename, const sh_point *cc, SHFLT radius) { /*{{{*/
	FILE *fd = fopen(filename, "w");
	assert(fd);
	fprintf(fd, "%" PSHFLT ",%" PSHFLT ",%" PSHFLT "\n", cc->x, cc->y, radius);
	fclose(fd);
} /*}}}*/

int main(int argc, char **argv) {
	sh_point *points = NULL;
	unsigned int seed = getpid() + time(NULL);
	size_t n = NPOINTS;
	bool frame = false;

	for (int i=1; i<argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'f':
					frame = true;
					break;
				case 's':
					seed = (unsigned int) strtoul(argv[++i], NULL, 0);
					break;
				case 'n':
					n = (unsigned int) strtoul(argv[++i], NULL, 0);
					break;
			}
		}
	}

	write_seed("data/seed.txt", seed);
	printf("USING SEED %u\n", seed);
	srand(seed);

	if (frame) {
		points = random_framed_unique_points(n);
	}
	else {
		points = random_scaled_unique_points(n);
	}

	sh_triangulation_data td;
	int result = delaunay(&td, points, n);

	if (result > 0) {

		printf("highest edge flip count = %d\n", result);

		write_points("data/points.txt", points, n);
		write_edge_list("data/hull_edges.txt", td.hull_edges);
		write_edge_list("data/internal_edges.txt", td.internal_edges);
		write_triangle_list("data/triangles.txt", td.triangles);
		write_triangle_circumcircle_list("data/circumcircles.txt", td.triangles);

		ll_mapdestroy(td.triangles, free);
		ll_mapdestroy(td.hull_edges, free);
		ll_mapdestroy(td.internal_edges, free);
	}
	else {
		printf("could not make a delaunay triangulation of these points\n");
		write_seed("data/failed_seed.txt", seed);
		write_points("data/failed_points.txt", points, n);
	}

	free(points);
	return 0;
}
