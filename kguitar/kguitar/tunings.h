// TUNINGS.H - library of stringed instrument tunings

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
    { 1, {},                       "User defined" },

    { 6, {40,45,50,55,59,64},      "Guitar (6): standard" },
    { 6, {39,44,49,54,58,63},      "Guitar (6): Eb" },
    { 6, {38,43,48,53,57,62},      "Guitar (6): D" },
    { 6, {38,45,50,55,59,64},      "Guitar (6): drop D" },
    { 6, {36,43,48,55,60,64},      "Guitar (6): open C" },
    { 6, {38,45,50,54,57,62},      "Guitar (6): open D" },
    { 6, {40,47,52,56,59,64},      "Guitar (6): open E" },

    { 7, {33,40,45,50,55,59,64},   "Guitar (7): rock" },
    { 7, {38,43,47,50,55,59,62},   "Guitar (7): traditional" },

    { 4, {28,33,38,43},            "Bass (4): standard" },
    { 4, {26,33,38,43},            "Bass (4): drop D" },

    { 5, {28,33,38,43,47},         "Bass (5): standard" },

    { 4, {57,50,54,59},            "Ukulele (4): standard" },                   

    { 0 }
};
