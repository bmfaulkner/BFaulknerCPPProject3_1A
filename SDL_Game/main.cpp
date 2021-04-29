#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

//Bool to maintain program loop
bool quit = false;

//DeltaTime init() - for frame rate independence
float deltaTime = 0.0;
int thisTime = 0;
int lastTime = 0;

//Set player speed
float playerSpeed = 500.0f;

//Create rectangle for the player pos
SDL_Rect playerPos;

//Movement in the y axis - up or down
float yDir;

//Variables used to control integer creep 
float pos_X, pos_Y;

//Enemy information
#include "enemy.h"
#include <vector>

//Enemy list variables
vector<Enemy> enemyList;

//Bullet Information
#include "bullet.h"
vector<Bullet> bulletList;

//Declare sounds
Mix_Chunk* laser;
Mix_Chunk* explosion;
Mix_Music* bgm;

void CreateBullet()
{
    for (int i = 0; i < bulletList.size(); i++)
    {
        if (bulletList[i].active == false)
        {
            //Play laser sound
            Mix_PlayChannel(-1, laser, 0);

            bulletList[i].active = true;

            bulletList[i].posRect.y = (pos_Y + (playerPos.h / 2));

            bulletList[i].posRect.y = (bulletList[i].posRect.y - (bulletList[i].posRect.h / 2));

            bulletList[i].posRect.x = playerPos.x;

            bulletList[i].pos_Y = pos_Y;

            bulletList[i].pos_X = playerPos.x;

            break;
        }
    }
}

//Variables for score and lives
//SCore, lives and fonts
int playerScore, oldScore, playerLives, oldLives;

//Desclare font
TTF_Font* font;

//Font color
SDL_Color colorP1 = { 255,255,255,255};

//Create Surfaces
SDL_Surface* scoreSurface, * livesSurface;

//Create textures
SDL_Texture* scoreTexture, * livesTexture;

SDL_Rect scorePos, livesPos;

string tempScore, tempLives;

void UpdateScore(SDL_Renderer* renderer)
{
    //Creating the text
    tempScore = "Player Score: " + std::to_string(playerScore);

    //Create render text
    scoreSurface = TTF_RenderText_Solid(font, tempScore.c_str(), colorP1);

    //Creating texture
    scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scorePos.w, &scorePos.h);

    SDL_FreeSurface(scoreSurface);

    oldScore = playerScore;
}

void UpdateLives(SDL_Renderer* renderer)
{
    //Creating the text
    tempLives = "Player Lives: " + std::to_string(playerLives);

    //Create render text
    livesSurface = TTF_RenderText_Solid(font, tempLives.c_str(), colorP1);

    //Creating texture
    livesTexture = SDL_CreateTextureFromSurface(renderer, livesSurface);

    SDL_QueryTexture(livesTexture, NULL, NULL, &livesPos.w, &livesPos.h);

    SDL_FreeSurface(livesSurface);

    oldLives = playerLives;
}

