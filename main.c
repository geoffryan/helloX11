#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "raymarch.h"

struct displaySetup
{
    Display *dis;
    int screen;
    Window win;
    GC gc;
    Colormap cm;
    XColor black;
    XColor white;
    XColor red;
    XColor green;
    XColor blue;
    XColor yellow;
    XColor cyan;
    XColor magenta;
};

void init_X(struct displaySetup *ds)
{
    int ww = 400;
    int wh = 300;

    unsigned long black, white;

    printf("Opening.\n");
    ds->dis = XOpenDisplay((char *)0);
    printf("Make screen.\n");
    ds->screen = DefaultScreen(ds->dis);
    printf("Get colors.\n");
    black = BlackPixel(ds->dis, ds->screen);
    white = WhitePixel(ds->dis, ds->screen);

    printf("Create window.\n");
    ds->win = XCreateSimpleWindow(ds->dis, DefaultRootWindow(ds->dis), 0, 0,
                                    ww, wh, 0, black, white);

    printf("Set Properties.\n");
    XSetStandardProperties(ds->dis, ds->win, "My Window", "HI!", None, NULL, 0,
                            NULL);

    printf("Select Input.\n");
    XSelectInput(ds->dis, ds->win, ExposureMask|ButtonPressMask|KeyPressMask);

    printf("Create GC.\n");
    ds->gc = XCreateGC(ds->dis, ds->win, 0, 0);

    printf("Set Colors.\n");
    XSetBackground(ds->dis, ds->gc, white);
    XSetForeground(ds->dis, ds->gc, black);

    printf("Clear window.\n");
    XClearWindow(ds->dis, ds->win);
    
    printf("Raise map.\n");
    XMapRaised(ds->dis, ds->win);

    printf("Get Colormap.\n");
    ds->cm = DefaultColormap(ds->dis, ds->screen);
    
    printf("Get Colors.\n");
    XAllocNamedColor(ds->dis, ds->cm, "black", &(ds->black), &(ds->black));
    XAllocNamedColor(ds->dis, ds->cm, "white", &(ds->white), &(ds->white));
    XAllocNamedColor(ds->dis, ds->cm, "red", &(ds->red), &(ds->red));
    XAllocNamedColor(ds->dis, ds->cm, "green", &(ds->green), &(ds->green));
    XAllocNamedColor(ds->dis, ds->cm, "blue", &(ds->blue), &(ds->blue));
    XAllocNamedColor(ds->dis, ds->cm, "cyan", &(ds->cyan), &(ds->cyan));
    XAllocNamedColor(ds->dis, ds->cm, "yellow", &(ds->yellow), &(ds->yellow));
    XAllocNamedColor(ds->dis, ds->cm, "magenta", &(ds->magenta), 
                        &(ds->magenta));
}

void close_X(struct displaySetup *ds)
{
    printf("Closing.\n");
    printf("Free GC.\n");
    XFreeGC(ds->dis, ds->gc);
    printf("Destroy Window.\n");
    XDestroyWindow(ds->dis, ds->win);
    printf("Close Display.\n");
    XCloseDisplay(ds->dis);
    printf("Closed.\n");
}

void drawImage(struct displaySetup *ds)
{
    Visual *vis = DefaultVisual(ds->dis, ds->screen);
    int depth = DefaultDepth(ds->dis, ds->screen);

    char *data = (char *)malloc(256*256*4*sizeof(char));
    XImage *im = XCreateImage(ds->dis, vis, depth, ZPixmap,
                                0, data, 256, 256, 32, 0);

    int i, j;
    for(i=0; i<256; i++)
        for(j=0; j<256; j++)
        {
            XPutPixel(im, i, j, i+j);
        }

    XPutImage(ds->dis, ds->win, ds->gc, im, 0, 0, 0, 0, 256, 256);

    XDestroyImage(im);
}

void drawCircle(struct displaySetup *ds, circle *c)
{
    XDrawArc(ds->dis, ds->win, ds->gc, c->x-c->r, c->y-c->r, 2*c->r, 2*c->r,
                0, 64*360);
}

