#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <errno.h>
#include <ctype.h>

#include "main.h"

 
int main(int argc, char *argv[]) {
    // error if no requested window
    if (argc != 2) return printf("usage: wincycle <app name>\n"), 2;

    Display *disp = XOpenDisplay(NULL);
 
    if (!disp) {
        puts("no display!");
        return -1;
    }
 
    unsigned long len;
    Window *list;
    list = (Window*)winlist(disp,&len);
 
    char *name;
    for (unsigned long i = 0; i < len; i++) {
        name = winame(disp,list[i]);
        printf("-->%s<--\n",name);
        stolower(name);
        if (!strncmp(name, argv[1], strlen(argv[1]))) {
            printf("%s == %s\n", name, argv[1]);
            activate_window(disp, list[i]);
            free(name);
            return 0;
        }

        free(name);
    }
 
    XFree(list);
 
    XCloseDisplay(disp);
    return 0;
}
 
 
Window *winlist (Display *disp, unsigned long *len) {
    Atom prop = XInternAtom(disp,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;
 
    errno = 0;
    if (XGetWindowProperty(disp,XDefaultRootWindow(disp),prop,0,1024,False,XA_WINDOW,
                &type,&form,len,&remain,&list) != Success) {
        perror("winlist() -- GetWinProp");
        return 0;
    }
     
    return (Window*)list;
}
 
 
char *winame (Display *disp, Window win) {
    XClassHint hint;
    char* n = malloc(64);
    if (XGetClassHint(disp, win, &hint)) {
        n = strncpy(n, hint.res_class, 64);
        XFree(hint.res_name);
        XFree(hint.res_class);
    }

    return n;
}

void stolower(char* p) {
    for ( ; *p; ++p) *p = tolower(*p);
}

void activate_window(Display *dpy, Window win) {
    XClientMessageEvent e = {0};
    Atom net_active = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    Atom net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);

    e.type = ClientMessage;
    e.window = win;
    e.message_type = net_active;
    e.format = 32;

    e.data.l[0] = 1;  // 1 = request active (source: application)
    e.data.l[1] = CurrentTime;
    e.data.l[2] = 0;  // usually unused

    XSendEvent(
        dpy,
        DefaultRootWindow(dpy),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        (XEvent*)&e
    );

    XFlush(dpy);
}