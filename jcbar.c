/*
 * This file is part of JCWM.
 *
 * JCWM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * JCWM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with JCWM. If not, see <https://www.gnu.org/licenses/>.
 */


#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "commons.h"
#define NAME "jcwm"
#define PANEL_FG_CLR 0xFFFFFF

Display *display;
Window root, panel;
int screen;
int sw;


Bool show(Window w)
{
    Atom t = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom n = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    Atom at; int af;

    unsigned long nitems, bytes_after;
    Atom* data = NULL;

    if(XGetWindowProperty(display, w, t, 0, (~0L), 
    False, XA_ATOM, &at, &af, &nitems, &bytes_after,
     (unsigned char**)&data) == Success && data)
    {
        for(unsigned long i = 0; i < nitems; i++)
        {
            if(data[i] == n)
            {
                XFree(data);
                return True;
            }
        }
        XFree(data);
    }
    return False;
}

char *window_name(Window w) {
    Atom prop = XInternAtom(display, "_NET_WM_NAME", False);
    Atom type;
    int format;
    unsigned long nitems, bytes;
    unsigned char *data = NULL;

    if (XGetWindowProperty(display, w, prop, 0, (~0L), False, AnyPropertyType,
                           &type, &format, &nitems, &bytes, &data) == Success) {
        if (data) return (char *)data;
    }
    XFetchName(display, w, (char **)&data);
    return (char *)data;
}

void draw() {
    XClearWindow(display, panel);
    int x = 5;
    GC gc = XCreateGC(display, panel, 0, NULL);
    XSetForeground(display, gc, PANEL_FG_CLR);
    XDrawString(display, panel, gc, x, 16, NAME, strlen(NAME));
    x += 100;
    Atom p = XInternAtom(display, "_NET_CLIENT_LIST", False);
    Atom at;
    int af;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;

    if (XGetWindowProperty(display, root, p, 0, 1024, False, XA_WINDOW,
                           &at, &af,
                           &nitems, &bytes_after, &data) == Success) {
        if (data) {
            Window *wins = (Window *)data;
            for (unsigned long i = 0; i < nitems; i++) {
                if(!show(wins[i])) continue;
                char *name = window_name(wins[i]);
                if(strlen(name) > 32) name[32] = '\0';
                if (name) {
                    XDrawString(display, panel, gc, x, 16, name, strlen(name));
                    x += strlen(name) * 8 + 20;
                }
                free(name);
            }
            XFree(data);
        }
    }
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_now);
    XDrawString(display, panel, gc, sw - 50, 16, timebuf, strlen(timebuf));
    XFreeGC(display, gc);
}

void unmin(Window win) {
    XEvent e = {0};
    Atom active = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    e.xclient.type = ClientMessage;
    e.xclient.window = win;
    e.xclient.message_type = active;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 1;
    e.xclient.data.l[1] = CurrentTime;
    XSendEvent(display, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
}

int main() {
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "epic fail\n");
        return 1;
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    sw = DisplayWidth(display, screen);
    panel = XCreateSimpleWindow(display, root, 0, 0, sw, PANEL_HEIGHT, 0,
                                0xC0C0C0, 0xC0C0C0);
    XSelectInput(display, panel, ExposureMask | ButtonPressMask);
    XSetWindowAttributes pwa;
    pwa.override_redirect = True;
    XChangeWindowAttributes(display, panel, CWOverrideRedirect, &pwa);
    XMapWindow(display, panel);

    while (1) {
        draw();
        while (XPending(display)) {
            XEvent ev;
            XNextEvent(display, &ev);
            if (ev.type == Expose) {
                draw();
            } else if (ev.type == ButtonPress) {
                int cx = ev.xbutton.x;
                int x = 105;
                if(cx < 50)
                {launch(LAUNCHER);}
                Atom prop = XInternAtom(display, "_NET_CLIENT_LIST", False);
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char *data = NULL;

                if (XGetWindowProperty(display, root, prop, 0, 1024, False, XA_WINDOW,
                                       &actual_type, &actual_format,
                                       &nitems, &bytes_after, &data) == Success) {
                    if (data) {
                        Window *wins = (Window *)data;
                        for (unsigned long i = 0; i < nitems; i++) {
                            char *name = window_name(wins[i]);
                            if (name) {
                                int width = strlen(name) * 8 + 20;
                                if (cx >= x && cx <= x + width) {
                                    unmin(wins[i]);
                                }
                                x += width;
                            }
                            free(name);
                        }
                        XFree(data);
                    }
                }
            }
        }

        usleep(400000);
    }

    return 0;
}