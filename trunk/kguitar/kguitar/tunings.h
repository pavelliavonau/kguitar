// TUNINGS.H - library of stringed instrument tunings

#include <klocale.h>

typedef struct {
    int strings;
    uchar shift[MAX_STRINGS];
    QString name;
} tuning;

// The library itself. Columns are:
//
// 1st - number of strings
// 2nd - array of tuning - midi note numbers (should be fairly easy to guess)
// 3rd - name of the tuning in the form "Instrument (strings): tuning"
//
// Various instruments should be separated by an empty line.

// Contributors:
//
// 04 Sep 2000 - Ukulele (4): standard - Alex Brand

tuning lib_tuning[] = {
    { 1, {},                        i18n("User defined") },

    { 6, {40,45,50,55,59,64},       i18n("Guitar") + " (6): E (standard)" },
    { 6, {39,44,49,54,58,63},       i18n("Guitar") + " (6): Eb" },
    { 6, {38,43,48,53,57,62},       i18n("Guitar") + " (6): D" },
    { 6, {38,45,50,55,59,64},       i18n("Guitar") + " (6): drop D" },
    { 6, {36,43,48,55,60,64},       i18n("Guitar") + " (6): open C" },
    { 6, {38,45,50,54,57,62},       i18n("Guitar") + " (6): open D" },
    { 6, {40,47,52,56,59,64},       i18n("Guitar") + " (6): open E" },

    { 7, {33,40,45,50,55,59,64},    i18n("Guitar") + " (7): rock" },
    { 7, {38,43,47,50,55,59,62},    i18n("Guitar") + " (7): traditional" },

    { 8, {33,40,45,50,55,59,64,69}, i18n("Guitar") + " (8): brahms" },

    { 4, {28,33,38,43},             i18n("Bass") + " (4): standard" },
    { 4, {26,33,38,43},             i18n("Bass") + " (4): drop D" },

    { 5, {28,33,38,43,47},          i18n("Bass") + " (5): standard" },

    { 5, {67,50,55,59,62},          i18n("Banjo") + " (5): open G" },
    { 5, {67,48,55,59,62},          i18n("Banjo") + " (5): drop C" },
    { 5, {69,50,54,57,62},          i18n("Banjo") + " (5): open D" },
    { 5, {67,50,55,58,62},          i18n("Banjo") + " (5): G minor" },
    { 5, {67,50,55,57,62},          i18n("Banjo") + " (5): G modal" },

    { 4, {55,62,69,76},             i18n("Mandolin") + " (4): standard" },

    { 4, {57,50,54,59},             i18n("Ukulele") + " (4): standard" },

    { 0, {}, 0 }
};
