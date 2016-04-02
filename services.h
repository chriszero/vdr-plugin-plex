#pragma once
#define MPV_PLAY_FILE "Mpv_PlayFile"
#define MPV_SET_TITLE "Mpv_SetTitle"
#define MPV_SEEK "Mpv_Seek"

// play the given Filename, this can be a media file or a playlist
typedef struct {
    char *Filename;
} Mpv_PlayFile;


// Overrides the displayed title in replay info
typedef struct {
    char *Title;
} Mpv_SetTitle;

typedef struct {
    int SeekAbsolute;
    int SeekRelative;
} Mpv_Seek;