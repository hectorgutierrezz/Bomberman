#ifndef _SOUND_MANAGER_INCLUDE
#define _SOUND_MANAGER_INCLUDE

#include <string>

enum SoundEffect {
	SFX_BOMB_LAY,
	SFX_BOMB_EXPLOSION,
	SFX_POWERUP,
	SFX_BOMBERMAN_KILLED,
	SFX_PAUSE,
	SFX_STEP_HORIZONTAL,
	SFX_STEP_VERTICAL
};

enum MusicTrack {
	MUSIC_NONE,
	MUSIC_TITLE,         // 02_Title Screen.mp3
	MUSIC_MAP,           // 04_Map Screen.mp3
	MUSIC_WORLD_INTRO,   // 05_World Intro.mp3
	MUSIC_STAGE_FOREST,  // 06_World of Forest [Area 1].mp3
	MUSIC_BOSS_BATTLE,   // 11. Boss Battle.mp3
	MUSIC_CONTINUE,      // 14. Continue.mp3
	MUSIC_ENDING,        // 15. Ending.mp3
	MUSIC_STAGE_CLEAR,   // 16. Stage Clear.mp3
	MUSIC_WORLD_CLEAR,   // 17. World Clear.mp3
	MUSIC_TIME_OVER,     // 19. Time Over.mp3
	MUSIC_GAME_OVER      // 20. Game Over.mp3
};

class SoundManager {
public:
	static SoundManager& instance();

	bool init();
	void free();

	void playSound(SoundEffect sfx, float volume = 1.0f);
	void playMusic(MusicTrack track, bool loop = true, float volume = 0.7f);
	void stopMusic();
	void pauseMusic();
	void resumeMusic();

	MusicTrack getCurrentMusic() const { return currentTrack; }
	bool isMusicPlaying() const;

	void setMasterVolume(float volume);
	void setMusicVolume(float volume);
	void setSoundVolume(float volume);

private:
	SoundManager();
	~SoundManager();

	struct Impl;
	Impl* pImpl;

	MusicTrack currentTrack;
	bool initialized;
};

#endif // _SOUND_MANAGER_INCLUDE
