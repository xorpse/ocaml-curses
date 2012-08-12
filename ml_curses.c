#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <stdio.h>
#include <unistd.h>

/* For PDCurses we need to define the following so that global
 * variables are declared (in the header) with __dllspec(dllimport).
 */
#ifdef PDCURSES
#define PDC_DLL_BUILD
#undef CURSES_LIBRARY
#endif

#ifdef CURSES_HEADER
#include CURSES_HEADER
#else
#include <ncurses.h>
#endif

#ifdef CURSES_TERM_H
#include CURSES_TERM_H
#else
#include <term.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

/* Du travail pour les esclaves de M$ */
#include <signal.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#define AWB(x) caml__dummy_##x=caml__dummy_##x; /* anti-warning bugware */

#define r_unit(f)	f; CAMLreturn(Val_unit);
#define r_window(f)	CAMLreturn((value)f)
#define r_terminal(f)	CAMLreturn((value)f)
#define r_err(f)	CAMLreturn(Val_bool((f)!=ERR))
#define r_int(f)	CAMLreturn(Val_int(f))
#define r_char(f)	CAMLreturn(Val_int((f)&255))
#define r_chtype(f)	CAMLreturn(Val_int(f))
#define r_attr_t(f)	CAMLreturn(Val_int(f))
#define r_bool(f)	CAMLreturn(Val_bool(f))
#define r_int_int(x,y)	\
  { CAMLlocal1(ret); AWB(ret); \
    ret=alloc_tuple(2); \
    Store_field(ret,0,Val_int(x)); \
    Store_field(ret,1,Val_int(y)); \
    CAMLreturn(ret); }
#define r_window_int(x,y)	\
  { CAMLlocal1(ret); AWB(ret); \
    ret=alloc_tuple(2); \
    Store_field(ret,0,(value)(x)); \
    Store_field(ret,1,Val_int(y)); \
    CAMLreturn(ret); }
#define r_int_int_int(x,y,z) \
  { CAMLlocal1(ret); AWB(ret); \
    ret=alloc_tuple(3); \
    Store_field(ret,0,Val_int(x)); \
    Store_field(ret,1,Val_int(y)); \
    Store_field(ret,2,Val_int(z)); \
    CAMLreturn(ret); }
#define r_string(f)	\
  { char *ret=f; \
    if(ret==NULL) failwith("Null pointer"); \
    CAMLreturn(copy_string(ret)); }

#define a_window(a)	((WINDOW * )a)
#define a_terminal(a)	((TERMINAL * )a)
#define a_screen(a)	((SCREEN * )Field(a,2))
#define a_int(a)	Int_val(a)
#define a_bool(a)	Bool_val(a)
#define a_chtype(a)	Int_val(a)
#define a_attr_t(a)	Int_val(a)
#define a_string(a)	String_val(a)

#define RA0 CAMLparam0();
#define RA1 CAMLparam1(aa); AWB(aa);
#define RA2 CAMLparam2(aa,ab); AWB(aa);
#define RA3 CAMLparam3(aa,ab,ac); AWB(aa);
#define RA4 CAMLparam4(aa,ab,ac,ad); AWB(aa);
#define RA5 CAMLparam5(aa,ab,ac,ad,ae); AWB(aa);
#define RA6 CAMLparam5(aa,ab,ac,ad,ae); CAMLxparam1(af); AWB(aa); AWB(af);
#define RA7 CAMLparam5(aa,ab,ac,ad,ae); CAMLxparam2(af,ag); AWB(aa); AWB(af);
#define RA8 CAMLparam5(aa,ab,ac,ad,ae); CAMLxparam3(af,ag,ah); AWB(aa); AWB(af);
#define RA9 CAMLparam5(aa,ab,ac,ad,ae); CAMLxparam4(af,ag,ah,ai); AWB(aa); AWB(af);

