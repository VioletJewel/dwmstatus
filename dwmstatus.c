#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <X11/Xlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <locale.h>
#include <libintl.h>
#include <langinfo.h>

#define TFMT_L 27
#define STATUS_L 64
#define TITLE_L 64
#define D_FMT_L 32

#define LOCALE_DIR ".local/share/locales"

static volatile int keepalive = 1;

static Display *dpy;
static Window root;

static FILE *f_enow, *f_efull, *f_estatus;

static struct timespec ts;
static struct timespec tn;

static char datefmt[D_FMT_L];

static void signalhandler(int); // handle system signals

static void refreshstatus(time_t); // refresh status

static void signalhandler(int num) {
  if (num == SIGUSR1) {
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    refreshstatus(ts.tv_sec + 3);
  }
}

static void inline refreshstatus(time_t sec) {
  static struct tm *tm;
  static char dt[TFMT_L + 1], status[STATUS_L],
    title[TITLE_L + 1], *status_tr;
  static int enow, efull;
  static double pct;

  fseek(f_enow, 0, SEEK_SET); fscanf(f_enow, "%d", &enow);
  fseek(f_efull, 0, SEEK_SET); fscanf(f_efull, "%d", &efull);
  fseek(f_estatus, 0, SEEK_SET); fscanf(f_estatus, "%s", status);
  status_tr = gettext(status);

  pct = 100.0 * enow / efull;
  tm = localtime(&sec);
  strftime(dt, TFMT_L, datefmt, tm);

  snprintf(title, TITLE_L, "%.2f%% (%s)  %s", pct, status_tr, dt);
  XStoreName(dpy, root, title);
  XFlush(dpy);
}

int main(void) {
  int scr;

  char *nl;
  unsigned nli, nll, di;
  char pflag, // percent flag
       cflag, // colon flag
       fchar; // format char

  unsigned long int s;
  unsigned long int rns;

  time_t sec;

  char localedir[64];

  struct passwd *pw;
  size_t pwlen;

  pw = getpwuid(getuid());
  pwlen = strlen(pw->pw_dir);
  memcpy(localedir, pw->pw_dir, pwlen);
  localedir[pwlen] = '/';
  memcpy(localedir + pwlen + 1, LOCALE_DIR, sizeof(LOCALE_DIR));

  setlocale(LC_ALL, "");
  bindtextdomain("dwmstatus", localedir);
  textdomain("dwmstatus");

  nl = nl_langinfo(D_T_FMT);
  nll = strlen(nl);
  for (nli = 0, di = 0, pflag = 0, cflag = 0, fchar = 0; nli < nll && nli < D_FMT_L; nli++) {
    char nlc = nl[nli];
    if (pflag == '%') {
      if (nlc == '_' || nlc == '-' || nlc == '0' || nlc == '+' || nlc == '^' || nlc == '#') {
        fchar = nlc;
        continue;
      }
      if (nlc == 'T') { // convert %T to %H:%M
        if (cflag == ':')
          datefmt[di++] = ':'; // idk who would set date_fmt like this, but ...
        else
          datefmt[di++] = ' '; // (double) space before %H:%M
        datefmt[di++] = '%'; // add %H:%M
        datefmt[di++] = 'H';
        datefmt[di++] = ':';
        datefmt[di++] = '%';
        datefmt[di++] = 'M';
      } else if (nlc != 'S') {
        if (cflag == ':')
          datefmt[di++] = ':';
        else if (nlc == 'H')
          datefmt[di++] = ' '; // space before %H
        datefmt[di++] = '%';
        if (nlc == 'e')
          datefmt[di++] = '-'; // remove padding from %e
        else if (fchar != 0)
          datefmt[di++] = fchar; // add format char back
        datefmt[di++] = nlc;
      }
      fchar = 0;
      pflag = 0;
      cflag = 0;
    } else if (nlc == '%') {
      pflag = '%';
    } else if (nlc == ':') {
      cflag = ':';
    } else {
      if (cflag == ':') {
        datefmt[di++] = ':';
        cflag = 0;
      }
      datefmt[di++] = nlc;
    }
  }

  signal(SIGUSR1, signalhandler);

  f_enow = fopen("/sys/class/power_supply/BAT0/energy_now", "r");
  setvbuf(f_enow, NULL, _IONBF, 0);

  f_efull = fopen("/sys/class/power_supply/BAT0/energy_full", "r");
  setvbuf(f_efull, NULL, _IONBF, 0);

  f_estatus = fopen("/sys/class/power_supply/BAT0/status", "r");
  setvbuf(f_estatus, NULL, _IONBF, 0);

  dpy = XOpenDisplay(NULL);
  if (!dpy) return 1;
  scr = DefaultScreen(dpy);
  root = RootWindow(dpy, scr);

  if (clock_gettime(CLOCK_REALTIME_COARSE, &ts)) return 2;
  s = ts.tv_sec % 60;
  rns = (s > 56 ? 120U - s : 60U - s) * 1000000000U - (unsigned)ts.tv_nsec;
  sec = ts.tv_sec;
  refreshstatus(sec);
  tn.tv_sec = rns / 1000000000UL;
  tn.tv_nsec = rns % 1000000000UL;
  nanosleep(&tn, NULL);
  while (keepalive) {
    if (clock_gettime(CLOCK_REALTIME_COARSE, &ts)) return 2;
    s = ts.tv_sec % 60;
    rns = (s > 56 ? 120U - s : 60U - s) * 1000000000U - (unsigned)ts.tv_nsec;
    sec = ts.tv_sec + 3;
    refreshstatus(sec);
    tn.tv_sec = rns / 1000000000UL;
    tn.tv_nsec = rns % 1000000000UL;
    nanosleep(&tn, NULL);
  }
  return 0;
}
