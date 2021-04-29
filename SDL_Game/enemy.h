#pragma once

#include <SDL.h>
#include <SDL_image.h>

#include <iostream>
using namespace std;

class Enemy
{
	public:
		bool active;

		SDL_Texture* texture;

		SDL_Rect posRect;

		float xDir, yDir;

		float speed;

		float pos_X, pos_Y;

		Enemy(SDL_Renderer* renderer);

		void Update(float deltaTime);

		void Draw(SDL_Renderer* renderer);

		void Reset();

		~Enemy();
};