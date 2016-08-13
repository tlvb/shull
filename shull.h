#ifndef shull_h
#define shull_h

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "ll.h"

#define SHFLT int
#define PSHFLT "d"

typedef struct sh_point sh_point;
typedef struct sh_edge sh_edge;
typedef struct sh_triangle sh_triangle;

struct sh_point {
	SHFLT x;
	SHFLT y;
};

struct sh_edge {
	sh_point *p[2];
	sh_triangle *t[2];
	unsigned int flipcount;
};

struct sh_triangle {
	sh_point *p[3];
	sh_edge *e[3];
	sh_point cc;
	SHFLT ccr2;
};

typedef struct {
	ll_node *hull_edges;
	ll_node *internal_edges;
	ll_node *triangles;
} sh_triangulation_data;

int triangulate(sh_triangulation_data *td, sh_point *ps, size_t n);
int make_delaunay(sh_triangulation_data *td);
int delaunay(sh_triangulation_data *td, sh_point *ps, size_t n);

#endif
