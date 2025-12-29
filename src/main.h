#ifndef WINCYCLE_MAIN_H
#define WINCYCLE_MAIN_H

#include <X11/Xlib.h>

Window* winlist (Display* dpy, unsigned long* len);
char* winame (Display* dpy, Window win); 
Window get_active_window(Display* dpy);
int filter_by_class(Display* dpy, Window* in, int nin, const char* wclass, Window* out, int maxout);
int index_of(Window* list, int n, Window w);
void stolower(char* p);
void activate_window(Display* dpy, Window win);

#endif