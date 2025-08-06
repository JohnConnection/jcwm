/*
 * This file is part of JCWM.
 *
 * JCWM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * JCWM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with JCWM. If not, see <https://www.gnu.org/licenses/>.
 */



/*
JCWM - Written by John Connection in 2025

JCWM stands for "John Connection Window Manager". It's the result of countless attempts at 
writing a window manager. I have no idea WHY, but in early 2024, the thought of writing a Window Manager
piqued my interest, so over the course of about a year, I tried writing a window manager. In the process,
I was learning about X11/Xlib. I think I tried writing a wm about 10-15 times or so, but it was
always plagued by various issues, and they fell apart rather quickly due to a poor design, esp. code wise.
I thought I'd try again earlier this year, and JCWM is the result. Its' code is partly based on/inspired by dwm, because its
one-source-file-approach made it easy for me to understand how it's supposed to be done. As a result, this WM did not fall apart like
a house of cards after two days. It has gotten to the stage where I am ready to release it in Alpha. 
There are still things I'd like to add, but at this point I think it's pretty usable. If you have any 
questions/bug reports/feature requests/
please do not hesitate to message me.

NOTE: jcwm is meant to be used with jcbar. You can try using it without, but that would get very technical
very quick, so I do not recommend it if you don't know what you're doing.

NOTE: Please configure the WM via the few Macros below.
*/
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "commons.h"

//Uncomment this if you want to be able to read the one or two DEBUG messages in the code
//#define DEBUG
#define FRAME_HEIGHT 20
#define FRAME_BORDER_WIDTH 3
#define FRAME_BORDER_COLOR 0x808080
#define FRAME_INACTIVE_BORDER_COLOR 0xC0C0C0
#define FRAME_BACKGROUND_COLOR 0xFFFFFF

#define VERSION "JCWM - Alpha 1.2_04"

#define FRAME_WIDTH_INC 10
#define MIN_SIZE 32

//Depending on what colour your background is, you may change the color of the Watermark to one that suits your setup
#define WATERMARK_CLR 0xC0C0C0


typedef struct Client Client;
typedef struct Monitor Monitor;
struct Client
{
    Window win, frame;
    int x, y, w, h, bw;
    int oldx, oldy, oldw, oldh, oldbw;
    int basew, baseh, maxw, maxh, minw, minh, incw, inch, hintsvalid;
    int neverfocus;
    int maximized, minimized, fullscreen;
    int is_normal, is_dialog, is_toolbar, is_splashscreen, is_dropdown;
    Client* next;
    Client* snext;
    Monitor* mon;
    XSetWindowAttributes winevmask;
    XSetWindowAttributes frameevmask;
    char name[256];
};
struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;	      
	int mx, my, mw, mh;   
	int wx, wy, ww, wh;   
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
};

enum CursorStates {NORMAL = XC_center_ptr, MOV = XC_fleur, RES = XC_double_arrow, DEL = XC_cross};
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
    NetWMFullscreen, NetActiveWindow, NetWMWindowType,
    NetWMWindowTypeDialog, NetClientList,NetWMStateHidden, NetLast,NetWMWindowTypeDesktop, NetWMWindowTypeNormal,NetWMWindowTypeDock, NetWMWindowTypeToolbar,NetWMWindowTypeUtility,NetWMWindowTypeSplash}; 
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast };


Display *display;
Window root;
int screen;
int ccount = 0;
int wm_y = 0;
Cursor normal, mov, res, del;
static Atom wmatom[WMLast], netatom[NetLast];

Client* clients = NULL;
Monitor* monitors = NULL, *selmon;
Client *lastfocused = NULL;

