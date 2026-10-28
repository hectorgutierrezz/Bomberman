#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "SoundManager.h"
#include <iostream>
#include <unordered_map>
#include <cstdio>

static std::string resolveAudioPath(const std::string& path) {
	FILE* f = fopen(path.c_str(), "rb");
	if (f) {
		fclose(f);
		return path;
	}

	std::string alt1 = "02-Bubble/" + path;
	f = fopen(alt1.c_str(), "rb");
	if (f) {
		fclose(f);
		return alt1;
	}

	std::string alt2 = "../../02-Bubble/" + path;
	f = fopen(alt2.c_str(), "rb");
	if (f) {
		fclose(f);
		return alt2;
	}

	// Provant variació sfx minúscula / majúscula
	std::string alt3 = path;
	size_t pos = alt3.find("SFX");
	if (pos != std::string::npos) {
		alt3.replace(pos, 3, "sfx");
		f = fopen(alt3.c_str(), "rb");
		if (f) {
			fclose(f);
			return alt3;
		}
	}

	return path;
}

struct SoundManager::Impl {
	ma_context context;
	bool hasCustomContext;
	ma_engine engine;
	ma_sound_group musicGroup;
	ma_sound_group sfxGroup;
	ma_sound_group explosionGroup;
	ma_sound_group stepGroup;
	ma_sound music;
	bool musicActive;

	std::unordered_map<SoundEffect, std::string> sfxFiles;
	std::unordered_map<MusicTrack, std::string> musicFiles;

	Impl() : hasCustomContext(false), musicActive(false) {}
};

SoundManager::SoundManager() : pImpl(NULL), currentTrack(MUSIC_NONE), initialized(false) {
}

SoundManager::~SoundManager() {
	free();
}

SoundManager& SoundManager::instance() {
	static SoundManager sm;
	return sm;
}

bool SoundManager::init() {
	if (initialized)
		return true;

	pImpl = new Impl();

#ifdef __linux__
	// En entorns WSL2 / WSLg, assegurar que PulseServer apunta al socket del sistema
	if (getenv("PULSE_SERVER") == NULL) {
		FILE* wslgTest = fopen("/mnt/wslg/PulseServer", "rb");
		if (wslgTest != NULL) {
			fclose(wslgTest);
			setenv("PULSE_SERVER", "unix:/mnt/wslg/PulseServer", 1);
		}
	}

	ma_backend backends[] = { ma_backend_pulseaudio, ma_backend_alsa };
	ma_context_config contextConfig = ma_context_config_init();
	if (ma_context_init(backends, 2, &contextConfig, &pImpl->context) == MA_SUCCESS) {
		pImpl->hasCustomContext = true;
	}
#endif

	ma_engine_config engineConfig = ma_engine_config_init();
	if (pImpl->hasCustomContext) {
		engineConfig.pContext = &pImpl->context;
	}

	ma_result result = ma_engine_init(&engineConfig, &pImpl->engine);
	if (result != MA_SUCCESS) {
		std::cerr << "[SoundManager] Error: No s'ha pogut inicialitzar l'engine d'audio (codi " << result << ")" << std::endl;
		if (pImpl->hasCustomContext)
			ma_context_uninit(&pImpl->context);
		delete pImpl;
		pImpl = NULL;
		return false;
	}

	// Grups de so per controlar volum independent
	ma_sound_group_init(&pImpl->engine, 0, NULL, &pImpl->musicGroup);
	ma_sound_group_init(&pImpl->engine, 0, NULL, &pImpl->sfxGroup);
	ma_sound_group_init(&pImpl->engine, 0, NULL, &pImpl->explosionGroup);
	ma_sound_group_init(&pImpl->engine, 0, NULL, &pImpl->stepGroup);

	ma_sound_group_set_volume(&pImpl->musicGroup, 0.40f);     // Música de fons continguda
	ma_sound_group_set_volume(&pImpl->sfxGroup, 1.20f);       // Efectes sonors clars
	ma_sound_group_set_volume(&pImpl->explosionGroup, 2.20f); // Explosions contundents amb potència extra
	ma_sound_group_set_volume(&pImpl->stepGroup, 0.35f);      // Passos suaus i discrets que no tapen el joc

	// Diccionari de sons SFX
	pImpl->sfxFiles[SFX_BOMB_LAY]           = "sound/SFX/bomb_lay.wav";
	pImpl->sfxFiles[SFX_BOMB_EXPLOSION]     = "sound/SFX/bomb_explosion.wav";
	pImpl->sfxFiles[SFX_POWERUP]            = "sound/SFX/powerup.wav";
	pImpl->sfxFiles[SFX_BOMBERMAN_KILLED]   = "sound/SFX/bomberman_killed.wav";
	pImpl->sfxFiles[SFX_PAUSE]              = "sound/SFX/pause.wav";
	pImpl->sfxFiles[SFX_STEP_HORIZONTAL]    = "sound/SFX/step_horizontal.wav";
	pImpl->sfxFiles[SFX_STEP_VERTICAL]      = "sound/SFX/step_vertical.wav";

	// Diccionari de cançons de música
	pImpl->musicFiles[MUSIC_TITLE]        = "sound/music/02_Title Screen.mp3";
	pImpl->musicFiles[MUSIC_MAP]          = "sound/music/04_Map Screen.mp3";
	pImpl->musicFiles[MUSIC_WORLD_INTRO]  = "sound/music/05_World Intro.mp3";
	pImpl->musicFiles[MUSIC_STAGE_FOREST] = "sound/music/06_World of Forest [Area 1].mp3";
	pImpl->musicFiles[MUSIC_BOSS_BATTLE]  = "sound/music/11. Boss Battle.mp3";
	pImpl->musicFiles[MUSIC_CONTINUE]     = "sound/music/14. Continue.mp3";
	pImpl->musicFiles[MUSIC_ENDING]       = "sound/music/15. Ending.mp3";
	pImpl->musicFiles[MUSIC_STAGE_CLEAR]  = "sound/music/16. Stage Clear.mp3";
	pImpl->musicFiles[MUSIC_WORLD_CLEAR]  = "sound/music/17. World Clear.mp3";
	pImpl->musicFiles[MUSIC_TIME_OVER]    = "sound/music/19. Time Over.mp3";
	pImpl->musicFiles[MUSIC_GAME_OVER]    = "sound/music/20. Game Over.mp3";

	initialized = true;

	ma_device* pDevice = ma_engine_get_device(&pImpl->engine);
	if (pDevice != NULL && pDevice->pContext != NULL) {
		std::cout << "[SoundManager] Audio inicialitzat correctament (" 
		          << ma_get_backend_name(pDevice->pContext->backend) 
		          << " - " << pDevice->playback.name << ")." << std::endl;
	} else {
		std::cout << "[SoundManager] Audio engine inicialitzat correctament." << std::endl;
	}
	return true;
}

