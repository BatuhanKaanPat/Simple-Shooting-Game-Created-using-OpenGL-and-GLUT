Game Features:
Start/Pause Game: Press F1 to start or pause the game.
Time Limit: The game lasts for a specific duration (20 seconds), with a countdown displayed on the screen.
Bullet Cooldown: There is a cooldown between shots to prevent firing too quickly.

Target Interaction:
Collision Detection: Targets can be hit by the player's cannonball, with each hit granting 5 points.
Target Respawn: Targets reset after being hit, and their position is randomized within the window.
Multiple Targets: The game spawns several targets that move across the screen.
Exit Game: Press ESC to exit the game.
Scoring: Points are calculated based on hits, and the total score is displayed.
Movement: The player can move the cannon left and right using the A/D keys or arrow keys.
Game Over: When the game time ends, a prompt appears to restart the game by pressing F1.

Main Components:
Cannon: Controlled by the player, it shoots bullets to hit the targets.
Targets (Rockets): They move across the screen and respawn when hit. The target's position is randomized to avoid overlapping.
Bullet: The player fires a cannonball that travels across the screen and can hit targets.

Key OpenGL Functions:
glBegin() / glEnd(): These functions are used to draw various shapes, like the cannon, bullet, and targets.
glColor3f(): This sets the color of the objects being drawn.
vprint(): This function is used to display text (such as score, time, and instructions) on the screen.

Control Logic:
Keyboard Controls:
A/D or arrow keys to move the cannon left and right.
F1 to start or pause the game.
ESC to exit the game.
The code initializes the game window, targets, and player cannon, and implements the logic for firing bullets, scoring, and moving the cannon. It uses OpenGL for rendering the game elements.