/*function declarations, uncomplete*/
Monitor* getmon(int x, int y);
static void update_monitors(void);
Client* find_client(Window w);
void remove_client(Window w);
void maprequest(XEvent* e);
void updatename(Client* c);
void manage(Window w);
void draw_name(Client* c);
void init(void);
void run(void);
int gettextprop(Window w, Atom atom, char *text, unsigned int size);
void configure(Client *c);
void buttonpress(XEvent* e);
void buttonrelease(XEvent* e);
void motionnotify(XEvent* e);
void raiseclient(Client* c);
void movemouse(Client* c);
void resizemouse(Client* c);
void maximize(Client* c);
void fullscreen(Client* c);
void unmanage(Client *c, int destroyed);
void setclientstate(Client *c, long state);
int sendevent(Client *c, Atom proto);
void killclient(Client* c);
void keypress(XEvent* e);
void focus(Client* c);
void expose(XEvent* e);
void net_support_init();
void constrain(Client *c);
void setnetwmstate(Client* c, Atom state);
void fullscreen(Client* c);
void netclientlist();
void wmnormalhints(Client *c);
void draw_watermark();
void grabkeys();
void ungrabkeys();


void wmnormalhints(Client* c)
{
    XSizeHints hints; long supplied;
    if(XGetWMNormalHints(display, c->win, &hints, &supplied))
    {
        c->basew = c->baseh = c->minw = c->minh = c->maxh = c->maxw = c->incw = c->inch = 0;
        if(hints.flags & PBaseSize)
        {
            c->basew = hints.base_width;
            c->baseh = hints.base_height;
        }
        if(hints.flags & PMinSize)
        {
            c->minw = hints.min_width;
            c->minh = hints.min_height;
        }
        if (hints.flags & PMaxSize)
        {
            c->maxw = hints.max_width;
            c->maxh = hints.max_height;
        }
        if (hints.flags & PResizeInc)
        {
            c->incw = hints.width_inc;
            c->inch = hints.height_inc;
        }
        if (hints.flags & (USPosition | PPosition))
        {
            c->x = hints.x;
            c->y = hints.y;
        }
        c->hintsvalid = 1;
    } else {c->hintsvalid = 0;}
}



void netclientlist()
{
    Window* w = malloc(sizeof(Window)*ccount); int i = 0; Client* c;
    for(c = clients; c; c=c->next){w[i++] = c->win;}
    XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeReplace,(unsigned char*)w, ccount);   
}

void fullscreen(Client* c)
{
    if(!c)
        return;
    if(!c->fullscreen)
    {
        c->oldx = c->x;c->oldy = c->y;c->oldw = c->w;c->oldh = c->h;
        int sw = DisplayWidth(display, screen); int sh = DisplayHeight(display, screen);
        c->x = -FRAME_BORDER_WIDTH;
        c->y = -FRAME_BORDER_WIDTH;
        c->w = sw;
        c->h = sh;
        configure(c);
        XMoveResizeWindow(display, c->frame, c->x, c->y, c->w, c->h);
        XResizeWindow(display, c->win, sw, sh - FRAME_HEIGHT);
        //draw_name(c);
        c->fullscreen = 1;
        setnetwmstate(c, netatom[NetWMFullscreen]);
        raiseclient(c);
    } 
    else if(c->fullscreen)
    {
        c->x = c->oldx;c->y = c->oldy;c->w = c->oldw;c->h = c->oldh;
        configure(c);
        XMoveResizeWindow(display, c->frame, c->x, c->y, c->w, c->h);
        XResizeWindow(display, c->win, c->w - FRAME_BORDER_WIDTH, c->h - FRAME_HEIGHT);
        draw_name(c);
        draw_watermark();
        c->fullscreen = 0;   
        setnetwmstate(c, 0);
        raiseclient(c);
    }

}

void net_support_init()
{
    Atom supported[] = {        
        netatom[NetActiveWindow],
        netatom[NetWMName],
        netatom[NetClientList],
        netatom[NetWMState],
        netatom[NetWMFullscreen],
        netatom[NetWMWindowType],
        netatom[NetWMWindowTypeDialog],
        netatom[NetLast]};
    
    XChangeProperty(display, root, netatom[NetSupported], XA_ATOM, 32,
         PropModeReplace, (unsigned char *)supported, sizeof(supported) / sizeof(Atom));
}

