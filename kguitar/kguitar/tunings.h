// TUNINGS.H - library of stringed instrument tunings

typedef struct {
    int strings;
    uchar shift[MAX_STRINGS];
    QString name;
} tuning;

// The library itself. Columns are:
// 1st - number of strings
// 2nd - array of tuning - midi note numbers (should be fairly easy to guess)
// 3rd - name of the tuning in the form "Instrument (strings): tuning"

tuning lib_tuning[] = {
    { 1, {},                       "User defined" },
    { 6, {40,45,50,55,59,64},      "Guitar (6): standard" },
    { 6, {39,44,49,54,58,63},      "Guitar (6): dropped 1/2 tone" },
    { 6, {38,43,48,53,57,62},      "Guitar (6): dropped 1 tone" },
    { 6, {38,45,50,55,59,64},      "Guitar (6): drop D" },
    { 4, {28,33,38,43},            "Bass (4): standard" },
    { 4, {26,33,38,43},            "Bass (4): drop D" },
    { 5, {28,33,38,43,47},         "Bass (5): standard" },
    { 0 }
};
