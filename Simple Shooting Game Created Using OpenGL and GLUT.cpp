/*
-----------------------------------
ADDITIONAL FEATURES :
    -F1 to start or pause the game
    -Program calculates total shots
    -Program calculates total points according to each collision (each collision 5 points)
    -Program calculates total hits
    -When you press ESC you exit the game
**********************************/
#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define D2R 0.017453292
#define TARGET_COUNT 5
#define GAME_DURATION 20 // in seconds
#define PI 3.14

// Global variables for Template File
int windowWidth = 800;
int windowHeight = 600; 

int totalShots = 0;

int bulletCooldown = 1; // Cooldown duration in milliseconds
int lastShotTime = 0; // Time stamp of the last shot

int remainingSeconds = GAME_DURATION; // Remaining time in seconds
int remainingMilliseconds = 0; // Remaining time in milliseconds
int gameTime = 0;

int RESET_DELAY = 1000; // Miliseconds after a target being respawned

int targetHeight = 80; // The height of the target
int targetRadius = 20; // The radius of the target

int targetSpeed = 2;
int targetDirection = 1;

int weaponWidth = 60;
int weaponHeight = 20;
int weaponSpeed = 15;
int weaponX = 0; // Adjust the initial X position of the weapon to the left edge
int weaponY = windowHeight / 2 - weaponHeight / 2; // Center the weapon vertically
float targetWeaponY = weaponY; // Target position for the weapon

int bulletWidth = 7; // Adjust bullet width as needed
int bulletX = -1;
int bulletY = -1;
int bulletSpeed = 20;
bool bulletFired = false;

int paused = 0;

typedef struct {
    int x;
    int y;
    int hit;
    int speed;
} rocket_t;

rocket_t targets[TARGET_COUNT];

int totalTargetsHit = 0; // Total number of targets hit
int totalPoints = 0; // Total points scored

// Display text with variables
void vprint(int x, int y, void* font, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
        glutBitmapCharacter(font, str[i]);
}

void resetTarget(int targetIndex) {
    // Reset the hit state of the target after the delay
    targets[targetIndex].hit = false;
    // Reset target position to the bottom of the screen
    targets[targetIndex].x = 10 + rand() % 380; // Adjust x-coordinate as needed
    targets[targetIndex].y = -25; // Move the target to the bottom
}

void initRockets() {
    srand(time(NULL));
    int minDistance = 3 * targetRadius; // Minimum distance between targets

    for (int i = 0; i < TARGET_COUNT; i++) {
        // Randomly generate x-coordinate between (10, 350)
        targets[i].x = 10 + rand() % 350;

        // Randomly generate y-coordinate within the gaming area, adjusted to start before the window
        targets[i].y = rand() % (windowHeight - 29 * targetRadius);

        // Ensure that targets are not spawned too close to each other
        bool collisionDetected = false;
        for (int j = 0; j < i; j++) {
            int dx = abs(targets[i].x - targets[j].x);
            int dy = abs(targets[i].y - targets[j].y);
            if (dx < minDistance && dy < minDistance) {
                collisionDetected = true;
                break;
            }
        }

        // If collision is detected, regenerate the coordinates until no collision occurs
        while (collisionDetected) {
            targets[i].x = 200 + rand() % 300;
            targets[i].y = rand() % (600 - 2 * targetRadius);
            collisionDetected = false;
            for (int j = 0; j < i; j++) {
                int dx = abs(targets[i].x - targets[j].x);
                int dy = abs(targets[i].y - targets[j].y);
                if (dx < minDistance && dy < minDistance) {
                    collisionDetected = true;
                    break;
                }
            }
        }

        targets[i].hit = false;
    }
}

void drawCannonBullet() {
    if (bulletFired) {
        glColor3f(0.3, 0.3, 0.3); // Gray color for the cannonball
        int numSegments = 30; // Number of segments to approximate a circle
        float radius = 5.0; // Radius of the bullet circle

        glBegin(GL_POLYGON);
        for (int i = 0; i < numSegments; i++) {
            float theta = 2.0f * PI * float(i) / float(numSegments); // Calculate angle for each segment
            float x = radius * cosf(theta); // Calculate x-coordinate
            float y = radius * sinf(theta); // Calculate y-coordinate
            glVertex2f(bulletX + x, bulletY + y);
        }
        glEnd();
    }
}

