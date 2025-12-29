#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "main.h"

 
int main(int argc, char* argv[]) {
    // error if no requested window
    if (argc < 2 || argc > 3) return printf("usage: wincycle <app name> <optional: program to launch>\n"), 2;

    // display
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) return printf("no display!\n"), 1;
 
    // _net_client_list
    unsigned long len;
    Window* list = (Window*)winlist(dpy,&len);

    // match
    Window matches[64];
    int nmatches = filter_by_class(dpy, list, len, argv[1], matches, 64);

    // cycle & activate
    if (nmatches) {
        Window active = get_active_window(dpy);

        int i = index_of(matches, nmatches, active);

        if (i >= 0) activate_window(dpy, matches[(i+1) % nmatches]);
        else activate_window(dpy, matches[0]);

        XFree(list);
        XCloseDisplay(dpy);

        return 0;
    }
 
    XFree(list);
    XCloseDisplay(dpy);

    // if no results
    printf("could not find any open \"%s\", going to exec now\n", argv[1]);
    char* args[] = { argc > 2 ? argv[2] : argv[1], NULL };
    execvp(args[0], args);

    // if we still have control then exec failed
    perror("could not exec");
    return 1;
}
 
 
Window* winlist(Display* dpy, unsigned long* len) {
    Atom prop = XInternAtom(dpy,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char* list;

    errno = 0;
    if (XGetWindowProperty(dpy,XDefaultRootWindow(dpy),prop,0,1024,False,XA_WINDOW,
                &type,&form,len,&remain,&list) != Success) {
        perror("winlist: XGetWindowProperty");
        return 0;
    }
     
    return (Window*)list;
}
 
 
char* winame(Display* dpy, Window win) {
    XClassHint hint;
    char* n = malloc(64);
    if (XGetClassHint(dpy, win, &hint)) {
        n = strncpy(n, hint.res_class, 64);
        XFree(hint.res_name);
        XFree(hint.res_class);
    }

    return n;
}

Window get_active_window(Display* dpy) {
    Atom prop = XInternAtom(dpy,"_NET_ACTIVE_WINDOW",False), type;
    int form;
    unsigned long len;
    unsigned long remain;
    unsigned char* data;

    errno = 0;
    if (XGetWindowProperty(dpy,XDefaultRootWindow(dpy),prop,0,1024,False,XA_WINDOW,
                &type,&form,&len,&remain,&data) != Success) {
        perror("get_active_window: XGetWindowProperty");
        return 0;
    }
     
    Window win = ((Window*) data)[0];
    XFree(data);
    return win;
}

int filter_by_class(Display* dpy, Window* in, int nin, const char* wclass, Window* out, int maxout) {
    int nout = 0;
    for (int i = 0; i < nin && nout < maxout; i++) {
        // get name
        char* name = winame(dpy, in[i]);
        stolower(name);

        // compare
        if (!strcmp(name, wclass) ) out[nout++] = in[i];
        free(name);
    }
    
    return nout;
}

int index_of(Window* list, int n, Window w) {
    for (int i = 0; i < n; i++)
        if (list[i] == w) return i;
    return -1;
}

void activate_window(Display* dpy, Window win) {
    XClientMessageEvent e = {0};
    Atom net_active = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    Atom net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);

    e.type = ClientMessage;
    e.window = win;
    e.message_type = net_active;
    e.format = 32;

    e.data.l[0] = 1;
    e.data.l[1] = CurrentTime;
    e.data.l[2] = 0;

    XSendEvent(
        dpy,
        DefaultRootWindow(dpy),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        (XEvent*)&e
    );

    XFlush(dpy);
}

void stolower(char* p) {
    for (;*p;++p) *p = tolower(*p);
}