int main(int argc, char* argv[])
{
    //Declare a pointer
    SDL_Window* window;

    //Create a renderer variable
    SDL_Renderer* renderer = NULL;

    //Initialize SDL2
    SDL_Init(SDL_INIT_EVERYTHING);

    //Create an application window with the following settings:
    window = SDL_CreateWindow
    (
        //Window Title
        "Space War",

        //Initial x position
        SDL_WINDOWPOS_UNDEFINED,

        //Intital y position,
        SDL_WINDOWPOS_UNDEFINED,

        //Width in pixels
        1024,

        //Height in pixels
        768,

        //Flags - See Below
        SDL_WINDOW_OPENGL
    );

    //Check that the window was successfully created
    if (window == NULL)
    {
        //In the case that the window could not be made
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    //Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Background image -- CREATE

    //Create a SDL Surface
    SDL_Surface* surface = IMG_Load("./Assets/bkgd.png");

    //Create bkgd texture
    SDL_Texture* bkgd;

    //Place surface into the texture
    bkgd = SDL_CreateTextureFromSurface(renderer, surface);

    //Free the surface
    SDL_FreeSurface(surface);

    //Create rectangle for the background position
    SDL_Rect bkgdPos;

    //Set bkgdPos x, y , width and height
    bkgdPos.x = 0;
    bkgdPos.y = 0;
    bkgdPos.w = 1024;
    bkgdPos.h = 768;

    //Background image -- CREATE END

    //Player image -- CREATE

    //Create a SDL surface
    surface = IMG_Load("./Assets/player.png");

    //Create bkgd texture
    SDL_Texture* player;

    //Place surface into the texture
    player = SDL_CreateTextureFromSurface(renderer, surface);

    //Free the surface
    SDL_FreeSurface(surface);

    //Set playerPos x, y , width and height
    playerPos.x = 0;
    playerPos.y = 384;
    playerPos.w = 125;
    playerPos.h = 75;

    //Player image -- CREATE END

    //SDL Event to handle input
    SDL_Event event;

    //Create bullets and place in list
    for (int i = 0; i < 10; i++)
    {
        Bullet tempBullet(renderer, i + 5, i + 5);

        bulletList.push_back(tempBullet);
    }

    //Clear enemy list
    enemyList.clear();

    //Fill enemy list
    for (int i = 0; i < 8; i++)
    {
        Enemy tempEnemy(renderer);

        enemyList.push_back(tempEnemy);
    }

    //Initialize audio playback
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    //Load laser sound
    laser = Mix_LoadWAV("./Assets/projectile.wav");

    //Load explosion sound
    explosion = Mix_LoadWAV("./Assets/explosion.wav");

    //Load and loop backgorund music
    bgm = Mix_LoadMUS("./Assets/backgroundLoop.wav");

    //If the music is not playing, play it and loop
    if (!Mix_PlayingMusic())
    {
        Mix_PlayMusic(bgm, -1);
    }

    //Initialize player score, lives and font
    oldScore = 0;
    playerScore = 0;
    oldLives = 0;
    playerLives = 5;

    TTF_Init();
    font = TTF_OpenFont("./Assets/Cosmic Blaster.TTF", 20);

    //Set up rectangles
    scorePos.x = scorePos.y = 10;
    livesPos.x = 10;
    livesPos.y = 40;

    UpdateScore(renderer);
    UpdateLives(renderer);

    //Set up our enum to handle all the game states
    enum GameState {GAME, WIN, LOSE };

    //Variable to track where we are in the game
    GameState gameState = GAME;

    //Bool value to allow movement through the individual states
    bool game, win, lose;

    //Basic program loop
    while (!quit)
    {
        switch (gameState)
        {        
        case GAME:
        {
            game = true;

            std::cout << "The Game State is GAME" << endl;

            //Load instructions background
            //Create a SDL surface
            surface = IMG_Load("./Assets/bkgd.png");

            //Place surface into the texture
            bkgd = SDL_CreateTextureFromSurface(renderer, surface);

            //Free the surface 
            SDL_FreeSurface(surface);

            //Background Image -- Create END

            //Start the game fresh
            enemyList.clear();

            //Fill enemy list
            for (int i = 0; i < 8; i++)
            {
                Enemy tempEnemy(renderer);

                enemyList.push_back(tempEnemy);
            }

            //Reset player's score and lives
            playerScore = 0;
            playerLives = 5;

            while (game)
            {
                //Create deltaTime
                thisTime = SDL_GetTicks();
                deltaTime = (float)(thisTime - lastTime) / 1000;
                lastTime = thisTime;

                //Check for input
                if (SDL_PollEvent(&event))
                {
                    //Close window by the window's X button
                    if (event.type == SDL_QUIT)
                    {
                        quit = true;
                        game = false;
                        break;
                    }

                    switch (event.type)
                    {
                        //Look for a keypress
                    case SDL_KEYUP:

                        //Check the space bar
                        switch (event.key.keysym.sym)
                        {
                        case SDLK_SPACE:

                            CreateBullet();

                            break;
                        default:
                            break;
                        }

                        break;

                    default:
                        break;
                    }

                }

                //Player movement
                //Get the keyboard state
                const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

                //Check which arrows are pressed
                if (currentKeyStates[SDL_SCANCODE_UP])
                {
                    yDir = -1;
                }
                else if (currentKeyStates[SDL_SCANCODE_DOWN])
                {
                    yDir = 1;
                }
                else
                {
                    yDir = 0;
                }

                //UPDATE ------------------------------------------

                //Get new position to move player using direction and deltaTime
                pos_Y += (playerSpeed * yDir) * deltaTime;

                //Move player
                playerPos.y = (int)(pos_Y + 0.5f);

                //Keep player on screen
                //Moving up
                if (playerPos.y < 0)
                {
                    playerPos.y = 0;
                    pos_Y = 0;
                }

                //Moving down
                if (playerPos.y > 768 - playerPos.h)
                {
                    playerPos.y = 768 - playerPos.h;
                    pos_Y = 768 - playerPos.h;
                }

                //Update Bullets
                for (int i = 0; i < bulletList.size(); i++)
                {
                    if (bulletList[i].active == true)
                    {
                        bulletList[i].Update(deltaTime);
                    }
                }

                //Update enemy list
                for (int i = 0; i < 8; i++)
                {
                    enemyList[i].Update(deltaTime);
                }

                //Check for collision of bullets with enemies
                //For loop to scroll through all the player's bullets
                for (int i = 0; i < bulletList.size(); i++)
                {
                    //Check to see if this bullet is active in the world
                    if (bulletList[i].active == true)
                    {
                        //Check all anemies against active bullet
                        for (int j = 0; j < enemyList.size(); j++)
                        {
                            //See if there is a collision between this bullet and this enemy using SDL
                            if (SDL_HasIntersection(&bulletList[i].posRect, &enemyList[j].posRect))
                            {
                                //Play explosion sound
                                Mix_PlayChannel(-1, explosion, 0);

                                //Reset the enemy
                                enemyList[j].Reset();

                                //Reset the bullet
                                bulletList[i].Reset();

                                //Give player points
                                playerScore += 10;

                                //Check to see if the player won
                                if (playerScore >= 500)
                                {
                                    game = false;
                                    gameState = WIN;
                                }
                            }
                        }
                    }
                }

                //Check for colliusion with the player and all enemies
                //Check all enemies against active bullet
                for (int i = 0; i < enemyList.size(); i++)
                {
                    //See if there ios a collision between this bullet and this enemy using SDL
                    if (SDL_HasIntersection(&playerPos, &enemyList[i].posRect))
                    {
                        //Play explosion sound
                        Mix_PlayChannel(-1, explosion, 0);

                        //Reset the enemy
                        enemyList[i].Reset();

                        //Take player life away
                        playerLives -= 1;

                        //Check to see if the player loses
                        if (playerLives <= 0)
                        {
                            game = false;
                            gameState = LOSE;
                        }
                    }
                }

                //Update score and lives on screen text
                if (playerScore != oldScore)
                {
                    UpdateScore(renderer);
                }

                if (playerLives != oldLives)
                {
                    UpdateLives(renderer);
                }

                //END UPDATE --------------------------------------

                //DRAW --------------------------------------------

                //Clear the old buffer
                SDL_RenderClear(renderer);

                //Prepare to draw bkgd
                SDL_RenderCopy(renderer, bkgd, NULL, &bkgdPos);

                //Draw Bullets
                for (int i = 0; i < bulletList.size(); i++)
                {
                    if (bulletList[i].active == true)
                    {
                        bulletList[i].Draw(renderer);
                    }
                }

                //Prepare to draw player
                SDL_RenderCopy(renderer, player, NULL, &playerPos);

                //Draw enemy list
                for (int i = 0; i < 8; i++)
                {
                    enemyList[i].Draw(renderer);
                }

                //Draw score texture
                SDL_RenderCopy(renderer, scoreTexture, NULL, &scorePos);

                //Draw lives texture
                SDL_RenderCopy(renderer, livesTexture, NULL, &livesPos);

                //Draww new info to the screen
                SDL_RenderPresent(renderer);

                //END DRAW ----------------------------------------

            } // End Game Screen
            break;
        }
        case WIN:

            win = true;

            std::cout << "The Game State is WIN" << endl;
            std::cout << "Press the R key to Replay The Game" << endl;
            std::cout << "Press the Q key to Quit The Game" << endl;
            std::cout << endl;


            //Load win background
            //Create a SDL surface
            surface = IMG_Load("./Assets/win.png");

            //Place a surface into the texture
            bkgd = SDL_CreateTextureFromSurface(renderer, surface);

            //Free the surface
            SDL_FreeSurface(surface);

            //Background Image -- CREATE END
            while (win)
            {
                //Create DeltaTime
                thisTime = SDL_GetTicks();
                deltaTime = (float)(thisTime - lastTime) / 1000;
                lastTime = thisTime;

                //Check for input
                if (SDL_PollEvent(&event))
                {
                    //Close window by the window's X button
                    if (event.type == SDL_QUIT)
                    {
                        quit = true;
                        win = false;
                        break;
                    }

                    switch (event.type)
                    {
                        //Look for a Keypress
                    case SDL_KEYUP:

                        //Check the SDLKey values and move change the coordinates
                        switch (event.key.keysym.sym)
                        {
                        case SDLK_r:
                            win = false;
                            gameState = GAME;
                            break;
                        case SDLK_q:
                            quit = true;
                            win = false;
                            break;
                        }
                    }
                }

                //DRAW ***************************************************************

                //Draw Section
                //Clear the old buffer
                SDL_RenderClear(renderer);

                //Prepare to draw bkgd1
                SDL_RenderCopy(renderer, bkgd, NULL, &bkgdPos);

                //Draw new info to the screen
                SDL_RenderPresent(renderer);

                //END DRAW ***********************************************************

            } //End Win Screen
            break;

        case LOSE:

            lose = true;

            std::cout << "The Game State is LOSE" << endl;
            std::cout << "Press the R key to Replay The Game" << endl;
            std::cout << "Press the Q key to Quit The Game" << endl;
            std::cout << endl;


            //Load lose background
            //Create a SDL surface
            surface = IMG_Load("./Assets/lose.png");

            //Place a surface into the texture
            bkgd = SDL_CreateTextureFromSurface(renderer, surface);

            //Free the surface
            SDL_FreeSurface(surface);

            //Background Image -- CREATE END
            while (lose)
            {
                //Create DeltaTime
                thisTime = SDL_GetTicks();
                deltaTime = (float)(thisTime - lastTime) / 1000;
                lastTime = thisTime;

                //Check for input
                if (SDL_PollEvent(&event))
                {
                    //Close losedow by the losedow's X button
                    if (event.type == SDL_QUIT)
                    {
                        quit = true;
                        lose = false;
                        break;
                    }

                    switch (event.type)
                    {
                        //Look for a Keypress
                    case SDL_KEYUP:

                        //Check the SDLKey values and move change the coordinates
                        switch (event.key.keysym.sym)
                        {
                        case SDLK_r:
                            lose = false;
                            gameState = GAME;
                            break;
                        case SDLK_q:
                            quit = true;
                            lose = false;
                            break;
                        }
                    }
                }

                //DRAW ***************************************************************

                //Draw Section
                //Clear the old buffer
                SDL_RenderClear(renderer);

                //Prepare to draw bkgd1
                SDL_RenderCopy(renderer, bkgd, NULL, &bkgdPos);

                //Draw new info to the screen
                SDL_RenderPresent(renderer);

                //END DRAW ***********************************************************

            } //End Win Screen
            break;
        }// Ends Switch for gameState

    } //Ends Game Loop

    //Close and destroy the window
    SDL_DestroyWindow(window);

    //Clean up
    SDL_Quit();

    return 0;
}