/*
	music.h
	Wednesday, July 12, 1995 11:43:21 PM
	This only allows for one song to be played at a given time.
*/

enum { // All of our songs.
	_introduction_song= 0,
	NUMBER_OF_SONGS
};

boolean initialize_music_handler(FileDesc *song_file);

void queue_song(short song_index);

void music_idle_proc(void);

void stop_music(void);
void pause_music(boolean pause);

boolean music_playing(void);

void free_music_channel(void);
void fade_out_music(short duration);