void drawRockets() {
    for (int i = 0; i < TARGET_COUNT; i++) {
        if (!targets[i].hit) {
            // Draw rocket body
            glColor3f(0.5, 0.5, 0.5); // Grey color for rocket body
            glBegin(GL_POLYGON);
            glVertex2f(targets[i].x - targetRadius, targets[i].y - targetHeight / 2); // Bottom-left
            glVertex2f(targets[i].x + targetRadius, targets[i].y - targetHeight / 2); // Bottom-right
            glVertex2f(targets[i].x + targetRadius + 10, targets[i].y); // Tail-right
            glVertex2f(targets[i].x + targetRadius, targets[i].y + targetHeight / 2); // Top-right
            glVertex2f(targets[i].x - targetRadius, targets[i].y + targetHeight / 2); // Top-left
            glVertex2f(targets[i].x - targetRadius - 10, targets[i].y); // Tail-left
            glEnd();

            // Draw rocket tips
            glColor3f(1.0, 1.0, 1.0); // White color for rocket tips
            glBegin(GL_TRIANGLES);
            glVertex2f(targets[i].x - targetRadius, targets[i].y + targetHeight / 2); // Bottom tip
            glVertex2f(targets[i].x, targets[i].y + targetHeight + 10); // Top tip, adjusted y-coordinate to make it slightly higher
            glVertex2f(targets[i].x + targetRadius, targets[i].y + targetHeight / 2); // Bottom tip
            glEnd();

            // Draw flames behind the rocket
            glColor3f(1.0, 0.5, 0.0); // Orange color for flames
            glBegin(GL_POLYGON);
            glVertex2f(targets[i].x - targetRadius - 10, targets[i].y - targetHeight / 2); // Left flame base
            glVertex2f(targets[i].x - targetRadius - 15, targets[i].y - targetHeight / 2 - 30); // Bottom-left corner
            glVertex2f(targets[i].x - targetRadius , targets[i].y - targetHeight / 2 - 15); // Bottom-left corner
            glVertex2f(targets[i].x , targets[i].y - targetHeight - 20); // Center point
            glVertex2f(targets[i].x + targetRadius + 10, targets[i].y - targetHeight / 2); // Right flame base
            glVertex2f(targets[i].x + targetRadius + 15, targets[i].y - targetHeight / 2 - 30); // Bottom-right corner
            glEnd();
        }
    }
}

void drawCannon() {
    // Cannon body
    glColor3f(0.1, 0.1, 0.1); // Dark gray color for the cannon body
    glBegin(GL_POLYGON);
    glVertex2f((weaponX), (windowHeight - weaponHeight));
    glVertex2f((weaponX + weaponWidth), (windowHeight - weaponHeight));
    glVertex2f((weaponX + weaponWidth), (windowHeight - weaponHeight + 10));
    glVertex2f((weaponX), (windowHeight - weaponHeight + 10));
    glEnd();

    // Cannon barrel
    glColor3f(0.2, 0.2, 0.2); // Gray color for the cannon barrel
    glBegin(GL_POLYGON);
    glVertex2f((weaponX + weaponWidth / 2 - 10), (windowHeight - weaponHeight + 10));
    glVertex2f((weaponX + weaponWidth / 2 + 10), (windowHeight - weaponHeight + 10));
    glVertex2f((weaponX + weaponWidth / 2 + 10),(windowHeight - weaponHeight - 50));
    glVertex2f((weaponX + weaponWidth / 2 - 10), (windowHeight - weaponHeight - 50));
    glEnd();

    // Cannon details
    glColor3f(0.3, 0.3, 0.3); // Light gray color for cannon details
    glLineWidth(2.0); // Set line width for details
    glBegin(GL_LINES);
    // Cannon body details
    glVertex2f((weaponX + 5), (windowHeight - weaponHeight + 5));
    glVertex2f((weaponX + 5), (windowHeight - 5));
    glVertex2f((weaponX + weaponWidth - 5), (windowHeight - weaponHeight + 5));
    glVertex2f((weaponX + weaponWidth - 5), (windowHeight - 5));
    // Cannon barrel details
    glVertex2f(weaponX + weaponWidth / 2 - 5, windowHeight - weaponHeight + 15);
    glVertex2f(weaponX + weaponWidth / 2 - 5, windowHeight - weaponHeight - 35);
    glVertex2f(weaponX + weaponWidth / 2 + 5, windowHeight - weaponHeight + 15);
    glVertex2f(weaponX + weaponWidth / 2 + 5,windowHeight - weaponHeight - 35);
    glEnd();
}