#define ML0(f,tr) \
  value mlcurses_##f(void) \
  { RA0 r_##tr(f()); }
#define ML1(f,tr,ta) \
  value mlcurses_##f(value aa) \
  { RA1 r_##tr(f(a_##ta(aa))); }
#define ML2(f,tr,ta,tb) \
  value mlcurses_##f(value aa,value ab) \
  { RA2 r_##tr(f(a_##ta(aa),a_##tb(ab))); }
#define ML3(f,tr,ta,tb,tc) \
  value mlcurses_##f(value aa,value ab,value ac) \
  { RA3 r_##tr(f(a_##ta(aa),a_##tb(ab),a_##tc(ac))); }
#define ML4(f,tr,ta,tb,tc,td) \
  value mlcurses_##f(value aa,value ab,value ac,value ad) \
  { RA4 r_##tr(f(a_##ta(aa),a_##tb(ab),a_##tc(ac),a_##td(ad))); }
#define ML5(f,tr,ta,tb,tc,td,te) \
  value mlcurses_##f(value aa,value ab,value ac,value ad,value ae) \
  { RA5 r_##tr(f(a_##ta(aa),a_##tb(ab),a_##tc(ac),a_##td(ad),a_##te(ae))); }

#define ML7(f,tr,ta,tb,tc,td,te,tf,tg) \
  value mlcurses_##f##_bytecode(value *a,int n) \
  { RA0 r_##tr(f(a_##ta(a[0]),a_##tb(a[1]),a_##tc(a[2]),a_##td(a[3]), \
  a_##te(a[4]),a_##tf(a[5]),a_##tg(a[6]))); } \
  value mlcurses_##f##_native(value aa,value ab,value ac,value ad, \
  value ae,value af,value ag) \
  { RA7 r_##tr(f(a_##ta(aa),a_##tb(ab),a_##tc(ac),a_##td(ad), \
  a_##te(ae),a_##tf(af),a_##tg(ag))); }

#define ML8(f,tr,ta,tb,tc,td,te,tf,tg,th) \
  value mlcurses_##f##_bytecode(value *a,int n) \
  { RA0 r_##tr(f(a_##ta(a[0]),a_##tb(a[1]),a_##tc(a[2]),a_##td(a[3]), \
  a_##te(a[4]),a_##tf(a[5]),a_##tg(a[6]),a_##th(a[7]))); } \
  value mlcurses_##f##_native(value aa,value ab,value ac,value ad, \
  value ae,value af,value ag,value ah) \
  { RA8 r_##tr(f(a_##ta(aa),a_##tb(ab),a_##tc(ac),a_##td(ad), \
    a_##te(ae),a_##tf(af),a_##tg(ag),a_##th(ah))); }

#define ML9(f,tr,ta,tb,tc,td,te,tf,tg,th,ti) \
  value mlcurses_##f##_bytecode(value *a,int n) \
  { RA0 r_##tr(f(a_##ta(a[0]),a_##tb(a[1]),a_##tc(a[2]),a_##td(a[3]),a_##te(a[4]), \
  a_##tf(a[5]),a_##tg(a[6]),a_##th(a[7]),a_##ti(a[8]))); } \
  value mlcurses_##f##_native(value aa,value ab,value ac,value ad,value ae, \
  value af,value ag,value ah,value ai) \
  { RA9 r_##tr(f(a_##ta(aa),a_##tb(ab),a_##tc(ac),a_##td(ad),a_##te(ae), \
    a_##tf(af),a_##tg(ag),a_##th(ah),a_##ti(ai))); }

#define ML0d(f,tr) value mlcurses_##f(void)
#define ML1d(f,tr,ta) value mlcurses_##f(value aa)
#define ML2d(f,tr,ta,tb) value mlcurses_##f(value aa,value ab)
#define ML3d(f,tr,ta,tb,tc) value mlcurses_##f(value aa,value ab,value ac)
#define ML4d(f,tr,ta,tb,tc,td) value mlcurses_##f(value aa,value ab,\
  value ac,value ad)
#define ML5d(f,tr,ta,tb,tc,td,te) value mlcurses_##f(value aa,value ab,\
  value ac,value ad,value ae)
#define ML6d(f,tr,ta,tb,tc,td,te,tf) value mlcurses_##f##_native(value,value,\
  value,value,value,value); \
  value mlcurses_##f##_bytecode(value *a,int n) \
  { return(mlcurses_##f##_native(a[0],a[1],a[2],a[3],a[4],a[5])); } \
  value mlcurses_##f##_native(value aa,value ab,value ac,value ad,value ae,value af)

#define BEG0 { RA0 {
#define BEG1 { RA1 {
#define BEG2 { RA2 {
#define BEG3 { RA3 {
#define BEG4 { RA4 {
#define BEG5 { RA5 {
#define BEG6 { RA6 {
#define BEG7 { RA7 {
#define BEG8 { RA8 {
#define BEG9 { RA9 {
#define END }}

/* RWMJ: Not implemented functions raise Invalid_argument
 * ("function_name").  This can happen for example when we are not
 * linked to real ncurses, particularly on Windows.
 */
#define ML0_notimpl(f,tr)					\
  value mlcurses_##f(void) BEG0 caml_invalid_argument (#f); CAMLnoreturn; END
#define ML1_notimpl(f,tr,ta)						\
  value mlcurses_##f(value aa) BEG1 caml_invalid_argument (#f); CAMLnoreturn; END
#define ML2_notimpl(f,tr,ta,tb)						\
  value mlcurses_##f(value aa, value ab) BEG2 caml_invalid_argument (#f); CAMLnoreturn; END

static WINDOW *ripoff_w[5];
static int ripoff_l[5];
static int ripoff_niv=0;
static int ripoff_callback(WINDOW *w,int l)
{
  if(ripoff_niv==5) return(0);
  ripoff_w[ripoff_niv]=w;
  ripoff_l[ripoff_niv]=l;
  ripoff_niv++;
  return(0);
}

value putc_function;
static int putc_callback(int c)
{
  CAMLparam0();
  CAMLlocal1(ret);

  AWB(ret);
  ret=callback_exn(putc_function,Val_int(c&255));
  CAMLreturn(Is_exception_result(ret)?-1:0);
}

#ifndef WIN32
/* Du travail pour les esclaves de M$ */
static void winch_handler(int n)
{
  signal(n,winch_handler);
  ungetch(KEY_RESIZE);
}
#endif

#include "functions.c"
#include "caml/signals.h"

/* The following routines are special-cased to allow other threads to run 
 * while getch() is blocking */
value mlcurses_getch(void)
{
   CAMLparam0();
   int ch;

   enter_blocking_section();
   ch = getch();
   leave_blocking_section();

   CAMLreturn(Val_int(ch));
}


value mlcurses_wgetch(value win)
{
   CAMLparam1(win);
   int ch;
   WINDOW* w;

   caml__dummy_win = caml__dummy_win;
   w = (WINDOW *) win;

   enter_blocking_section();
   ch = wgetch(w);
   leave_blocking_section();

   CAMLreturn(Val_int(ch));
}

