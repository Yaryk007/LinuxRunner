/* LinuxRunner fork: after the X screen changed size (rotation with the Native
 * resolution), resize top-level windows that covered the old screen (nested
 * compositors such as Wayfire, Phosh/phoc or Plasma Mobile's KWin, and
 * fullscreen apps without a window manager) to the new size. GPLv3. */
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
static int ignore(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }
int main(int argc, char **argv) {
    if (argc != 5) { fprintf(stderr, "usage: fit-windows old_w old_h new_w new_h\n"); return 2; }
    int ow = atoi(argv[1]), oh = atoi(argv[2]), nw = atoi(argv[3]), nh = atoi(argv[4]);
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;
    XSetErrorHandler(ignore);
    Window root = DefaultRootWindow(d), r, p, *children = NULL;
    unsigned count = 0, fitted = 0;
    if (XQueryTree(d, root, &r, &p, &children, &count)) {
        for (unsigned i = 0; i < count; i++) {
            XWindowAttributes a;
            if (!XGetWindowAttributes(d, children[i], &a) || a.map_state != IsViewable || a.class != InputOutput)
                continue;
            /* Covered (nearly) the whole old screen: follow the screen. */
            if (a.x <= 0 && a.y <= 0 && a.width >= ow * 9 / 10 && a.height >= oh * 9 / 10) {
                XMoveResizeWindow(d, children[i], 0, 0, (unsigned) nw, (unsigned) nh);
                fitted++;
            }
        }
        if (children) XFree(children);
    }
    XSync(d, False);
    printf("FIT: %u window(s) -> %dx%d\n", fitted, nw, nh);
    XCloseDisplay(d);
    return 0;
}
