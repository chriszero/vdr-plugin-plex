#pragma once
#include <string>

#define PLAY_OSD_3DMODE_SERVICE	"Play-Osd3DModeService-v1.0"
#define PLAY_START_PLAY_SERVICE "Play-StartPlayService_v1_0"
#define PLAY_SET_TITLE_SERVICE "Play-SetTitleService_v1_0"

typedef struct
{
    int Mode;
} Play_Osd3DModeService_v1_0_t;

typedef struct
{
	char* Filename;
	char* Title;
} Play_StartPlayService_v1_0_t;

typedef struct
{
	char* Title;
} Play_SetTitleService_v1_0_t;