typedef struct {
    int strings;
    uchar shift[MAX_STRINGS];
    QString name;
} tuning;

tuning lib_tuning[] = {
    { 6, {40,45,50,55,59,64},      "Guitar (6): standard" },
    { 6, {39,44,49,54,58,63},      "Guitar (6): dropped 1/2 tone" },
    { 6, {38,43,48,53,57,62},      "Guitar (6): dropped 1 tone" },
    { 6, {38,45,50,55,59,64},      "Guitar (6): drop D" },
    { 4, {28,33,38,43},            "Bass (4): standard" },
    { 5, {28,33,38,43,47},         "Bass (5): standard" },
    { 0 }
};
