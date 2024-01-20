#include <iostream>
#include "game.hpp"

#define REDFLAG_IMPLEMENTATION
#include "redflag.h"

int main(int argc, char **argv) {
	rf_context_t rfCtx;
	rf_init_context(&rfCtx);
	rf_flag_t *gridWidthFlag = rf_flag_int(&rfCtx, "w", "Grid width", 30);
	rf_flag_t *gridHeightFlag = rf_flag_int(&rfCtx, "h", "Grid height", 16);
	rf_flag_t *mineCountFlag = rf_flag_int(&rfCtx, "m", "Mine count", 99);
	rf_parse_flags(&rfCtx, argc, argv);

	Game game(gridWidthFlag->value.as_int, gridHeightFlag->value.as_int, mineCountFlag->value.as_int);
	rf_free_context(&rfCtx);
	game.start();
}
