#include <stdio.h>
#include <string.h>

#include "../util.h"
#include "battery.h"

#define ICONe COL2 "" COL0 /* unexpected error */
#define ICONa COL1 "" COL0 /* no battery */

#define PDN COL1
#define PUP COL2

#define ICON0 "" COL0
#define ICON1 "" COL0
#define ICON2 "" COL0
#define ICON3 "" COL0
#define ICON4 "" COL0
#define ICON5 "" COL0
#define ICON6 "" COL0
#define ICON7 "" COL0
#define ICON8 "" COL0
#define ICON9 "" COL0

#define ICON(bat) icons[(bat * (LENGTH(icons) - 1) + 50) / 100]

#define BATC 10 /* critical level */
#define BATL 20 /* low level */
#define BATP 40 /* plug in level */
#define BATU 80 /* unplug level */

#define BATCAPFILE "/sys/class/power_supply/BAT0/capacity"
#define ACSTATEFILE "/sys/class/power_supply/ADP0/online"
#define BATCFULLFILE "/sys/class/power_supply/BAT0/energy_full"
#define BATCNOWFILE "/sys/class/power_supply/BAT0/energy_now"
#define BATRATEFILE "/sys/class/power_supply/BAT0/power_now"

#define CNOTIFY(t, msg)                                                        \
  (char *[]) {                                                                 \
    "/usr/bin/notify-send", "-h",                                              \
        "string:x-canonical-private-synchronous:batmon", "-u", "critical",     \
        "-t", t, "BatMon", msg, NULL                                           \
  }

#define NNOTIFY(t, msg)                                                        \
  (char *[]) {                                                                 \
    "/usr/bin/notify-send", "-h",                                              \
        "string:x-canonical-private-synchronous:batmon", "-t", t, "BatMon",    \
        msg, NULL                                                              \
  }

#define TNOTIFY(t, msg)                                                        \
  (char *[]) {                                                                 \
    "/usr/bin/notify-send", "-h",                                              \
        "string:x-canonical-private-synchronous:batmon", "-h",                 \
        "int:transient:1", "-t", t, "BatMon", msg, NULL                        \
  }

#define UCNOTIFY(t, msg) uspawn(CNOTIFY(t, msg))
#define UNNOTIFY(t, msg) uspawn(NNOTIFY(t, msg))
#define UTNOTIFY(t, msg) uspawn(TNOTIFY(t, msg))
#define CCNOTIFY(t, msg) cspawn(CNOTIFY(t, msg))
#define CNNOTIFY(t, msg) cspawn(NNOTIFY(t, msg))
#define CTNOTIFY(t, msg) cspawn(TNOTIFY(t, msg))

enum { Normal, Critical, Low, Plug, Unplug };

void batteryu(char *str, int ac) {
  int bat;
  if (!readint(BATCAPFILE, &bat)) {
    return;
  }
  snprintf(str, BLOCKLENGTH, "BAT: %d%%", bat);
}

void batteryc(int button) {
  int ac;
  int cur, rate;
  int hr, mn;

  if (!readint(ACSTATEFILE, &ac)) {
    CCNOTIFY("0", "Error: couldn't read " ACSTATEFILE);
    return;
  }
  if (ac) {
    int cnow;

    if (!readint(BATCFULLFILE, &cur)) {
      CNNOTIFY("1000", "On AC power");
      return;
    }
    if (!readint(BATCNOWFILE, &cnow)) {
      CNNOTIFY("1000", "On AC power");
      return;
    }
    cur -= cnow;
    if (!readint(BATRATEFILE, &rate)) {
      CNNOTIFY("1000", "On AC power");
      return;
    }
  } else {
    if (!readint(BATCNOWFILE, &cur)) {
      CCNOTIFY("0", "Error: couldn't read " BATCNOWFILE);
      return;
    }
    if (!readint(BATRATEFILE, &rate)) {
      CCNOTIFY("0", "Error: couldn't read " BATRATEFILE);
      return;
    }
  }
  if (!rate) {
    CNNOTIFY("2000", "Battery fully charged");
    return;
  }
  hr = cur / rate;
  mn = ((cur * 60) / rate) - hr * 60;
  switch (hr) {
  case 0:
    switch (mn) {
    case 0:
      CNNOTIFY("2000", "Battery fully charged");
      break;
    case 1:
      CNNOTIFY("2000", "1 minute remaining");
      break;
    default: {
      char buf[21];

      snprintf(buf, sizeof buf, "%d minutes remaining", mn);
      CNNOTIFY("2000", buf);
    } break;
    }
    break;
  case 1:
    switch (mn) {
    case 0:
      CNNOTIFY("2000", "1 hour remaining");
      break;
    case 1:
      CNNOTIFY("2000", "1 hour, 1 minute remaining");
      break;
    default: {
      char buf[29];

      snprintf(buf, sizeof buf, "1 hour, %d minutes remaining", mn);
      CNNOTIFY("2000", buf);
    } break;
    }
    break;
  default:
    switch (mn) {
    case 0: {
      char buf[19];

      snprintf(buf, sizeof buf, "%d hours remaining", hr);
      CNNOTIFY("2000", buf);
    } break;
    case 1: {
      char buf[29];

      snprintf(buf, sizeof buf, "%d hours, 1 minute remaining", hr);
      CNNOTIFY("2000", buf);
    } break;
    default: {
      char buf[31];

      snprintf(buf, sizeof buf, "%d hours, %d minutes remaining", hr, mn);
      CNNOTIFY("2000", buf);
    } break;
    }
    break;
  }
}