void drawSquare(struct displaySetup *ds, square *s)
{
    XDrawRectangle(ds->dis, ds->win, ds->gc, s->x, s->y, s->s, s->s);
}

void drawRay(struct displaySetup *ds, ray *r)
{
    XSetForeground(ds->dis, ds->gc, ds->red.pixel);

    int i;
    for(i=0; i<r->len; i++)
    {
        circle c;
        if(i==0)
        {
            c.x = r->x0;
            c.y = r->y0;
        }
        else
        {
            c.x = r->x[i-1];
            c.y = r->y[i-1];
        }
        c.r = dist(c.x, c.y, r->x[i], r->y[i]);
        drawCircle(ds, &c);
    }
    XSetForeground(ds->dis, ds->gc, ds->blue.pixel);
    XFillArc(ds->dis, ds->win, ds->gc, r->x0-5, r->y0-5, 10, 10, 0, 64*360);
    for(i=0; i<r->len; i++)
    {
        if(i == 0)
            XDrawLine(ds->dis, ds->win, ds->gc, r->x0, r->y0, 
                        r->x[i], r->y[i]);
        else
            XDrawLine(ds->dis, ds->win, ds->gc, r->x[i-1], r->y[i-1],
                        r->x[i], r->y[i]);
        XFillArc(ds->dis, ds->win, ds->gc, r->x[i]-5, r->y[i]-5, 10, 10, 0,
                 64*360);
    }
}

void drawScene(struct displaySetup *ds, scene *sc)
{
    int i;
    XSetForeground(ds->dis, ds->gc, ds->black.pixel);
    for(i=0; i<sc->N_circle; i++)
        drawCircle(ds, sc->circles + i);
    
    for(i=0; i<sc->N_square; i++)
        drawSquare(ds, sc->squares + i);
    
    for(i=0; i<sc->N_ray; i++)
        drawRay(ds, sc->rays + i);
}

void addCircle(scene *sc, int x, int y)
{
    scene_add_circle(sc, x, y, 30);
}

void addSquare(scene *sc, int x, int y)
{
    scene_add_square(sc, x, y, 30);
}

void addRay(scene *sc, int x, int y)
{
    scene_add_ray(sc, x, y, 0.0);
}

void marchRays(scene *sc)
{
    int i;
    for(i=0; i<sc->N_ray; i++)
        ray_march(sc->rays+i, sc);
}

void eventLoop(struct displaySetup *ds, scene *sc)
{
    XEvent event;
    KeySym key;
    char text[256];

    while(1)
    {
        XNextEvent(ds->dis, &event);

        if(event.type==KeyPress)
        {
            int ret = XLookupString(&event.xkey, text, 256, &key, 0);
            if(!ret)
                continue;

            if(text[0] == 'q')
                return;
            else if(text[0] == 'i')
                drawImage(ds);
            else if(text[0] == 'c')
                addCircle(sc, event.xkey.x, event.xkey.y);
            else if(text[0] == 's')
                addSquare(sc, event.xkey.x, event.xkey.y);
            else if(text[0] == 'r')
                addRay(sc, event.xkey.x, event.xkey.y);
            else if(text[0] == 'm')
                marchRays(sc);
            else
                printf("You pressed %c\n", text[0]);
        }

        if(event.type==ButtonPress)
        {
            printf("Mouse pressed at (%d, %d)\n", event.xbutton.x,
                    event.xbutton.y);
        }

        if(sc->updated)
        {    
            drawScene(ds, sc);
            sc->updated = 0;
        }

        //sleep(1);
    }
}

int main(int argc, char *argv[])
{
    struct displaySetup ds = {0};

    scene sc;
    scene_init(&sc);

    printf("Initing.\n");
    init_X(&ds);
    printf("Working.\n");
    eventLoop(&ds, &sc);
    close_X(&ds);

    scene_free(&sc);

    return 0;
}
