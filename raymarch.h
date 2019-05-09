#ifndef RAYMARCH_H
#define RAYMARCH_H

struct ray
{
    double x0;
    double y0;
    double vx;
    double vy;
    int size;
    int len;
    double *x;
    double *y;

};
typedef struct ray ray;

struct lineSeg
{
    double x1;
    double y1;
    double x2;
    double y2;
};
typedef struct lineSeg lineSeg;

struct square
{
    double x;
    double y;
    double s;
};
typedef struct square square;


struct circle
{
    double x;
    double y;
    double r;
};
typedef struct circle circle;


struct scene
{
    int N_circle;
    int N_square;
    int N_ray;
    circle *circles;
    square *squares;
    ray *rays;
    int updated;
};
typedef struct scene scene;

void scene_init(scene *sc);
void scene_free(scene *sc);
void ray_free(ray *r);
void scene_add_circle(scene *sc, double x, double y, double r);
void scene_add_square(scene *sc, double x, double y, double s);
void scene_add_ray(scene *sc, double x, double y, double angle);
double min2(double a, double b);
double min3(double a, double b, double c);
double min4(double a, double b, double c, double d);
double max2(double a, double b);
double max3(double a, double b, double c);
double max4(double a, double b, double c, double d);
double dist(double x1, double y1, double x2, double y2);
double circle_signed_dist(circle *c, double x, double y);
double lineSeg_dist(lineSeg *l, double x, double y);
double square_signed_dist(square *s, double x, double y);
void ray_march(ray *r, scene *sc);

#endif
