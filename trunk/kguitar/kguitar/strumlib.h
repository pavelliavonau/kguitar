// Strumming library defines and headers

#define MAX_STRUM_LENGTH	8

#define ST1			1
#define ST2			2
#define ST3			4
#define ST4			8
#define ST5			16
#define ST6			32

typedef struct {
	int mask[MAX_STRUM_LENGTH];
	uint len[MAX_STRUM_LENGTH];
	QString name;
} strummer;