void expose(XEvent* e)
{
    if(!e) 
        return;
    draw_watermark();
    XExposeEvent *ev = &e->xexpose;
    Client* c = find_client(ev->window);
    if(!c) return;
    draw_name(c);
    
}

Monitor* getmon(int x, int y) {
    for (Monitor* m = monitors; m; m = m->next) {
        if (x >= m->mx && x < (m->mx + m->mw) &&
            y >= m->my && y < (m->my + m->mh)) {
            return m;
        }
    }
    return monitors;
}

void setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(display, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

void setnetwmstate(Client* c, Atom state) {
    Atom data[] = { state };
    XChangeProperty(display, c->win, netatom[NetWMState], XA_ATOM, 32,PropModeReplace, (unsigned char *)data, 1);
}

int sendevent(Client *c, Atom proto)
{
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;

	if (XGetWMProtocols(display, c->win, &protocols, &n)) {
		while (!exists && n--)
			exists = protocols[n] == proto;
		if(protocols) 
            XFree(protocols);
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(display, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void killclient(Client* c)
{
    if(!sendevent(c, wmatom[WMDelete]))
    {
        XGrabServer(display);
        XSetCloseDownMode(display, DestroyAll);
        XKillClient(display, c->win);
        XSync(display, False);
        XUngrabServer(display);
    }
    if(c->frame != None)
        XDestroyWindow(display, c->frame);
}

void minimize(Client* c)
{
    if(!c)
        return;
    XGrabServer(display);
    if(!c->minimized)
    {
        c->minimized = True;
        XUnmapWindow(display, c->frame);
        setclientstate(c, IconicState);
        setnetwmstate(c, netatom[NetWMStateHidden]);
    }
    else if(c->minimized)
    {
        c->minimized = False;
        XMapWindow(display, c->frame);
        setclientstate(c, NormalState);
        setnetwmstate(c, 0);
    }
    XUngrabServer(display);
}

void unmanage(Client* c, int destroyed)
{
    XWindowChanges wc;
    if(!destroyed)
    {
        wc.border_width = c->oldbw;
        XGrabServer(display);
        XSelectInput(display, c->win, NoEventMask);
		XConfigureWindow(display, c->win, CWBorderWidth, &wc); 
		XUngrabButton(display, AnyButton, AnyModifier, c->win);
        XReparentWindow(display,c->win, root, c->x, c->y);
        setclientstate(c, WithdrawnState);
        XDestroyWindow(display, c->frame);
        XUnmapWindow(display, c->win);
        XSync(display, False);
        XUngrabServer(display);
    }
    remove_client(c->win);
    if(lastfocused = c)
        lastfocused = NULL;
    netclientlist();
}



int errorhandler(Display* display, XErrorEvent *error) {
    char error_text[1024];
    XGetErrorText(display, error->error_code, error_text, sizeof(error_text));
    fprintf(stderr, "Xlib Error: %s\n", error_text);
}

void resizemouse(Client* c)
{
    if(!c || !c->frame || c->fullscreen) return;
    int x, y, ox, oy, ow, oh, nw, nh;
    XEvent ev; 
    ox = c->x; oy = c->y; ow = c->w; oh = c->h; nh = oh; nw = ow; XDefineCursor(display, c->frame, res);
    if(XGrabPointer(display, root,False, PointerMotionMask | ButtonReleaseMask, GrabModeAsync,
                    GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
                    {return;}
    XQueryPointer(display, root, &(ev.xbutton.root), &(ev.xbutton.subwindow),
                  &x, &y, &(ev.xbutton.x), &(ev.xbutton.y), &(ev.xbutton.state));
    
    while(True) {
        XMaskEvent(display, PointerMotionMask | ButtonReleaseMask, &ev);
        if(ev.type == MotionNotify){
            
            nw = ow + (ev.xmotion.x - x);
            nh = oh + (ev.xmotion.y - y);
            if(c->hintsvalid)
            {
                nw = (nw < c->minw) ? c->minw : nw;
                nh = (nh < c->minh) ? c->minh : nh;

                if (c->maxw > 0) nw = (nw > c->maxw) ? c->maxw : nw;
                if (c->maxh > 0) nh = (nh > c->maxh) ? c->maxh : nh;
                
                if (c->incw > 0) nw -= (nw - c->basew) % c->incw;
                if (c->inch > 0) nh -= (nh - c->baseh) % c->inch;
                
                #ifdef DEBUG
                    printf("nw: %d\n,nh: %d\n,ow: %d\n,oh: %d\n,x: %d\n,y: %d\n", nw, nh, ow, oh, x, y);
                #endif
                nw = (nw < MIN_SIZE) ? MIN_SIZE : nw;
                nh = (nh < MIN_SIZE) ? MIN_SIZE : nh;
            
                XResizeWindow(display, c->frame, nw, nh + FRAME_HEIGHT);
                
            }
        }
        if(ev.type == ButtonRelease)
        {   
            nw = (nw < MIN_SIZE) ? MIN_SIZE : nw;
            nh = (nh < MIN_SIZE) ? MIN_SIZE : nh;
            XResizeWindow(display, c->win, nw, nh); c->w = nw; c->h = nh;
            draw_name(c);
            draw_watermark();
            break;
        }
    }
    XUngrabPointer(display, CurrentTime);
    XDefineCursor(display, c->frame, normal);
    configure(c);
}

void movemouse(Client* c)
{
    if(!c->win || !c->frame || c->fullscreen)
        return;
    int x, y, ox, oy, nx, ny, nw, nhm, origx, origy, origw, origh, minw, minh, maxw, maxh;
    XEvent ev; 
    ox = c->x; oy = c->y; 
    XDefineCursor(display, c->frame, mov);
    if(XGrabPointer(display, root,False, PointerMotionMask | ButtonReleaseMask, GrabModeAsync,
                    GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
                    {return;}
    XQueryPointer(display, root, &(ev.xbutton.root), &(ev.xbutton.subwindow),
                  &x, &y, &(ev.xbutton.x), &(ev.xbutton.y), &(ev.xbutton.state));
    while(True) {
        XMaskEvent(display, PointerMotionMask | ButtonReleaseMask, &ev);
        if(ev.type == MotionNotify){
            nx = ox + (ev.xmotion.x - x); ny = oy + (ev.xmotion.y - y);
            if(nx < 0) nx = 0;
            if(ny < 0) ny = 0;
                XMoveWindow(display, c->frame, nx + c->mon->wx, ny+ c->mon->wy); c->x = nx; c->y = ny;
            
            }
        if(ev.type == ButtonRelease)
        {
            break;
        }
    }
    XUngrabPointer(display, CurrentTime);
    XDefineCursor(display, c->frame, normal);
}
void raiseclient(Client* c)
{
    if(!c)return;
    //if(c->frame == lastfocused->frame) return;
    if(!lastfocused) 
        lastfocused = c;
    XLowerWindow(display, lastfocused->frame);
    XSetWindowBorder(display, lastfocused->frame, FRAME_INACTIVE_BORDER_COLOR);
    lastfocused = c;
    XSetWindowBorder(display, lastfocused->frame, FRAME_BORDER_COLOR);
    
    XRaiseWindow(display, c->frame);
    focus(c);
    #ifdef DEBUG
        printf("raised a window!\n");
    #endif
}

void focus(Client* c)
{
    if(!c->neverfocus)
    {
        XSetInputFocus(display, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(display, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);

    }
    sendevent(c, wmatom[WMTakeFocus]);

}

int gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(display, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(display, &name, &list, &n) >= Success && n > 0 && *list) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = display;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(display, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

static void update_monitors()
{
    XRRScreenResources *res = XRRGetScreenResources(display, root); int moncount = 0, i;
    for(i = 0; i < res->ncrtc; ++i)
    {
        XRRCrtcInfo *crtc = XRRGetCrtcInfo(display, res, res->crtcs[i]);
        if(crtc->width == 0 || crtc->height == 0) 
            continue;
        Monitor *m = calloc(1, sizeof(Monitor));
        m->mx = m->wx = crtc->x;
        m->my = m->wy = crtc->y;
        m->mw = m->ww = crtc->width;
        m->mh = m->wh = crtc->height;
        m->num = moncount++;
        m->next = NULL;
        if(!monitors) {monitors = m;} else {Monitor *last = monitors; while(last->next){last = last->next;} last->next = m;}
        XRRFreeCrtcInfo(crtc);
    }
    XRRFreeScreenResources(res);
    #ifdef DEBUG
        printf("Monitor count: %i\n", moncount);
    #endif
}

Client* find_client(Window w)
{
    Client *c;
    for(c = clients; c; c = c->next)
    {
        if(c->win == w || c->frame == w)
            return c;
    }
    return NULL;
}

void remove_client(Window w) {
    Client **client_ref = &clients; 

    while (*client_ref) {
        if ((*client_ref)->win == w) {
            Client *temp = *client_ref;
            *client_ref = (*client_ref)->next; 
            free(temp);
            #ifdef DEBUG
                puts("removing client");
            #endif
            ccount--;
            return;
        }
        client_ref = &(*client_ref)->next; 
    }
    #ifdef DEBUG
        puts("Client not found.");
    #endif
    return; 
}

void maprequest(XEvent* e)
{
    XMapRequestEvent *ev = &e->xmaprequest;
    XWindowAttributes xwa;
    if(!(XGetWindowAttributes(display, ev->window, &xwa)) || xwa.override_redirect)
        return;
    if(!find_client(ev->window))
        manage(ev->window);
}

void buttonpress(XEvent* e)
{
    XButtonEvent *ev = &e->xbutton;
    if((ev->state & ControlMask) && ev->button == Button1 && ev->window == root) {
        XCloseDisplay(display);
        system("pkill -u $USER"); //Hack, TO-DO: FIX 
        exit(0);
    }
    else if((ev->state & ShiftMask) && ev->button == Button1 && ev->window == root) {
        launch(LAUNCHER);
    }
    Client* c = find_client(ev->window);
    if(!c) {
        //Unsure how this might affect operation, so it's commented out to be safe
        //This is intended to give Transient (therefore unmanaged) Windows Input Focus in the case
        //they lose it. Use at your own "risk" I guess...
        //if(ev->window){XSetInputFocus(display, ev->window, RevertToPointerRoot, CurrentTime);}
        #ifdef DEBUG 
            printf("buttonpress - invalid client - returning");
        #endif
        return;
    }
       
    if(ev->button == Button1)
    {
        raiseclient(c);
        if((ev->state & Mod1Mask))
        {
            raiseclient(c);
            movemouse(c);
            
        } 
        else if(ev->state & ControlMask)
        {
            killclient(c);
            if(c->next)
                raiseclient(c->next);
            unmanage(c, True);
        } 


    }
    if(ev->button == Button2)
    {
        if((ev->state & Mod1Mask))
        {
            minimize(c);
        } 
        else if(ev->state & ControlMask)
        {
            maximize(c);
            raiseclient(c);
        }
    }
    if(ev->button == Button3)
    {
        if((ev->state & Mod1Mask))
        {
            raiseclient(c);
            resizemouse(c);
            
        } 
        else if(ev->state & ControlMask)
        {
            fullscreen(c);
        }
    }
}

void updatename(Client* c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, "Broken");
}

void manage(Window w)
{
    if(find_client(w))
        return;
    #ifdef DEBUG
        printf("we managin'\n");
    #endif
    Atom type;
    int format; 
    unsigned long nitems, bytes_after;
    Atom* wintype = NULL;
    XWindowAttributes xwa;
    XGetWindowProperty(display, w, netatom[NetWMWindowType], 0, 1024, False, XA_ATOM, &type, &format, 
                        &nitems, &bytes_after, (unsigned char**)&wintype);
    Client* c = calloc(1, sizeof(Client)); 
    if(!c) return;
    c->win = w; 
    wmnormalhints(c);

    /*The handling of transient windows is a bit weird in this WM, I must admit. 
    I've noticed that some windows that are transient, should absolutely not be managed,
    while others are fine being managed, from a usability standpoint.
    
    Think of a window serving as a magnifying glass in an image viewer vs. an "Open Folder"
    Dialog. I've found it works better if I dont manage them at all. This works fine, but if
    you know of a way to differentiate better, please let me know via a message, Pull Request,
    or anything else really*/
    Window transient_for;
    if (XGetTransientForHint(display, w, &transient_for)) {
        #ifdef DEBUG
            puts("Transient window");
        #endif
        XWindowAttributes xwa;
        XGetWindowAttributes(display, w, &xwa);
        XSizeHints hints;
        long supplied_return;
        XGetWMNormalHints(display, w, &hints, &supplied_return);
        int wh = hints.width > 0 ? hints.width : xwa.width;
        int h = hints.height > 0 ? hints.height : xwa.height;
        XWindowAttributes parent_xwa;
        XGetWindowAttributes(display, transient_for, &parent_xwa);
        int nx = parent_xwa.x + (parent_xwa.width - wh) / 2;
        int ny = parent_xwa.y + (parent_xwa.height - h) / 2;
        if(nx < 0) nx = 0; if(ny < 0) ny = 0;
        XResizeWindow(display, w, wh, h);
        XMoveWindow(display, w, nx, ny);
        XMapWindow(display, w);
        XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);
        return;
    }

    if (wintype) {
        if (*wintype == netatom[NetWMWindowTypeDialog])
            c->is_dialog = 1;
        else if (*wintype == netatom[NetWMWindowTypeNormal])
            c->is_normal = 1;
        else if (*wintype == netatom[NetWMWindowTypeToolbar])
            c->is_toolbar = 1;
        else if (*wintype == netatom[NetWMWindowTypeDock] ||
                 *wintype == netatom[NetWMWindowTypeDialog] || 
                 *wintype == netatom[NetWMWindowTypeSplash] || 
                 *wintype == netatom[NetWMWindowTypeDesktop]) {
            XMapWindow(display, c->win);
            return;
        }
        else
            c->is_normal = 1;

        XFree(wintype);
    }

    XGetWindowAttributes(display, c->win, &xwa);
    c->x = xwa.x; c->y = xwa.y; c->w = xwa.width; c->h = xwa.height; c->oldbw = xwa.border_width;
    c->maximized = False; c->minimized = False; c->fullscreen = False;
    c->mon = getmon(c->x, c->y);

    updatename(c);
    if(c->w < MIN_SIZE) c->w = 2*MIN_SIZE;
    if(c->h < MIN_SIZE) c->h = 2*MIN_SIZE;
    if(c->y < wm_y) c->y = wm_y;
    XSetWindowBorderWidth(display, c->win, 0);
    XSelectInput(display, c->win,
         EnterWindowMask|FocusChangeMask|PropertyChangeMask|
         StructureNotifyMask|ButtonPressMask| KeyPressMask|
         PointerMotionMask);
    
    XSetWindowAttributes fwa; 
    fwa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
    |ButtonPressMask|PointerMotionMask|EnterWindowMask|KeyPressMask
    |LeaveWindowMask|StructureNotifyMask|PropertyChangeMask|ExposureMask;
    c->frame = XCreateSimpleWindow(display,
        root, c->x + c->mon->wx, c->y + c->mon->wy, c->w,
        (c->h + FRAME_HEIGHT), FRAME_BORDER_WIDTH, FRAME_BORDER_COLOR,
        FRAME_BACKGROUND_COLOR);
    
    XReparentWindow(display, c->win, c->frame,
                    0, FRAME_HEIGHT);
    XSelectInput(display, c->frame, fwa.event_mask);

    XMapWindow(display, c->frame);
    XMapWindow(display, c->win);
    
    configure(c);
    draw_name(c);
    //TO-DO: Implement actual checking of this
    c->neverfocus = False;
    lastfocused = c;
    raiseclient(c);
    c->next = clients;
    clients = c;
    ccount++;
    netclientlist();
}

void maximize(Client* c)
{
    if(!c)
        return;
    if(!c->maximized)
    {
        c->oldx = c->x; c->oldy = c->y; c->oldw = c->w; c->oldh = c->h;
        c->x = 0; c->y = wm_y; c->w = DisplayWidth(display, screen); c->h = DisplayHeight(display, screen);
        configure(c);
        XMoveResizeWindow(display, c->frame, c->x, c->y, c->w, c->h);
        XResizeWindow(display, c->win, c->w - FRAME_BORDER_WIDTH, c->h - FRAME_HEIGHT - PANEL_HEIGHT - FRAME_BORDER_WIDTH);
        draw_name(c);
        draw_watermark();
        c->maximized = 1;
        raiseclient(c);
    } else if(c->maximized)
    {
        c-> x = c->oldx, c->y = c->oldy; c->w = c->oldw; c->h = c->oldh;
        configure(c);
        XMoveResizeWindow(display, c->frame, c->x, c->y, c->w, c->h);
        XResizeWindow(display, c->win, c->w - FRAME_BORDER_WIDTH, c->h - FRAME_HEIGHT);
        draw_name(c);
        draw_watermark();
        c->maximized = 0;   
        raiseclient(c);
    }
}

void keypress(XEvent* e)
{
    /*So you may wonder why this part is empty. I tried everything under the sun to make keypresses work.
    Using ALT+Q would be so much more clean than this concotion of MOD+MB things I have going on. Nontheless,
    I've kinda taken a liking to it. Nevertheless, if you think you can get those keybinds to work, you can try,
    but since I've experienced it myself, I recommend you do something useful with your time instead of losing your mind over this.
    
    Sincerely,
        -John Connection*/
}

void draw_name(Client* c) { if(!c) return; XClearWindow(display, c->frame); XDrawString(display, c->frame, DefaultGC(display, screen), 2, 15, c->name, strlen(c->name));}

void draw_watermark()
{
    XClearWindow(display, root);
    char* watermark = VERSION;
    char* link = "https://github.com/JohnConnection/jcwm";
    char* ctl1 = "ALT+LMB = MOVE WINDOW";
    char* ctl2 = "ALT+MMB = MINIMIZE";
    char* ctl3 = "ALT+RMB = RESIZE WINDOW";
    char* ctl4 = "CTRL+LMB = KILL WINDOW";
    char* ctl5 = "CTRL+MMB = MAXIMIZE WINDOW";
    char* ctl6 = "CTRL+RMB = FULLSCREEN WINDOW";
    char* ctl7 = "CTRL+LMB ON BACKGROUND = EXIT WM";
    char* ctl8 = "SHIFT+LMB ON BACKGROUND = LAUNCHER";
    GC gc = XCreateGC(display, root, 0, NULL);
    XSetForeground(display, gc, WATERMARK_CLR);
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 60, watermark, strlen(watermark));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 75, link, strlen(link));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 90, ctl1, strlen(ctl1));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 105, ctl2, strlen(ctl2));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 120, ctl3, strlen(ctl3));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 135, ctl4, strlen(ctl4));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 150, ctl5, strlen(ctl5));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 175, ctl6, strlen(ctl6));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 190, ctl7, strlen(ctl7));
    XDrawString(display, root, gc, (DisplayWidth(display, screen) - 350), 205, ctl8, strlen(ctl8));
}

