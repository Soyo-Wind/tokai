/* TinyWM is written by Nick Welch <nick@incise.org> in 2005 & 2011.
 * tokai is forked by Soyo-Wind in 2026.
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MOD Mod1Mask
#define MAX_KEYS 16

typedef struct {
    KeySym keysym;
    char cmd[256];
} KeyBind;

KeyBind keys[MAX_KEYS];
int keycount = 0;

void loadrc(Display *dpy, Window root) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.tokairc", getenv("HOME"));

    FILE *f = fopen(path, "r");
    if(!f) return;

    char key[32];
    char cmd[256];

    while(fscanf(f, "%31s %255[^\n]\n", key, cmd) == 2) {
        if(keycount >= MAX_KEYS) break;

        KeySym ks = XStringToKeysym(key);
        if(ks == NoSymbol) continue;

        keys[keycount].keysym = ks;
        strcpy(keys[keycount].cmd, cmd);

        KeyCode kc = XKeysymToKeycode(dpy, ks);
        XGrabKey(dpy, kc, MOD, root, True, GrabModeAsync, GrabModeAsync);

        keycount++;
    }

    fclose(f);
}

void run_cmd(const char *cmd) {
    if(fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(0);
    }
}

int main(void)
{
    Display * dpy;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;
    Window root;

    if(!(dpy = XOpenDisplay(0x0))) return 1;

    root = DefaultRootWindow(dpy);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("g")), MOD,
            root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("h")), MOD,
            root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(dpy, 1, MOD, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MOD, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    loadrc(dpy, root);

    start.subwindow = None;
    while(1)
    {
        XNextEvent(dpy, &ev);
        if(ev.type == KeyPress && ev.xkey.subwindow != None)
        {
            KeySym ks = XKeycodeToKeysym(dpy, ev.xkey.keycode, 0);
            if(ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("g")))
                XRaiseWindow(dpy, ev.xkey.subwindow);
            else if(ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("h")))
                XLowerWindow(dpy, ev.xkey.subwindow);
            else {
                for(int i=0;i<keycount;i++) {
                    if(keys[i].keysym == ks) {
                        run_cmd(keys[i].cmd);
                    }
                }
            }
        }
        else if(ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        else if(ev.type == MotionNotify && start.subwindow != None)
        {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, start.subwindow,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        }
        else if(ev.type == ButtonRelease)
            start.subwindow = None;
    }
}
