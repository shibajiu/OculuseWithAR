#pragma once
typedef enum class _Operation {
	NONE = -1,
	DISPLAY = 0b0,
	ROTATION,
	DISPLACEMENT,
	SCALE,
	ROTATION_SWITCH,
	DISPLACEMENT_SWITCH,
	SCALE_SWITCH
} Operation;

typedef enum class _iFlyCommandOperation {
	RESET = 100,
	QUIT = 0b0,
	ROTATION = 0b01,
	DISPLACEMENT = 0b10,
	SCALE = 0b100
} SCommand;