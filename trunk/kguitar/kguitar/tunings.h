typedef struct {
    int strings;
    uchar shift[MAX_STRINGS];
    QString name;
} tuning;

tuning lib_tuning[] = {
    { 6, {40,45,50,55,59,64},      "Standard 6-string guitar" },
    { 6, {39,44,49,54,58,63},      "Dropped 6-string guitar" },
    { 4, {28,33,38,43},            "Standard 4-string bass" },
    { 5, {28,33,38,43,47},         "Standard 5-string bass" },
    { 0 }
};
