#pragma once

#include <cstdint>

struct CellState {
	bool isFlagged : 1 = false;
	bool isMine : 1 = false;
	bool isRevealed : 1 = false;
	std::uint8_t neighboringMines : 4 = 0u;
};