void display() 
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw gradient background
    glBegin(GL_QUADS);
    // Top color (light blue)
    glColor3f(0.53, 0.81, 0.98); // Sky blue color
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    // Bottom color (dark blue)
    glColor3f(0.0, 0.0, 0.1); // Dark blue color
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();

    // Draw targets, weapon, and bullet
    drawRockets();
    drawCannon();
    drawCannonBullet();

    // Draw black side panel
    glBegin(GL_QUADS);
    glColor3f(0.0, 0.0, 0.0); // Black color for the panel
    glVertex2f(windowWidth - 200, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(windowWidth - 200, windowHeight);
    glEnd();

    // Draw border around the black side panel
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0, 1.0, 1.0); // White color for the border
    glVertex2f(windowWidth - 200, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(windowWidth - 200, windowHeight);
    glEnd();

    //After the game finishes it displays a text
    if (remainingSeconds <= 0 && remainingMilliseconds <= 0) {
        glColor3f(1.0, 0, 0);
        vprint(windowWidth - 650, 300, GLUT_BITMAP_TIMES_ROMAN_24, "Press F1 to start a new game");
        glEnd();
    }

    //Game Title
    vprint(windowWidth - 750,500, GLUT_BITMAP_8_BY_13, "SHOOT AT THE ROCKETS!!!");

    //For the movements
    vprint(windowWidth - 180, 450, GLUT_BITMAP_8_BY_13, "Movements:");
    vprint(windowWidth - 180, 430, GLUT_BITMAP_8_BY_13, "A or D buttons");
    vprint(windowWidth - 180, 410, GLUT_BITMAP_8_BY_13, "Left or Right Arrow");

    //For the timer
    vprint(windowWidth-180, 150, GLUT_BITMAP_TIMES_ROMAN_24, "Time: %d.%d \n", remainingSeconds, remainingMilliseconds);

    //For the hits
    vprint(windowWidth - 180, 200, GLUT_BITMAP_TIMES_ROMAN_24, "Targets Hit: %d", totalTargetsHit);

    //For the points
    vprint(windowWidth - 180, 250, GLUT_BITMAP_TIMES_ROMAN_24, "Total Points: %d", totalPoints);

    //For the total shots made
    vprint(windowWidth - 180, 300, GLUT_BITMAP_TIMES_ROMAN_24, "Total Shots: %d", totalShots);
    
    vprint(windowWidth - 180, 500, GLUT_BITMAP_8_BY_13, "<Spacebar> Fire");
    
    vprint(windowWidth - 180, 525, GLUT_BITMAP_8_BY_13, "<F1> Pause");

    vprint(windowWidth - 180, 550, GLUT_BITMAP_8_BY_13, "<ESC> EXIT");

    glutSwapBuffers();

}


