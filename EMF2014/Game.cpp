/*
 TiLDA Mk2

 Base class for games

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

#include "glcd.h"
#include "Tilda.h"
#include "Game.h"

GameApp::~GameApp() {
	delete game;
	delete bs;
}

void GameApp::task() {
	TickType_t lw = xTaskGetTickCount();
	bs = Tilda::createButtonSubscription(A | B | UP | DOWN | LEFT | RIGHT);

	Tilda::getLCDTask().disable();
	Tilda::getGUITask().clearRoot();
	GLCD.SetRotation(ROTATION_90);

	game->init();

	while (true) {
		Button b;
		while ((b = bs->waitForPress((TickType_t) 0))) {
			game->handleButton(b);
		}

		if (!game->paused) game->loop();

		vTaskDelayUntil(&lw, game->period / portTICK_PERIOD_MS);
	}
}

void GameApp::afterSuspension() {
	Tilda::getLCDTask().enable();
}

void GameApp::beforeResume() {
        if (bs) bs->clear();

	Tilda::getLCDTask().disable();
	Tilda::getGUITask().clearRoot();
	GLCD.SetRotation(ROTATION_90);
}