void SoundManager::free() {
	if (!initialized || pImpl == NULL)
		return;

	stopMusic();

	ma_sound_group_uninit(&pImpl->musicGroup);
	ma_sound_group_uninit(&pImpl->sfxGroup);
	ma_sound_group_uninit(&pImpl->explosionGroup);
	ma_sound_group_uninit(&pImpl->stepGroup);
	ma_engine_uninit(&pImpl->engine);

	if (pImpl->hasCustomContext) {
		ma_context_uninit(&pImpl->context);
		pImpl->hasCustomContext = false;
	}

	delete pImpl;
	pImpl = NULL;
	initialized = false;
	currentTrack = MUSIC_NONE;
}

void SoundManager::playSound(SoundEffect sfx, float volume) {
	if (!initialized || pImpl == NULL)
		return;

	auto it = pImpl->sfxFiles.find(sfx);
	if (it == pImpl->sfxFiles.end())
		return;

	std::string resolved = resolveAudioPath(it->second);
	(void)volume;
	ma_sound_group* group = &pImpl->sfxGroup;
	if (sfx == SFX_BOMB_EXPLOSION)
		group = &pImpl->explosionGroup;
	else if (sfx == SFX_STEP_HORIZONTAL || sfx == SFX_STEP_VERTICAL)
		group = &pImpl->stepGroup;

	ma_engine_play_sound(&pImpl->engine, resolved.c_str(), group);
}

void SoundManager::playMusic(MusicTrack track, bool loop, float volume) {
	if (!initialized || pImpl == NULL)
		return;

	if (track == MUSIC_NONE) {
		stopMusic();
		return;
	}

	// Si ja està sonant aquesta mateixa cançó en bucle, no la reiniciem
	if (currentTrack == track && pImpl->musicActive && isMusicPlaying() && loop) {
		return;
	}

	stopMusic();

	auto it = pImpl->musicFiles.find(track);
	if (it == pImpl->musicFiles.end())
		return;

	std::string resolved = resolveAudioPath(it->second);
	ma_result res = ma_sound_init_from_file(&pImpl->engine, resolved.c_str(), MA_SOUND_FLAG_STREAM, &pImpl->musicGroup, NULL, &pImpl->music);
	if (res != MA_SUCCESS) {
		std::cerr << "[SoundManager] No s'ha pogut carregar la pista musical: " << resolved << " (error " << res << ")" << std::endl;
		return;
	}

	ma_sound_set_looping(&pImpl->music, loop ? MA_TRUE : MA_FALSE);
	ma_sound_set_volume(&pImpl->music, volume);
	ma_sound_start(&pImpl->music);

	pImpl->musicActive = true;
	currentTrack = track;
}

void SoundManager::stopMusic() {
	if (!initialized || pImpl == NULL)
		return;

	if (pImpl->musicActive) {
		ma_sound_stop(&pImpl->music);
		ma_sound_uninit(&pImpl->music);
		pImpl->musicActive = false;
	}
	currentTrack = MUSIC_NONE;
}

void SoundManager::pauseMusic() {
	if (!initialized || pImpl == NULL || !pImpl->musicActive)
		return;

	ma_sound_stop(&pImpl->music);
}

void SoundManager::resumeMusic() {
	if (!initialized || pImpl == NULL || !pImpl->musicActive)
		return;

	ma_sound_start(&pImpl->music);
}

bool SoundManager::isMusicPlaying() const {
	if (!initialized || pImpl == NULL || !pImpl->musicActive)
		return false;

	return (ma_sound_is_playing(&pImpl->music) == MA_TRUE);
}

void SoundManager::setMasterVolume(float volume) {
	if (!initialized || pImpl == NULL)
		return;
	ma_engine_set_volume(&pImpl->engine, volume);
}

void SoundManager::setMusicVolume(float volume) {
	if (!initialized || pImpl == NULL)
		return;
	ma_sound_group_set_volume(&pImpl->musicGroup, volume);
}

void SoundManager::setSoundVolume(float volume) {
	if (!initialized || pImpl == NULL)
		return;
	ma_sound_group_set_volume(&pImpl->sfxGroup, volume);
}
