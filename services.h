#pragma once
#define MPV_START_PLAY_SERVICE "Mpv-StartPlayService_v1_0"
#define MPV_SET_TITLE_SERVICE "Mpv-SetTitleService_v1_0"

typedef struct
{
  char* Filename;
  char* Title;
} Mpv_StartPlayService_v1_0_t;

typedef struct
{
  char* Title;
} Mpv_SetTitleService_v1_0_t;