void onTimer(int value) {
    if (!paused) {
        
        // Move rockets upwards
        for (int i = 0; i < TARGET_COUNT; i++) {
            targets[i].y += targetSpeed;
            if (targets[i].y > windowHeight) {
                // Reset target position within the desired x-coordinate range
                targets[i].x = 200 + rand() % (windowWidth - 400); // Adjusted to fit the screen width
                targets[i].y = -targetHeight; // Move targets to the top
            }
        }

        // Update remaining time only if the game is not paused
        if (remainingSeconds > 0 || remainingMilliseconds > 0) {
            // Update remaining time
            remainingMilliseconds -= 10;
            if (remainingMilliseconds < 0) {
                remainingMilliseconds += 1000;
                remainingSeconds--;
            }
        }


        // Update cannonball position 
        if (bulletFired) {
            bulletY -= bulletSpeed; // Move the bullet downwards
            if (bulletY < 0) {
                bulletFired = false; // Reset bullet if it goes out of bounds
            }
        }

        // Collision detection between cannonball and targets
        if (bulletFired) {
            for (int i = 0; i < TARGET_COUNT; i++) {
                if (!targets[i].hit &&
                    bulletX >= targets[i].x - targetRadius &&
                    bulletX <= targets[i].x + targetRadius &&
                    bulletY >= targets[i].y - targetHeight / 2 &&
                    bulletY <= targets[i].y + targetHeight / 2) {
                    // Cannonball hits the target
                    targets[i].hit = true;
                    totalTargetsHit++; // Increment total targets hit
                    totalPoints += 5; // Increment total points 5 after collision
                    glutTimerFunc(RESET_DELAY, resetTarget, i);
                    bulletFired = false; // Reset bullet
                    break; // Exit loop after hitting one target
                }
            }
        }

        if (remainingSeconds <= 0 && remainingMilliseconds <= 0) {
            remainingSeconds = 0;
            paused = 1;
        }

        glutPostRedisplay();
    }

    glutTimerFunc(GAME_DURATION, onTimer, 0);
}

void updateWeapon() {

    float deltaY = targetWeaponY - weaponY;
    if (fabs(deltaY) > 0.1) {
        weaponY += deltaY * 0.1;
    }
}

// Key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
void onKeyDown(unsigned char key, int x, int y) {
    switch (key) {
        // Exit when ESC is pressed.
    case 27:
        exit(0);
        break;
    case 32: // Spacebar key
        if (!bulletFired) {
            // Calculate the initial position of the bullet based on the position of the cannon's barrel
            bulletX = weaponX + weaponWidth / 2; // Set bulletX to the center of the cannon's barrel
            bulletY = windowHeight - weaponHeight; // Set bulletY to the top of the cannon's barrel
            bulletFired = true;
            lastShotTime = glutGet(GLUT_ELAPSED_TIME);
            totalShots++;
        }
        break;
    case 'a':
    case 'A':
        // Move the gun to the left
        if (weaponX > 0) { // Check if the gun is already at the left boundary
            weaponX -= weaponSpeed;
        }
        break;
    case 'd':
    case 'D':
        // Move the gun to the right
        if (weaponX + weaponWidth < windowWidth - 200) { // Adjusted boundary considering the black side
            weaponX += weaponSpeed;
        }
        break;
    }
}

// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
void onSpecialKeyDown(int key, int x, int y) {
    switch (key) {
        // Move the gun to the left
    case GLUT_KEY_LEFT:
        if (weaponX > 0) {
            weaponX -= weaponSpeed;
        }
        break;
        // Move the gun to the right
    case GLUT_KEY_RIGHT:
        if (weaponX + weaponWidth < windowWidth-200) {
            weaponX += weaponSpeed;
        }
        break;
    case GLUT_KEY_F1://F1 key
        if (remainingSeconds > 0 || remainingMilliseconds > 0) {

            paused = !paused;
        }
        else {
            // If the game is over, restart the game
            initRockets();
            remainingSeconds = GAME_DURATION;
            remainingMilliseconds = 0;
            paused = 0;
            totalTargetsHit = 0;
            totalPoints = 0;
            totalShots = 0;
        }
        break;
    }
}


// This function is called when the window size changes.
// w : is the new width of the window in pixels
// h : is the new height of the window in pixels
void onResize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    windowWidth = w;
    windowHeight = h;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Shoot the rockets");
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Initialize targets before entering the main loop
    initRockets();
    remainingSeconds = 0;//Initializing seconds to press F1 to start the game

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);
    glutTimerFunc(GAME_DURATION, onTimer, 0);

    glutMainLoop();
    return 0;
}
