#include "audio.hpp"

Audio::Audio() {
	assert(revealSoundBuffer.loadFromFile("assets/audio/reveal.ogg"));
	assert(explosionSoundBuffer.loadFromFile("assets/audio/explosion.ogg"));
	assert(flagSoundBuffer.loadFromFile("assets/audio/flag.ogg"));
	assert(win.loadFromFile("assets/audio/win.ogg"));
}
