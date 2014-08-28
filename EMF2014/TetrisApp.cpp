/*
 TiLDA Mk2

 Tetris

 The MIT License (MIT)

 Copyright (c) 2014 Electromagnetic Field LTD

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <fonts/allFonts.h>

#include "debug.h"
#include "glcd.h"
#include "TetrisApp.h"

class Tetris : public Game {
private:
	static const int BLOCK_WIDTH = 5;
	static const int ROWS = 22;

	const int score_multipliers[4] = { 40, 100, 300, 1200 };

	enum tetradType {I, O, Z, S, T, L, J, NONE};

	class Pos {
	public:
		const Pos operator+(const Pos &other) {
			Pos sum;
			sum.x = x + other.x;
			sum.y = y + other.y;
			return sum;
		}
		int x;
		int y;
	};

	class Board {
	private:
		tetradType furniture[ROWS][10];
	public:
		Board() {
			for (int y = 0; y < ROWS; y++) {
				for (int x = 0; x < 10; x++) {
					furniture[y][x] = NONE;
				}
			}
		}

		void drawBlock(Pos pos, tetradType t) {
			if (t == NONE) return;

			int screenX = pos.x * BLOCK_WIDTH;
			int screenY = 127 - (pos.y + 1) * BLOCK_WIDTH;

			if (screenX < 0 || (screenX + BLOCK_WIDTH) > 63 ||
			    screenY < 0 || (screenY + BLOCK_WIDTH) > 127) {
				debug::log("Attempting to draw block at invalid position: " +
					   String(pos.x) + "," + String(pos.y) + " -> " +
					   String(screenX) + "," + String(screenY));
				return;
			}

			GLCD.DrawRect(screenX, screenY, BLOCK_WIDTH, BLOCK_WIDTH, BLACK);

			// Textures: note that these are fairly dependent on BLOCK_WIDTH being odd
			switch (t) {
			case I:  // double thick box
				GLCD.DrawRect(screenX+1, screenY+1, BLOCK_WIDTH-2, BLOCK_WIDTH-2, BLACK);
				break;
			case O:  // box filled with cross
				GLCD.DrawRect(screenX+1, screenY+BLOCK_WIDTH/2, BLOCK_WIDTH-2, 1, BLACK);
				GLCD.DrawRect(screenX+BLOCK_WIDTH/2, screenY+1, 1, BLOCK_WIDTH-2, BLACK);
				break;
			case Z:  // diagonals
				GLCD.DrawLine(screenX+1, screenY+BLOCK_WIDTH-1, screenX+BLOCK_WIDTH-1, screenY+1, BLACK);
				GLCD.DrawLine(screenX+1, screenY+1, screenX+BLOCK_WIDTH-1, screenY+BLOCK_WIDTH-1, BLACK);
				break;
			case S:  // dot in centre
				GLCD.DrawRect(screenX+2, screenY+2, 1, 1, BLACK);
				break;
			case T:  // top left to bottom right
				GLCD.DrawLine(screenX+1, screenY+BLOCK_WIDTH-1, screenX+BLOCK_WIDTH-1, screenY+1, BLACK);
				break;
			case L:  // chequerboard
				for (int x = 0; x < BLOCK_WIDTH - 1; x++) {
					for (int y = 0; y < BLOCK_WIDTH - 1; y++) {
						if (x % 2 ^ y % 2) {
							GLCD.SetDot(screenX+1+x, screenY+1+y, BLACK);
						}
					}
				}
				break;
			case J:  // bottom left to top right
				GLCD.DrawLine(screenX+1, screenY+1, screenX+BLOCK_WIDTH-1, screenY+BLOCK_WIDTH-1, BLACK);
				break;
			}

		}

		void draw() {
			GLCD.DrawRect(0, 127 - ROWS * BLOCK_WIDTH, 10 * BLOCK_WIDTH, ROWS * BLOCK_WIDTH);

			Pos p;
			for (int x = 0; x < 10; x++) {
				for (int y = 0; y < ROWS; y++) {
					p.x = x;
					p.y = y;
					drawBlock(p, furniture[y][x]);
				}
			}
		}

		bool isPosAvailable(Pos p) {
			return (p.x >= 0 && p.x < 10 &&
				p.y >= 0 && p.y < ROWS &&
				furniture[p.y][p.x] == NONE);
		}

		void setPos(Pos p, tetradType t) {
			furniture[p.y][p.x] = t;
		}

		int clearLines() {
			int x, y, cleared = 0;

			for (y = 0; y < ROWS;) {
				bool full = true;

				for (x = 0; x < 10; x++) {
					if (furniture[y][x] == NONE) {
						full = false;
						break;
					}
				}
				if (!full) {
					++y;
					continue;
				}

				int toMove = (ROWS - 1 - y);
				if (toMove) {
					// don't try this at home kids
					memmove(furniture[y], furniture[y + 1], sizeof(tetradType) * 10 * toMove);
				}
				for (x = 0; x < 10; x++) {
					furniture[ROWS - 1][x] = NONE;
				}
				cleared++;
			}
			return cleared;
		}
	};

	class Tetrad {
	private:
		const Pos layouts[7][4][4] = {
			{ // I
				{{0, 1}, {1, 1}, {2, 1}, {3, 1}},
				{{2, 0}, {2, 1}, {2, 2}, {2, 3}},
				{{0, 1}, {1, 1}, {2, 1}, {3, 1}},
				{{2, 0}, {2, 1}, {2, 2}, {2, 3}}
			},
			{ // O
				{{0, 0}, {0, 1}, {1, 0}, {1, 1}},
				{{0, 0}, {0, 1}, {1, 0}, {1, 1}},
				{{0, 0}, {0, 1}, {1, 0}, {1, 1}},
				{{0, 0}, {0, 1}, {1, 0}, {1, 1}}
			},
			{ // Z
				{{0, 1}, {1, 0}, {1, 1}, {2, 0}},
				{{1, 0}, {1, 1}, {2, 1}, {2, 2}},
				{{0, 1}, {1, 0}, {1, 1}, {2, 0}},
				{{1, 0}, {1, 1}, {2, 1}, {2, 2}},
			},
			{ // S
				{{0, 0}, {1, 0}, {1, 1}, {2, 1}},
				{{2, 0}, {1, 1}, {2, 1}, {1, 2}},
				{{0, 0}, {1, 0}, {1, 1}, {2, 1}},
				{{2, 0}, {1, 1}, {2, 1}, {1, 2}},
			},
			{ // T
				{{0, 1}, {1, 1}, {2, 1}, {1, 2}},
				{{1, 0}, {1, 1}, {1, 2}, {2, 1}},
				{{0, 1}, {1, 1}, {2, 1}, {1, 0}},
				{{1, 0}, {1, 1}, {1, 2}, {0, 1}}
			},
			{ // L
				{{0, 1}, {1, 1}, {2, 1}, {2, 2}},
				{{1, 0}, {1, 1}, {1, 2}, {2, 0}},
				{{0, 0}, {0, 1}, {1, 1}, {2, 1}},
				{{0, 2}, {1, 2}, {1, 1}, {1, 0}}
			},
			{ // J
				{{0, 2}, {0, 1}, {1, 1}, {2, 1}},
				{{1, 0}, {1, 1}, {1, 2}, {2, 2}},
				{{0, 1}, {1, 1}, {2, 1}, {2, 0}},
				{{0, 0}, {1, 0}, {1, 1}, {1, 2}}
			}
		};

		const Pos startPositions[7] = {{3, 1}, {4, 1}, {3, 1}, {3, 1}, {3, 2}, {3, 2}, {3, 2}};
		const Pos nextPositions[7] = {{6, 0}, {8, 1}, {7, 1}, {7, 1}, {7, 0}, {7, 0}, {7, 0}};

		Board *board;

		tetradType type;

		Pos pos;
		int orient = 0;

	public:
		static Tetrad *createRandom(Board *b) {
			return new Tetrad(b, (tetradType) random(7));
		}

		Tetrad(Board *b, tetradType t) : board(b), type(t) {
			pos.x = startPositions[t].x;
			pos.y = ROWS - 1 - startPositions[t].y;
		}

		void draw() {
			draw(pos, orient);
		}

		void draw(Pos p, int o) {
			for (int b = 0; b < 4; b++) {
				board->drawBlock(p + layouts[type][o][b], type);
			}
		}

		void drawNext() {
			Pos nextPos;
			nextPos.x = nextPositions[type].x;
			nextPos.y = ROWS + nextPositions[type].y;
			draw(nextPos, 0);
		}

		bool canMove(Pos newPos, int newOrient) {
			for (int b = 0; b < 4; b++) {
				if (!board->isPosAvailable(newPos + layouts[type][newOrient][b])) {
					return false;
				}
			}
		}

		bool canMoveDown() {
			Pos newPos = pos;
			newPos.y--;
			return canMove(newPos, orient);
		}

		bool moveLeft() {
			Pos newPos = pos;
			newPos.x--;
			if (!canMove(newPos, orient)) return false;
			pos = newPos;
			return true;
		}

		bool moveRight() {
			Pos newPos = pos;
			newPos.x++;
			if (!canMove(newPos, orient)) return false;
			pos = newPos;
			return true;
		}

		bool moveDown() {
			Pos newPos = pos;
			newPos.y--;
			if (!canMove(newPos, orient)) return false;
			pos = newPos;
			return true;
		}

		bool rotateLeft() {
			int newOrient = (orient == 0) ? 3 : orient - 1;
			if (!canMove(pos, newOrient)) return false;
			orient = newOrient;
			return true;
		}

		bool rotateRight() {
			int newOrient = (orient == 3) ? 0 : orient + 1;
			if (!canMove(pos, newOrient)) return false;
			orient = newOrient;
			return true;
		}

		void crystalise() {
			for (int b = 0; b < 4; b++) {
				board->setPos(pos + layouts[type][orient][b], type);
			}
		}
	};

	Board *board = 0;
	Tetrad *current = 0;
	Tetrad *next = 0;

	int score, level, lines;

	void setLevel(int l) {
		level = l;
		lines = 0;

		// Levels assumed to begin at 0.

		const float tick = 1000.0/59.73;
		const float a    = tick * (5.0/40.0);
		const float b    = tick * -5.0;
		const float c    = tick * 53.0;
		const int max_level = 20;
		const int fast   = 3;

		if (l >= max_level) {
			period = fast;
		}
		else {
			// Levels are modelled with a simple quadratic
			period = nearbyint(l*l*a + l*b + c);
		}
	}

	void drawAll() {
		GLCD.ClearScreen();

		GLCD.SelectFont(System5x7);

		for (int i = 0; i < 7; i++) {
			GLCD.CursorToXY(58, i * 8);
			GLCD.PutChar("EMFTRIS"[i]);
		}

		GLCD.CursorToXY(53, 120);
		GLCD.PrintNumber(level);
		GLCD.CursorToXY(0, 0);
		GLCD.PrintNumber(score);

		board->draw();
		current->draw();
		next->drawNext();

		GLCD.Display();
	}

	void newGame() {
		GLCD.ClearScreen();
		GLCD.Display();

		if (board) delete board;
		board = new Board;

		if (current) delete current;
		current = Tetrad::createRandom(board);

		if (next) delete next;
		next = Tetrad::createRandom(board);

		score = 0;
		setLevel(0);

		drawAll();
	}

public:
	void init() {
		debug::log("I know you're going to dig this");
		newGame();
	}

	void loop() {
		if (!current->moveDown()) {
			current->crystalise();
			delete current;
			current = next;
			next = Tetrad::createRandom(board);
			if (!current->canMoveDown()) newGame();
		}

		int cleared = board->clearLines();
		if (cleared) {
			score += score_multipliers[cleared - 1] * (level + 1);
			lines += cleared;
			if (lines >= 10) setLevel(level + 1);
		}

		drawAll();
	}

	void handleButton(Button b) {
		switch (b) {
		case A:
			current->rotateLeft();
			break;
		case B:
			current->rotateRight();
			break;
		case LEFT:
			current->moveLeft();
			break;
		case RIGHT:
			current->moveRight();
			break;
		case DOWN:
			while (current->moveDown());
			break;
		}
		drawAll();
	}
};

App* TetrisApp::New() {
	return new TetrisApp;
}

TetrisApp::TetrisApp() {
	game = new Tetris;
}

String TetrisApp::getName() const {
	return "Tetris";
}
