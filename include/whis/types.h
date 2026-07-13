#ifndef WHIS_TYPES_H_
#define WHIS_TYPES_H_ 1

typedef enum wh_fnresult
{
	WHIS_SUCCESS = 0,
	WHIS_INCOMPLETE,
	WHIS_FAILURE = -10,
	WHIS_INVARG
} wh_fnresult;

typedef struct wh_window wh_window;

#define WHIS_FALSE (0)
#define WHIS_TRUE (1)

typedef unsigned char wh_bool;

#endif /* WHIS_TYPES_H_ */
