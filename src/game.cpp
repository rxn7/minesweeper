#include "game.hpp"
#include "SFML/System/Vector2.hpp"
#include <cassert>
#include <iostream>
#include <random>

static const sf::VideoMode videoMode(sf::Vector2u(640u, 480u));

Game::Game() : m_VertexArray(sf::PrimitiveType::Triangles, 500), m_View(VIEW_SIZE * 0.5f, VIEW_SIZE) {
	m_Window.create(videoMode, "Minesweeper");
	m_Window.setView(m_View);
	assert(m_Texture.loadFromFile("assets/textures.png"));
	m_Texture.setSmooth(false);
	rebuild_va();

	while (m_Window.isOpen()) {
		sf::Event ev;
		while (m_Window.pollEvent(ev)) {
			switch (ev.type) {
				case sf::Event::Closed:
					m_Window.close();
					break;

				case sf::Event::MouseButtonPressed: {
					const sf::Vector2f mousePos = m_Window.mapPixelToCoords(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));
					const sf::Vector2u cellPos(mousePos.x / m_View.getSize().x * GRID_WIDTH, mousePos.y / m_View.getSize().y * GRID_HEIGHT);

					if(m_IsGameOver) {
						m_IsGameOver = false;
						generate_bombs(cellPos.x, cellPos.y);
						break;
					}

					if(!m_BombsGenerated) {
						generate_bombs(cellPos.x, cellPos.y);
					}


					CellState &cell = get_cell_at(cellPos.x, cellPos.y);

					switch(ev.mouseButton.button) {
						case sf::Mouse::Button::Left: {
							if(cell.isFlagged) 
								break;

							if(cell.isMine) {
								game_over();
								break;
							}

							cell.isRevealed = true;
							propagate_reveal(cellPos.x, cellPos.y);
							rebuild_va();
							
							break;
						}

						case sf::Mouse::Button::Right: {
							cell.isFlagged ^= 1;
							rebuild_va();
							break;

					    }

						default:
							break;
					}
					break;
				}

				case sf::Event::Resized: {
					// TODO: 
					// const float winRatio = ev.size.width / (float)ev.size.height;
					// const float viewRatio = m_View.getSize().x / (float)m_View.getSize().y;
					// sf::Vector2f size;
					// sf::Vector2f position;
					//
					// if (winRatio > viewRatio) {
					// 	size.x = viewRatio / winRatio;
					// 	position.x = (1 - size.x) * 0.5f;
					// } else {
					// 	size.y = winRatio / viewRatio;
					// 	position.y = (1 - size.y) * 0.5f;
					// }
					//
					// m_View.setSize(size);
					// m_View.setCenter(position);
					m_Window.setView(m_View);
					break;
				}


				default:
					break;
			}
		}

        m_Window.clear(sf::Color(200, 200, 200));
		m_Window.draw(m_VertexArray, &m_Texture);
        m_Window.display();
	}
}

void Game::generate_bombs(const std::uint32_t ignoreX, const std::uint32_t ignoreY) {
	m_BombsGenerated = true;

	std::fill(m_Cells.begin(), m_Cells.end(), CellState{});

	std::mt19937 mt(std::time(nullptr));
	std::uniform_int_distribution<std::uint32_t> dist(0, GRID_WIDTH*GRID_HEIGHT);

	std::uint32_t bombsRemaining = MINE_COUNT; 
	while(bombsRemaining != 0) {
		const std::uint32_t idx = dist(mt);
		const std::uint32_t x = idx % GRID_WIDTH;
		const std::uint32_t y = idx / GRID_WIDTH;

		if(x == ignoreX && y == ignoreY) {
			continue;
		}

		CellState &cell = m_Cells[idx];

		if(cell.isMine) {
			continue;
		}

		cell.isMine = true;

		for(std::int32_t offX = -1; offX <= 1; ++offX) {
			for(std::int32_t offY = -1; offY <= 1; ++offY) {
				const std::int32_t checkX = x + offX;
				const std::int32_t checkY = y + offY;

				if((offX == 0 && offY == 0) || checkX < 0 || checkX >= GRID_WIDTH || checkY < 0 || checkY >= GRID_HEIGHT) {
					continue;
				}

				get_cell_at(checkX, checkY).neighbouringMines++;
			}
		}
		bombsRemaining--;
	}

	for(std::uint32_t y = 0; y < GRID_HEIGHT; ++y) {
		for(std::uint32_t x = 0; x < GRID_WIDTH; ++x) {
			if(get_cell_at(x,y).isMine) {
				putc('#', stdout);
			} else {
				putc(' ', stdout);
			}
		}
		putc('\n', stdout);
	}

	rebuild_va();
}

void Game::propagate_reveal(const std::uint32_t x, const std::uint32_t y) {
	for(std::int32_t offX = -1; offX <= 1; ++offX) {
		for(std::int32_t offY = -1; offY <= 1; ++offY) {
			const std::int32_t cellY = y + offY;
			const std::int32_t cellX = x + offX;

			if((offX == 0 && offY == 0) || cellX < 0 || cellX >= GRID_WIDTH || cellY < 0 || cellY >= GRID_HEIGHT) {
				continue;
			}

			CellState &target = get_cell_at(cellX, cellY);

			if(!target.isRevealed && !target.isFlagged && !target.isMine) {
				target.isRevealed = true;

				if(target.neighbouringMines == 0) {
					propagate_reveal(cellX, cellY);
				}
			}
		}
	}
}

void Game::game_over() {
	m_IsGameOver = true;
	m_BombsGenerated = false;
	for(CellState &cell : m_Cells) {
		if(!cell.isFlagged && cell.isMine) {
			cell.isRevealed = true;
		}
	}
	rebuild_va();
}

void Game::rebuild_va() {
	m_VertexArray.clear();
	
	for(std::uint32_t y = 0; y < GRID_HEIGHT; ++y) {
		for(std::uint32_t x = 0; x < GRID_WIDTH; ++x) {
			const CellState &cellState = get_cell_at(x, y);

			if(cellState.isRevealed) {
				if(cellState.isMine) {
					add_cell_vertex(x, y, 9);
					continue;
				}

				if(cellState.neighbouringMines != 0) {
					add_cell_vertex(x, y, cellState.neighbouringMines);
				}

				continue;
			} 

			if(cellState.isFlagged) {
				add_cell_vertex(x, y, 10);
				continue;
			}

			add_cell_vertex(x, y, 0);
		}
	}
}

void Game::add_cell_vertex(std::uint32_t x, std::uint32_t y, const uint32_t spriteIdx) {
	const sf::Vector2f cellSize(m_View.getSize().x / GRID_WIDTH, m_View.getSize().y / GRID_HEIGHT);
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
