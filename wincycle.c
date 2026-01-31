#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static void
usage(const char *argv0)
{
    printf("usage: %s [window class] [optional: binary to run]\n", argv0);
    exit(2);
}

static Window*
win_list(Display *dpy, unsigned long *len) {
    Atom prop = XInternAtom(dpy,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;

    if (XGetWindowProperty(dpy,XDefaultRootWindow(dpy),prop,0,1024,False,XA_WINDOW,
                &type,&form,len,&remain,&list)) {
        perror("winlist: XGetWindowProperty");
        return NULL;
    }
     
    return (Window*)list;
}
 
 
static char*
win_name(Display *dpy, Window win) {
    XClassHint hint;
    char *n = malloc(64);

    if (XGetClassHint(dpy, win, &hint)) {
        n = strncpy(n, hint.res_class, 63);
        XFree(hint.res_name);
        XFree(hint.res_class);
    }

    return n;
}

static Window
active_win(Display *dpy) {
    Atom prop = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False), type;
    int form;
    unsigned long len;
    unsigned long remain;
    unsigned char *data;

    if (XGetWindowProperty(dpy,XDefaultRootWindow(dpy),prop,0,1024,False,XA_WINDOW,
                &type,&form,&len,&remain,&data) != Success) {
        perror("get_active_window: XGetWindowProperty");
        return 0;
    }
     
    Window win = ((Window*) data)[0];
    XFree(data);
    return win;
}

static void
stolower(char *p) {
    for (;*p;++p)
        *p = tolower(*p);
}

static int
win_class_filter(Display *dpy, Window *in, int nin, const char *wclass, Window *out, int maxout) {
    int nout = 0, i = 0;
    for (i = 0; i < nin && nout < maxout; i++) {
        char *name = win_name(dpy, in[i]);
        stolower(name);

        if (!strcmp(name, wclass) ) out[nout++] = in[i];
        free(name);
    }
    
    return nout;
}

static int 
index_of(Window *list, int n, Window w) {
    int i;

    if (!list)
        return -1;
    for (i = 0; i < n; i++)
        if (list[i] == w) return i;
    
    return -1;
}

static void
activate_win(Display *dpy, Window win) {
    XWindowAttributes attributes;
    XEvent e = { .xclient = {
        .type = ClientMessage,
        .window = win,
        .message_type = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False),
        .format = 32,
        .data.l = {1, CurrentTime, 0}
    }};

    XSendEvent(dpy, DefaultRootWindow(dpy), False,
        SubstructureRedirectMask | SubstructureNotifyMask, &e);

    XGetWindowAttributes(dpy, win, &attributes);
    XWarpPointer(dpy, None, win, 0, 0, 0, 0, attributes.width>>1, attributes.height>>1);
    XFlush(dpy);
}

 
int
main(int argc, char *argv[]) {
    Display *dpy;
    unsigned long len;
    Window* list;
    int nmatches;
    Window matches[64];
    char *args[2] = {NULL};
    int i;

    // valid args?
    if (argc < 2 || argc > 3) usage(argv[0]);

    // display?
    dpy = XOpenDisplay(NULL);
    if (!dpy) return printf("no display\n"), 1;
 
    // filter the window list
    list = (Window*)win_list(dpy,&len);
    nmatches = win_class_filter(dpy, list, len, argv[1], matches, 64);

    // matching windows? activate
    if (nmatches) {
        i = index_of(matches, nmatches, active_win(dpy));
        activate_win(dpy, i >= 0 ? matches[(i+1) % nmatches] : matches[0]);
    }
 
    XFree(list);
    XCloseDisplay(dpy);

    if (nmatches)
        return 0;

    // exec
    printf("could not find any open \"%s\", going to exec now\n", argv[1]);
    args[0] = argc > 2 ? argv[2] : argv[1];
    execvp(args[0], args);

    perror("execvp");
    return 1;
}