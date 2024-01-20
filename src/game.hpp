#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "SFML/Graphics/View.hpp"
#include "cell_state.hpp"

class Game {
public:
	Game();

private:
	void generate_bombs(const std::uint32_t ignoreX, const std::uint32_t ignoreY);
	void propagate_reveal(const std::uint32_t x, const std::uint32_t y);
	void game_over();
	void rebuild_va();
	void add_cell_vertex(std::uint32_t x, std::uint32_t y, const uint32_t spriteIdx);

	inline CellState &get_cell_at(const std::uint32_t x, const std::uint32_t y) {
		return m_Cells[y * GRID_WIDTH + x];
	}

private:
	static constexpr std::uint32_t TEXTURE_ATLAS_SPRITE_SIZE = 16;
	static constexpr std::uint32_t CELL_SIZE = 64;
	static constexpr std::uint32_t GRID_WIDTH = 30;
	static constexpr std::uint32_t GRID_HEIGHT = 16;
	static constexpr sf::Vector2f VIEW_SIZE{GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE};
	static constexpr std::uint32_t MINE_COUNT = 99;
	static constexpr std::uint32_t VERTEX_COUNT = GRID_WIDTH * GRID_HEIGHT * 4;

	sf::View m_View;
	sf::RenderWindow m_Window;
	sf::VertexArray m_VertexArray;
	sf::Texture m_Texture;
	std::array<CellState, GRID_HEIGHT * GRID_WIDTH> m_Cells;

	bool m_IsGameOver = false;
	bool m_BombsGenerated = false;
};
