#include <stdlib.h>
#include <math.h>
#include "raymarch.h"

void scene_init(scene *sc)
{
    sc->circles = NULL;
    sc->N_circle = 0;
    sc->squares = NULL;
    sc->N_square = 0;
    sc->rays = NULL;
    sc->N_ray = 0;
    sc->updated = 0;
}

void scene_free(scene *sc)
{
    free(sc->circles);
    free(sc->squares);
    int i;
    for(i=0; i<sc->N_ray; i++)
        ray_free(sc->rays + i);
}

void ray_free(ray *r)
{
    free(r->x);
    free(r->y);
}

void scene_add_circle(scene *sc, double x, double y, double r)
{
    sc->N_circle += 1;
    sc->circles = (circle *)realloc(sc->circles,
                                    sc->N_circle * sizeof(circle));
    sc->circles[sc->N_circle-1].x = x;
    sc->circles[sc->N_circle-1].y = y;
    sc->circles[sc->N_circle-1].r = r;
    sc->updated = 1;
}

void scene_add_square(scene *sc, double x, double y, double s)
{
    sc->N_square += 1;
    sc->squares = (square *)realloc(sc->squares,
                                    sc->N_square * sizeof(square));
    sc->squares[sc->N_square-1].x = x;
    sc->squares[sc->N_square-1].y = y;
    sc->squares[sc->N_square-1].s = s;
    sc->updated = 1;
}

void scene_add_ray(scene *sc, double x, double y, double angle)
{
    sc->N_ray += 1;
    sc->rays = (ray *)realloc(sc->rays, sc->N_ray * sizeof(ray));
    sc->rays[sc->N_ray-1].x0 = x;
    sc->rays[sc->N_ray-1].y0 = y;
    sc->rays[sc->N_ray-1].vx = cos(angle);
    sc->rays[sc->N_ray-1].vy = sin(angle);

    int size = 10;
    int len = 0;
    sc->rays[sc->N_ray-1].x = (double *)malloc(size * sizeof(double));
    sc->rays[sc->N_ray-1].y = (double *)malloc(size * sizeof(double));
    sc->rays[sc->N_ray-1].size = size;
    sc->rays[sc->N_ray-1].len = len;

    sc->updated = 1;
}

double min2(double a, double b)
{
    return a < b ? a : b;
}

double min3(double a, double b, double c)
{
    return min2(min2(a, b), c);
}

double min4(double a, double b, double c, double d)
{
    return min2(min2(a, b), min2(c, d));
}

double max2(double a, double b)
{
    return a > b ? a : b;
}

double max3(double a, double b, double c)
{
    return max2(max2(a, b), c);
}

double max4(double a, double b, double c, double d)
{
    return max2(max2(a, b), max2(c, d));
}

double dist(double x1, double y1, double x2, double y2)
{
    double dx = x2-x1;
    double dy = y2-y1;
    return sqrt(dx*dx + dy*dy);
}

double circle_signed_dist(circle *c, double x, double y)
{
    return dist(c->x, c->y, x, y) - c->r;
}

double lineSeg_dist(lineSeg *l, double x, double y)
{
    double dxl = l->x2 - l->x1;
    double dyl = l->y2 - l->y1;

    if(dxl == 0.0 && dyl == 0.0)
        return dist(l->x1, l->y1, x, y);

    double t = ((x-l->x1)*dxl + (y-l->y1)*dyl) / (dxl*dxl + dyl*dyl);

    if(t <= 0.0)
        return dist(l->x1, l->y1, x, y);
    else if(t >= 1.0)
        return dist(l->x2, l->y2, x, y);

    return dist(l->x1 + t*dxl, l->y1 + t*dyl, x, y);
}

double square_signed_dist(square *s, double x, double y)
{
   lineSeg l;
   l.x1 = s->x;         l.y1 = s->y;
   l.x2 = s->x + s->s;  l.y2 = s->y;
   double d1 = lineSeg_dist(&l, x, y);
   l.x1 = l.x2;         l.y1 = l.y2;
   l.x2 = s->x + s->s;  l.y2 = s->y + s->s;
   double d2 = lineSeg_dist(&l, x, y);
   l.x1 = l.x2;         l.y1 = l.y2;
   l.x2 = s->x;         l.y2 = s->y + s->s;
   double d3 = lineSeg_dist(&l, x, y);
   l.x1 = l.x2;         l.y1 = l.y2;
   l.x2 = s->x;         l.y2 = s->y;
   double d4 = lineSeg_dist(&l, x, y);

   double dmin = min4(d1, d2, d3, d4);
   double dmax = max4(d1, d2, d3, d4);

   if(dmax < s->s)
       return -dmin;

   return dmin;
}

void ray_march(ray *r, scene *sc)
{
    if(r->len == r->size)
    {
        r->size *= 2;
        r->x = (double *)realloc(r->x, r->size * sizeof(double));
        r->y = (double *)realloc(r->y, r->size * sizeof(double));
    }

    int i;
    double d = 1.0e100;

    double x0 = r->len == 0 ? r->x0 : r->x[r->len-1];
    double y0 = r->len == 0 ? r->y0 : r->y[r->len-1];

    for(i=0; i<sc->N_circle; i++)
    {
        double dc = circle_signed_dist(sc->circles+i, x0, y0);
        if(dc < d)
            d = dc;
    }
    for(i=0; i<sc->N_square; i++)
    {
        double ds = square_signed_dist(sc->squares+i, x0, y0);
        if(ds < d)
            d = ds;
    }

    if(d == 1.0e100)
        return;

    r->x[r->len] = x0 + d * r->vx;
    r->y[r->len] = y0 + d * r->vy;
    r->len += 1;
    sc->updated = 1;
}
