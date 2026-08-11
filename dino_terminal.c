//Create the Dinosaur game from google when no connection.
//In Part A which is a C file it will run through the terminal
//See and understand how make it work

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 7
#define MAX_OBSTACLES 3

typedef struct {
    int x;
    int type;
    bool active;
} Obstacle;

// Non-blocking keyboard hit detection for macOS terminal
int kbhit(void)
{
    struct termios old_sett, new_sett;
    int ch, oldf;

    tcgetattr(STDIN_FILENO, &old_sett);
    new_sett = old_sett;
    new_sett.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_sett);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &old_sett);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int getch(void) 
{
    return getchar();
}

// Get the total horizontal width of an obstacle type
int getObstacleWidth(int type) {
    switch (type) {
        case 0: return 1; // X
        case 1: return 2; // XX
        case 2: return 3; // X / XXX (2D Cactus)
        case 3: return 1; // Flying Bird
        default: return 1;
    }
}

// Check if a SINGLE obstacle occupies grid coordinate (x, y)
bool isObstacleAt(int x, int y, Obstacle obs) {
    if (!obs.active) return false;
    int relX = x - obs.x;

    switch (obs.type) {
        case 0: // Small Ground: X
            return (y == 0 && relX == 0);

        case 1: // Medium Ground: XX
            return (y == 0 && (relX == 0 || relX == 1));

        case 2: // 2D Cactus:
                //   X    (y = 1, relX = 1)
                //  XXX   (y = 0, relX = 0, 1, 2)
            if (y == 1 && relX == 1) return true;
            if (y == 0 && relX >= 0 && relX <= 2) return true;
            return false;

        case 3: // Flying Bird: X at y = 1
            return (y == 1 && relX == 0);

        default:
            return false;
    }
}

bool isAnyObstacleAt(int x, int y, Obstacle obstacles[])
{
    for(int i = 0; i < MAX_OBSTACLES; i++)
    {
        if(isObstacleAt(x, y, obstacles[i]))
        {
            return true;
        }
    }
    return false;
}

int main(void) 
{
    srand((unsigned int)time(NULL));

    int dinoY = 0;
    int dinoVel = 0;
    bool isJumping = false;

    //int obstacleX = WIDTH - 2;
    int dinoX = 4;
    int score = 0;
    bool gameOver = false;

    //Array of active/inactive
    Obstacle obstacles[MAX_OBSTACLES];
    for(int i = 0; i < MAX_OBSTACLES; i++)
    {
        obstacles[i].active = false;
    }

    //control Gap size
    int nextSpawnGap = (rand() % 16) + 7;

    //Obstacle Properties
    int obstacleWidth = 1; //Default X
    int obstacleY = 0;  // 0 = grounf, 1 or 2 is obstacle in the air

    // Clear terminal screen
    printf("\033[2J");
    printf("=== DINO JUMP (macOS) ===\nPress SPACE to jump, 'q' to quit.\nPress SPACE to start...\n");

    while(1)
    {
        if(kbhit() && getch() == ' ')
        {
            break;
        }
    }

    while(!gameOver)
    {
        //USER INPUT - space, q, and r
        if(kbhit())
        {
            char key = getch();
            if(key == ' ' && !isJumping)
            {
                isJumping = true;
                dinoVel = 2.5;    //Jump Height
            }
            else if(key == 'q')
            {
                //Quit the Game
                break;
            }
        }

        //Change the Physics for the Jump
        if(isJumping)
        {
            dinoY += dinoVel;
            dinoVel--; // GRAVITY
            if(dinoY <= 0)
            {
                dinoY = 0;
                isJumping = false;
            }
        }

        int rightmostX = -100;
        for(int i = 0; i < MAX_OBSTACLES; i++)
        {
            if(obstacles[i].active)
            {
                obstacles[i].x--;

                if(obstacles[i].x > rightmostX)
                {
                    rightmostX = obstacles[i].x;
                }
                
                int width = getObstacleWidth(obstacles[i].type);
                if(obstacles[i].x + width < 0)
                {
                    obstacles[i].active = false;
                    score++;
                }

            }
        }

        //Random Respawn Logic
        if((WIDTH - 1) - rightmostX >= nextSpawnGap)
        {
            for(int i = 0; i < MAX_OBSTACLES; i++)
            {
                if(!obstacles[i].active)
                {
                    obstacles[i].active = true;
                    obstacles[i].x = WIDTH - 1;
                    obstacles[i].type = rand() % 4;

                    nextSpawnGap = (rand() % 16) + 8;// + 2;
                    break;
                }
            }
        }

        // 5. MULTI-OBSTACLE COLLISION CHECK
        if (isAnyObstacleAt(dinoX, dinoY, obstacles)) {
            gameOver = true;
        }


        //Speed up after every 3 points
        int speedLevel = score / 3;
        int frameDelay = 70000 - (speedLevel * 7000);
        if(frameDelay < 20000)
        {
            frameDelay = 20000;
        }

        // Render Frame (moves cursor to top-left to avoid flicker)
        printf("\033[H");
        printf("Score: %d | Level: %d (Delay: %dms)           \n", score, speedLevel + 1, frameDelay / 1000);

        for(int i = HEIGHT - 1; i >= 0; i--)
        {
            for(int x = 0; x < WIDTH; x++)
            {
                if(x == dinoX && dinoY == i)
                {
                    printf("D");    //Dino
                }
                else if(isAnyObstacleAt(x, i, obstacles))
                {
                    printf("X");
                }
                else
                {
                    printf(" ");
                }
            }
            printf("\n");
        }
        for(int i = 0; i < WIDTH; i++)
        {
            printf("=");
        }
        printf("\n");

        usleep(frameDelay);  // 14 FPS update rate

    }
    printf("\nGAME OVER! Final Score: %d\n", score);
    return 0;
}


