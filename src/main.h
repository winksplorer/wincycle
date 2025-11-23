#ifndef WINCYCLE_MAIN_H
#define WINCYCLE_MAIN_H

#include <X11/Xlib.h>

Window *winlist (Display *disp, unsigned long *len);
char *winame (Display *disp, Window win); 
void stolower(char* p);
void activate_window(Display *dpy, Window win);
int forkexec(const char* process);

#endif