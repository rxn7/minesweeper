#pragma once

#include <cstdint>

struct CellState {
	bool isRevealed = false;
	bool isMine = false;
	bool isFlagged = false;
	std::uint8_t neighbouringMines = 0;
};
