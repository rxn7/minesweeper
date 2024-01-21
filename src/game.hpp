#pragma once

#include "audio.hpp"
#include "cell_state.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window.hpp>

enum GameState { Playing, Lost, Win };

class Game {
public:
	Game(const std::uint32_t gridWidth, const std::uint32_t gridHeight, const std::uint32_t mineCount);
	~Game();
	void start();

private:
	void generate_mines(const std::uint32_t ignoreX, const std::uint32_t ignoreY);
	void propagate_reveal(const std::uint32_t x, const std::uint32_t y);
	void end_game(GameState newState);
	void check_win();
	void play_sound(const sf::SoundBuffer &buffer);

	void handle_event(const sf::Event &event);
	void resize_view();

	void regenerate_vertices();
	void add_cell_vertex(std::uint32_t x, std::uint32_t y, const uint32_t spriteIdx);

	inline void reset_cells() { std::fill(m_Cells, m_Cells + get_grid_size(), CellState{}); }

	[[nodiscard]] inline std::uint32_t get_grid_size() const { return m_GridWidth * m_GridHeight; }

	[[nodiscard]] inline CellState &get_cell_at(const std::uint32_t x, const std::uint32_t y) {
		assert(is_valid_cell(x, y));
		return m_Cells[y * m_GridWidth + x];
	}

	[[nodiscard]] inline bool is_valid_cell(const std::uint32_t x, const std::uint32_t y) { return x >= 0 && x < m_GridWidth && y >= 0 && y < m_GridHeight; }

private:
	static constexpr std::uint32_t TEXTURE_ATLAS_SPRITE_SIZE = 16;
	static constexpr float CELL_SIZE = 1.0f;

	Audio m_Audio;
	sf::Sound m_Sound;
	sf::Font m_Font;
	sf::VertexArray m_VertexArray;

	sf::Text m_GameOverText;
	sf::Texture m_Texture;

	sf::View m_GridView;
	sf::View m_ViewUI;
	sf::RenderWindow m_Window;

	CellState *m_Cells;
	std::uint32_t m_GridWidth;
	std::uint32_t m_GridHeight;
	std::uint32_t m_MineCount;
	GameState m_State = GameState::Playing;
	bool m_MinesGenerated = false;
};
