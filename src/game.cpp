#include "game.hpp"
#include <cassert>
#include <iostream>
#include <random>

static const sf::VideoMode videoMode(sf::Vector2u(640u, 480u));

Game::Game() : m_VertexArray(sf::PrimitiveType::Triangles, 500) {
	m_Window.create(videoMode, "Minesweeper");
	assert(m_Texture.loadFromFile("assets/textures.png"));
	m_Texture.setSmooth(false);
	randomize_cells();
	rebuild_va();

	while (m_Window.isOpen()) {
		sf::Event ev;
		while (m_Window.pollEvent(ev)) {
			switch (ev.type) {
				case sf::Event::Closed:
					m_Window.close();
					break;

				default:
					break;
			}
		}

        m_Window.clear(sf::Color::Red);
		m_Window.draw(m_VertexArray, &m_Texture);
        m_Window.display();
	}
}

void Game::randomize_cells() {
	std::mt19937 mt(std::time(nullptr));
	std::uniform_real_distribution<float> dist(0.0f, static_cast<float>(GRID_WIDTH*GRID_HEIGHT));

	std::uint32_t bombsRemaining = MINE_COUNT; 
	while(bombsRemaining != 0) {
		const std::uint32_t idx = dist(mt);
		CellState &cell = m_Cells[idx];

		if(cell.hasBomb) {
			continue;
		}

		cell.hasBomb = true;
		bombsRemaining--;
	}

	for(std::uint32_t y = 0; y < GRID_HEIGHT; ++y) {
		for(std::uint32_t x = 0; x < GRID_WIDTH; ++x) {
			if(cell_at(x,y).hasBomb) {
				putc('#', stdout);
			} else {
				putc(' ', stdout);
			}
		}
		putc('\n', stdout);
	}
}

void Game::rebuild_va() {
	m_VertexArray.clear();
	
	for(std::uint32_t y = 0; y < GRID_HEIGHT; ++y) {
		for(std::uint32_t x = 0; x < GRID_WIDTH; ++x) {
			const CellState &cellState = cell_at(x, y);

			if(cellState.isRevealed) {
				const std::uint32_t neighbouringBombs = count_neighbouring_bombs(x, y);
				add_cell_vertex(x, y, neighbouringBombs);
				continue;
			} 

			if(cellState.hasBomb) {
				add_cell_vertex(x, y, 9);
			}
		}
	}
}

std::uint32_t Game::count_neighbouring_bombs(const std::uint32_t x, const std::uint32_t y) {
	std::uint32_t bombCount = 0;
	for(std::int32_t offX = -1; offX <= 1; ++offX) {
		for(std::int32_t offY = -1; offY <= 1; ++offY) {
			const std::int32_t checkX = x + offX;
			const std::int32_t checkY = y + offY;

			if(checkX < 0 || checkX >= GRID_WIDTH || checkY < 0 || checkY >= GRID_HEIGHT) {
				continue;
			}

			if(cell_at(checkX, checkY).hasBomb) {
				bombCount++;
			}
		}
	}

	return bombCount;
}

void Game::add_cell_vertex(std::uint32_t x, std::uint32_t y, const uint32_t spriteIdx) {
	const sf::Vector2f cellSize = sf::Vector2f(static_cast<float>(m_Window.getSize().x) / GRID_WIDTH, static_cast<float>(m_Window.getSize().y) / GRID_HEIGHT);
	x *= cellSize.x;
	y *= cellSize.y;

	const sf::Vector2f uvStart{static_cast<float>(spriteIdx) * TEXTURE_ATLAS_SPRITE_SIZE, 0.0f};
	const sf::Vector2f uvEnd{static_cast<float>(spriteIdx + 1) * TEXTURE_ATLAS_SPRITE_SIZE, TEXTURE_ATLAS_SPRITE_SIZE};

	m_VertexArray.append(sf::Vertex(sf::Vector2f(x,y), sf::Color::White, uvStart));
	m_VertexArray.append(sf::Vertex(sf::Vector2f(x + cellSize.x, y), sf::Color::White, {uvEnd.x, uvStart.y}));
	m_VertexArray.append(sf::Vertex(sf::Vector2f(x + cellSize.x, y + cellSize.y), sf::Color::White, {uvEnd}));

	m_VertexArray.append(sf::Vertex(sf::Vector2f(x + cellSize.x, y + cellSize.y), sf::Color::White, {uvEnd}));
	m_VertexArray.append(sf::Vertex(sf::Vector2f(x, y + cellSize.y), sf::Color::White, {uvStart.x, uvEnd.y}));
	m_VertexArray.append(sf::Vertex(sf::Vector2f(x,y), sf::Color::White, uvStart));
}
