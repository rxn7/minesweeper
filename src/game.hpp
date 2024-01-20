#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include "SFML/Graphics/Font.hpp"
#include "cell_state.hpp"

enum GameState { Playing, Lost, Win };

class Game {
public:
	Game(const std::uint32_t gridWidth, const std::uint32_t gridHeight, const std::uint32_t mineCount);
	~Game();
	void start();

private:
	void generate_mines(const std::uint32_t ignoreX, const std::uint32_t ignoreY);
	void propagate_reveal(const std::uint32_t x, const std::uint32_t y);
	void game_over(GameState newState);
	void check_win();
	void play_sound(const sf::SoundBuffer &buffer);

	void regenerate_vertices();
	void add_cell_vertex(std::uint32_t x, std::uint32_t y, const uint32_t spriteIdx);

	inline std::uint32_t get_grid_size() const {
		return m_GridWidth * m_GridHeight;
	}

	inline CellState &get_cell_at(const std::uint32_t x, const std::uint32_t y) {
		return m_Cells[y * m_GridWidth + x];
	}

private:
	static constexpr std::uint32_t TEXTURE_ATLAS_SPRITE_SIZE = 16;

	sf::SoundBuffer m_RevealSoundBuffer;
	sf::SoundBuffer m_ExplosionSoundBuffer;
	sf::SoundBuffer m_FlagSoundBuffer;
	sf::Sound m_Sound;
	sf::Font m_Font;
	sf::Text m_GameOverText;
	sf::View m_GridView;
	sf::View m_ViewUI;
	sf::RenderWindow m_Window;
	sf::VertexArray m_VertexArray;
	sf::Texture m_Texture;
	CellState *m_Cells;

	std::uint32_t m_GridWidth;
	std::uint32_t m_GridHeight;
	std::uint32_t m_MineCount;
	GameState m_State = GameState::Playing;
	bool m_MinesGenerated = false;
};
