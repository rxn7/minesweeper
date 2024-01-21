#pragma once

#include <SFML/Audio.hpp>

struct Audio {
	Audio();

	sf::SoundBuffer revealSoundBuffer;
	sf::SoundBuffer explosionSoundBuffer;
	sf::SoundBuffer flagSoundBuffer;
	sf::SoundBuffer win;
};