void 
init(void)
{
    XSetWindowAttributes attr;
    Atom utf8string;
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    update_monitors();
    normal = XCreateFontCursor(display, NORMAL);
    mov = XCreateFontCursor(display, MOV);
    res = XCreateFontCursor(display, RES);
    del = XCreateFontCursor(display, DEL);
    utf8string = XInternAtom(display, "UTF8_STRING", False); 
	wmatom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(display, "WM_DELETE_WINDOW", False); 
	wmatom[WMState] = XInternAtom(display, "WM_STATE", False); 
	wmatom[WMTakeFocus] = XInternAtom(display, "WM_TAKE_FOCUS", False); 
	netatom[NetActiveWindow] = XInternAtom(display, "_NET_ACTIVE_WINDOW", False); 
	netatom[NetSupported] = XInternAtom(display, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(display, "_NET_WM_NAME", False); 
	netatom[NetWMState] = XInternAtom(display, "_NET_WM_STATE", False); 
	netatom[NetWMCheck] = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(display, "_NET_CLIENT_LIST", False); 
    netatom[NetWMWindowTypeNormal] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    netatom[NetWMWindowTypeToolbar] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
    netatom[NetWMWindowTypeSplash] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    netatom[NetWMWindowTypeUtility] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
    netatom[NetWMWindowTypeDock] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    netatom[NetWMWindowTypeDesktop] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    netatom[NetWMStateHidden] = XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);
    
    
    attr.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
    |ButtonPressMask|PointerMotionMask|EnterWindowMask
    |LeaveWindowMask|StructureNotifyMask|PropertyChangeMask|ExposureMask;
    XChangeWindowAttributes(display, root, CWEventMask|CWCursor, &attr);
    XSelectInput(display, root, attr.event_mask);
    XDefineCursor(display, root, normal);
    XSetErrorHandler(errorhandler);
    net_support_init();
    draw_watermark();
    XUngrabButton(display, AnyButton, AnyModifier, root);
    wm_y = 0 + PANEL_HEIGHT;
}   

