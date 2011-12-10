/* xevent.h */

extern BOOLEAN x_repeating;
extern BOOLEAN x_didcmd;

void x_ev_repeat P_((XEvent *event, long timeout));
XEvent *x_ev_wait P_((void));
void x_ev_process P_((XEvent *event));
BOOLEAN x_ev_poll P_((BOOLEAN reset));
