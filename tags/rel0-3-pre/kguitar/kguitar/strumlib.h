// Strumming library defines and headers

#define MAX_STRUM_LENGTH	10

// Treble order

#define TR1			1
#define TR2			2
#define TR3			4
#define TR4			8
#define TR5			16
#define TR6			32

// Bass order

#define BS1			-1
#define BS2			-2
#define BS3			-4
#define BS4			-8
#define BS5			-16
#define BS6			-32

typedef struct {
	int mask[MAX_STRUM_LENGTH];
	uint len[MAX_STRUM_LENGTH];
	QString name;
	QString description;
} strummer;

