#include "game.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <sstream>

static const sf::VideoMode videoMode(sf::Vector2u(640u, 480u));

Game::Game(const std::uint32_t gridWidth, const std::uint32_t gridHeight, const std::uint32_t mineCount) : m_VertexArray(sf::PrimitiveType::Triangles, 500), m_Sound(m_RevealSoundBuffer), m_GameOverText(m_Font), m_GridWidth(gridWidth), m_GridHeight(gridHeight), m_MineCount(mineCount), m_Cells(new CellState[gridWidth * gridHeight]) {
	assert(m_MineCount < get_grid_size());
	std::fill(m_Cells, m_Cells + get_grid_size(), CellState{});
}

Game::~Game() {
	delete[] m_Cells;
}

void Game::start() {
	m_Window.create(videoMode, "Minesweeper");
	m_Window.setView(m_GridView);

	assert(m_Font.loadFromFile("assets/CozetteVector.ttf"));
	m_Font.setSmooth(false);

	m_GameOverText.setOutlineThickness(5.0f);
	m_GameOverText.setCharacterSize(64);

	assert(m_Texture.loadFromFile("assets/textures.png"));
	assert(m_RevealSoundBuffer.loadFromFile("assets/audio/reveal.ogg"));
	assert(m_ExplosionSoundBuffer.loadFromFile("assets/audio/explosion.ogg"));
	assert(m_FlagSoundBuffer.loadFromFile("assets/audio/flag.ogg"));

	m_Texture.setSmooth(false);
	regenerate_vertices();

	while (m_Window.isOpen()) {
		sf::Event ev;
		while (m_Window.pollEvent(ev)) {
			switch (ev.type) {
				case sf::Event::Closed:
					m_Window.close();
					break;

				case sf::Event::MouseButtonPressed: {
					m_Window.setView(m_GridView);
					const sf::Vector2f mousePos = m_Window.mapPixelToCoords(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));
					const sf::Vector2u cellPos(mousePos.x / m_GridView.getSize().x * m_GridWidth, mousePos.y / m_GridView.getSize().y * m_GridHeight);

					if(m_State != GameState::Playing) {
						generate_mines(cellPos.x, cellPos.y);
						m_State = GameState::Playing;
						break;
					}

					if(!m_MinesGenerated) {
						generate_mines(cellPos.x, cellPos.y);
					}

					CellState &cell = get_cell_at(cellPos.x, cellPos.y);

					switch(ev.mouseButton.button) {
						case sf::Mouse::Button::Left: {
							if(cell.isFlagged) 
								break;

							if(cell.isMine) {
								play_sound(m_ExplosionSoundBuffer);
								game_over(GameState::Lost);
								break;
							}

							if(cell.isRevealed)  {
								break;
							}

							play_sound(m_RevealSoundBuffer);

							cell.isRevealed = true;
							propagate_reveal(cellPos.x, cellPos.y);
							check_win();
							regenerate_vertices();
							
							break;
						}

						case sf::Mouse::Button::Right: {
							if(cell.isRevealed) {
								break;
							}

							cell.isFlagged ^= 1;
							play_sound(m_FlagSoundBuffer);
							regenerate_vertices();
							break;

					    }

						default:
							break;
					}
					break;
				}

				case sf::Event::Resized: {
					// TODO: letterboxing :(
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
					m_ViewUI.setSize(sf::Vector2f(m_Window.getSize()));
					m_ViewUI.setCenter(sf::Vector2f(0.0f, 0.0f));
					m_GameOverText.setScale(sf::Vector2f(1.0f, 1.0f) * m_ViewUI.getSize().x / 1000.0f);
					break;
				}


				default:
					break;
			}
		}

        m_Window.clear(sf::Color(200, 200, 200));

		m_Window.setView(m_GridView);
		m_Window.draw(m_VertexArray, &m_Texture);

		m_Window.setView(m_ViewUI);
		if(m_State != GameState::Playing) {
			m_Window.draw(m_GameOverText);
		}

        m_Window.display();
	}
}

void Game::generate_mines(const std::uint32_t ignoreX, const std::uint32_t ignoreY) {
	m_MinesGenerated = true;

	std::fill(m_Cells, m_Cells + get_grid_size(), CellState{});

	std::mt19937 mt(std::time(nullptr));
	std::uniform_int_distribution<std::uint32_t> dist(0, m_GridWidth*m_GridHeight);

	std::uint32_t minesRemaining = m_MineCount; 
	while(minesRemaining != 0) {
		const std::uint32_t idx = dist(mt);
		const std::uint32_t x = idx % m_GridWidth;
		const std::uint32_t y = idx / m_GridWidth;

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

				if((offX == 0 && offY == 0) || checkX < 0 || checkX >= m_GridWidth || checkY < 0 || checkY >= m_GridHeight) {
					continue;
				}

				get_cell_at(checkX, checkY).neighbouringMines++;
			}
		}

		minesRemaining--;
	}

	regenerate_vertices();
}

void Game::check_win() {
	for(std::uint32_t i=0; i<get_grid_size(); ++i) {
		CellState &cell = m_Cells[i];
		if(!cell.isMine && !cell.isRevealed) {
			return;
		}
	}

	game_over(GameState::Win);
}

void Game::play_sound(const sf::SoundBuffer &buffer) {
	m_Sound.setBuffer(buffer);
	m_Sound.setPitch(0.9f + (rand() / static_cast<float>(RAND_MAX) * 0.2f));
	m_Sound.play();
}

void Game::propagate_reveal(const std::uint32_t x, const std::uint32_t y) {
	for(std::int32_t offX = -1; offX <= 1; ++offX) {
		for(std::int32_t offY = -1; offY <= 1; ++offY) {
			const std::int32_t cellY = y + offY;
			const std::int32_t cellX = x + offX;

			if((offX == 0 && offY == 0) || cellX < 0 || cellX >= m_GridWidth || cellY < 0 || cellY >= m_GridHeight) {
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

void Game::game_over(GameState newState) {
	m_State = newState;
	m_MinesGenerated = false;

	for(std::uint32_t i = 0; i<get_grid_size(); ++i) {
		CellState &cell = m_Cells[i];
		if(!cell.isFlagged && cell.isMine) {
			cell.isRevealed = true;
		}
	}

	std::ostringstream ss;
	switch(newState) {
		case GameState::Win:
			ss << "You win!";
			break;

		case GameState::Lost:
			ss << "You lost!";
			break;

		default:
			break;
	}
	ss << "\nClick anywhere to play again";

	m_GameOverText.setString(ss.str());
	m_GameOverText.setOrigin(m_GameOverText.getLocalBounds().getSize() * 0.5f);

	regenerate_vertices();
}

void Game::regenerate_vertices() {
	m_VertexArray.clear();
	
	for(std::uint32_t y = 0; y < m_GridHeight; ++y) {
		for(std::uint32_t x = 0; x < m_GridWidth; ++x) {
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
	const sf::Vector2f cellSize(m_GridView.getSize().x / m_GridWidth, m_GridView.getSize().y / m_GridHeight);
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