void destroynotify(XEvent* e)
{
    #ifdef DEBUG
        puts("destroy notify event received!");
    #endif
    XDestroyWindowEvent *ev = &e->xdestroywindow;
    Client* c = find_client(ev->window);
    if (c) {
        XWindowAttributes wa;
        Status status = XGetWindowAttributes(display, c->frame, &wa);
        if (status == 0) {
            remove_client(c->win);
            if (c == lastfocused)
                lastfocused = NULL;
            netclientlist();
            return;
        }
        XUnmapWindow(display, c->frame);
        XDestroyWindow(display, c->frame);
        
        
        remove_client(c->win);
        if (c == lastfocused)
            lastfocused = NULL;
        netclientlist();
    } else {
        XSetInputFocus(display, root, RevertToParent, CurrentTime);
        lastfocused = NULL;
    }
}


void propertynotify(XEvent *e)
{
    XPropertyEvent *ev = &e->xproperty;
    Client* c = find_client(ev->window);
    if(c && (ev->atom == netatom[NetWMName]))
    {
        updatename(c);
        draw_name(c);
    }
}

void clientmessage(XEvent* e)
{
    #ifdef DEBUG
        puts("client message event received!");
    #endif
    XClientMessageEvent *ev = &e->xclient;
    if(!e || !ev)
        return;
    if(ev->message_type == netatom[NetActiveWindow])
    {
        Client* c = find_client(ev->window);
        raiseclient(c);
        if(c && c->minimized) {
            puts("cm2");
            minimize(c);
        }        
    }
}

void 
run(void)
{

    XEvent ev;
    for(;;)
    {
        XNextEvent(display, &ev);
        switch(ev.type)
        {
            case MapRequest:
                maprequest(&ev);
                break;
            case ButtonPress:
                buttonpress(&ev);
                break;
            case KeyPress:
                keypress(&ev);
                break;
            case Expose:
                expose(&ev);
                break;
            case DestroyNotify:
                destroynotify(&ev);
                break;
            case PropertyNotify:
                propertynotify(&ev);
                break;
            case ClientMessage:
                clientmessage(&ev);
                break;
        }
    }
}

int
main(int argc, char** argv)
{
    signal(SIGCHLD, SIG_IGN);
    init();
    launch("jcbar");
    launch("alttab");
    run();
    XCloseDisplay(display);
    return 0;
}
