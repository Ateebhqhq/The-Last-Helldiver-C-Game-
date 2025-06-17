#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>

// Window dimensions
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 700;

// Movement speeds for player, enemy, and bullet
const float PLAYER_SPEED = 3.0f;
const float ENEMY_SPEED = 1.0f;
const float BULLET_SPEED = 7.0f;

// Rate at which the safe zone shrinks
const float SAFE_ZONE_SHRINK_RATE = 0.1f;

// Cooldown time between player shots
const float BULLET_COOLDOWN = 0.3f;

// Base class representing any game entity with position, speed, health, and sprite
class Entity {
protected:
    sf::Sprite sprite;          // Visual representation
    sf::Vector2f velocity;      // Current movement velocity
    float speed;                // Movement speed
    int health;                 // Health points

public:
    // Constructor initializes position, texture, speed, and health
    Entity(float x, float y, const sf::Texture& texture, float speed, int health)
        : speed(speed), health(health) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
        // Set origin to center of texture for proper rotation and positioning
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    }

    // Move entity by its velocity vector
    void move() { sprite.move(velocity); }

    // Check if entity is alive (health > 0)
    bool isAlive() const { return health > 0; }

    // Reduce health by damage amount
    void takeDamage(int amount) { health -= amount; }

    // Get current position of entity
    sf::Vector2f getPosition() const { return sprite.getPosition(); }

    // Access the sprite for drawing
    sf::Sprite& getSprite() { return sprite; }

    // Get current health value
    int getHealth() const { return health; }
};

// Player class derived from Entity, handles player-specific input and behavior
class Player : public Entity {
public:
    // Initialize player at center of window with full health and player speed
    Player(const sf::Texture& texture) : Entity(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, texture, PLAYER_SPEED, 100) {}

    // Handle keyboard input to update velocity vector
    void handleInput() {
        velocity = {0, 0};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) velocity.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) velocity.y += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) velocity.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) velocity.x += speed;
    }
};

// Enemy class derived from Entity, moves towards the player
class Enemy : public Entity {
public:
    // Initialize enemy at given position with random speed variation and fixed health
    Enemy(float x, float y, const sf::Texture& texture)
        : Entity(x, y, texture, ENEMY_SPEED + static_cast<float>(rand() % 20) / 10.0f, 50) {}

    // Update velocity to move towards the player's position
    void update(const sf::Vector2f& playerPos) {
        sf::Vector2f direction = playerPos - sprite.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;  // Normalize direction vector
            velocity = direction * speed;
        }
    }
};

// Bullet class represents projectiles fired by the player
class Bullet {
private:
    sf::CircleShape shape;      // Visual representation as a circle
    sf::Vector2f velocity;      // Movement velocity
    float speed;                // Speed of bullet

public:
    // Initialize bullet at position with direction vector
    Bullet(float x, float y, float dirX, float dirY)
        : shape(5), speed(BULLET_SPEED) {
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(x, y);
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0) {
            velocity.x = (dirX / length) * speed;
            velocity.y = (dirY / length) * speed;
        }
    }

    // Move bullet by its velocity
    void move() { shape.move(velocity); }

    // Get current position of bullet
    sf::Vector2f getPosition() const { return shape.getPosition(); }

    // Get radius of bullet shape
    float getRadius() const { return shape.getRadius(); }

    // Access shape for drawing
    sf::CircleShape& getShape() { return shape; }
};

// SafeZone class represents the shrinking safe area in the game
class SafeZone {
private:
    sf::Sprite sprite;          // Visual representation of the safe zone
    float shrinkRate;           // Rate at which the zone shrinks
    float minRadius;            // Minimum radius the zone can shrink to
    float radius;               // Current radius of the safe zone

public:
    // Initialize safe zone with texture and starting radius
    SafeZone(const sf::Texture& texture)
        : shrinkRate(SAFE_ZONE_SHRINK_RATE), minRadius(50.0f) {
        sprite.setTexture(texture);
        radius = std::min(WINDOW_WIDTH, WINDOW_HEIGHT) * 0.5f;
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        float scaleFactor = radius / (texture.getSize().x / 2);
        sprite.setScale(scaleFactor, scaleFactor);
    }

    // Shrink the safe zone gradually until minimum radius is reached
    void update() {
        if (radius > minRadius) {
            radius -= shrinkRate;
            float scaleFactor = radius / (sprite.getTexture()->getSize().x / 2);
            sprite.setScale(scaleFactor, scaleFactor);
        }
    }

    // Check if a position is inside the safe zone radius
    bool isInside(const sf::Vector2f& position) const {
        sf::Vector2f center(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        float distance = std::sqrt(std::pow(position.x - center.x, 2) + std::pow(position.y - center.y, 2));
        return distance < radius;
    }

    // Access sprite for drawing
    sf::Sprite& getSprite() { return sprite; }
};

// Display the start screen with title, lore, and prompt to begin
void showStartScreen(sf::RenderWindow& window, sf::Font& font) {
    sf::Text title("THE LAST HELLDIVER", font, 48);
    title.setFillColor(sf::Color::Red);
    title.setStyle(sf::Text::Bold);
    title.setPosition(WINDOW_WIDTH / 2 - title.getLocalBounds().width / 2, 100);

    sf::Text lore("In a world consumed by chaos, only one survives.\nYou are the last Helldiver - forged in fire, bound by honor.\nSurvive the void. Protect the zone. Write your legend.", font, 18);
    lore.setFillColor(sf::Color(180, 180, 180));
    lore.setPosition(WINDOW_WIDTH / 2 - lore.getLocalBounds().width / 2, 200);

    sf::Text prompt("Press ENTER to Begin Your Dive", font, 24);
    prompt.setFillColor(sf::Color::White);
    prompt.setPosition(WINDOW_WIDTH / 2 - prompt.getLocalBounds().width / 2, 350);

    window.clear(sf::Color::Black);
    window.draw(title);
    window.draw(lore);
    window.draw(prompt);
    window.display();

    // Wait for user to press Enter or close window
    while (true) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) break;
    }
}

int main() {
    // Seed random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Create the main game window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "The Last Helldiver");
    window.setFramerateLimit(60);

    // Load and set background image scaled to window size
    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("background.jpeg");
    sf::Sprite background(backgroundTexture);
    background.setScale(
        float(WINDOW_WIDTH) / backgroundTexture.getSize().x,
        float(WINDOW_HEIGHT) / backgroundTexture.getSize().y);

    // Load textures for player, enemy, safe zone, and game over screen
    sf::Texture playerTexture, enemyTexture, zoneTexture, gameOverTexture;
    playerTexture.loadFromFile("player.png");
    enemyTexture.loadFromFile("enemy.png");
    zoneTexture.loadFromFile("zone_fire.png");
    gameOverTexture.loadFromFile("gameover.jpg");

    // Setup game over background sprite scaled to window size
    sf::Sprite gameOverBg(gameOverTexture);
    gameOverBg.setScale(
        float(WINDOW_WIDTH) / gameOverTexture.getSize().x,
        float(WINDOW_HEIGHT) / gameOverTexture.getSize().y);

    // Load font for HUD and start screen
    sf::Font font;
    font.loadFromFile("arial.ttf");

    // Show the start screen before gameplay begins
    showStartScreen(window, font);

    // Create player object
    Player player(playerTexture);

    // Create vectors to hold enemies and bullets
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;

    // Create the safe zone object
    SafeZone safeZone(zoneTexture);

    // Spawn initial enemies at random positions
    for (int i = 0; i < 5; ++i) {
        enemies.emplace_back(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, enemyTexture);
    }

    // Setup HUD text display
    sf::Text hudText("", font, 20);
    hudText.setFillColor(sf::Color::White);
    hudText.setPosition(10, 10);

    // Load shooting sound effect
    sf::SoundBuffer shootBuffer;
    shootBuffer.loadFromFile("shoot.wav");
    sf::Sound shootSound(shootBuffer);

    // Clocks for timing frame updates and bullet cooldown
    sf::Clock clock, bulletClock;
    float timeSinceLastSpawn = 0.0f;

    // Player score and kill count
    int score = 0, killCount = 0;

    // Main game loop runs while window is open
    while (window.isOpen()) {
        sf::Event event;
        // Process all window events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle shooting bullets on left mouse click with cooldown
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && bulletClock.getElapsedTime().asSeconds() > BULLET_COOLDOWN) {
                sf::Vector2f playerPos = player.getPosition();
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                bullets.emplace_back(playerPos.x, playerPos.y, mousePos.x - playerPos.x, mousePos.y - playerPos.y);
                shootSound.play();
                bulletClock.restart();
            }
        }

        // Calculate time elapsed since last frame
        float deltaTime = clock.restart().asSeconds();
        timeSinceLastSpawn += deltaTime;

        // Handle player input and movement
        player.handleInput();
        player.move();

        // Update bullets: move and remove if out of window bounds
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            it->move();
            if (it->getPosition().x < 0 || it->getPosition().x > WINDOW_WIDTH || it->getPosition().y < 0 || it->getPosition().y > WINDOW_HEIGHT) {
                it = bullets.erase(it);
            } else ++it;
        }

        // Update enemies: move towards player, check collisions with bullets and player
        for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ) {
            enemyIt->update(player.getPosition());
            enemyIt->move();

            bool dead = false;
            // Check collision with bullets
            for (auto bulletIt = bullets.begin(); bulletIt != bullets.end(); ) {
                float dx = bulletIt->getPosition().x - enemyIt->getPosition().x;
                float dy = bulletIt->getPosition().y - enemyIt->getPosition().y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < bulletIt->getRadius() + 12) {
                    enemyIt->takeDamage(25);
                    bulletIt = bullets.erase(bulletIt);
                    if (!enemyIt->isAlive()) {
                        score += 10;
                        killCount++;
                        dead = true;
                    }
                    break;
                } else ++bulletIt;
            }

            // Remove dead enemies, otherwise check collision with player
            if (dead) enemyIt = enemies.erase(enemyIt);
            else {
                float dx = player.getPosition().x - enemyIt->getPosition().x;
                float dy = player.getPosition().y - enemyIt->getPosition().y;
                if (std::sqrt(dx * dx + dy * dy) < 20) player.takeDamage(1);
                ++enemyIt;
            }
        }

        // Update safe zone size and check if player is outside it
        safeZone.update();
        bool out = !safeZone.isInside(player.getPosition());
        if (out) player.takeDamage(1);

        // Spawn new enemies periodically if below max count
        if (timeSinceLastSpawn > 3.0f && enemies.size() < 10) {
            enemies.emplace_back(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, enemyTexture);
            timeSinceLastSpawn = 0.0f;
        }

        // Update HUD text with current score, kills, and health
        hudText.setString("The Last Helldiver | Score: " + std::to_string(score) + " | Kills: " + std::to_string(killCount) + " | Health: " + std::to_string(player.getHealth()));

        // Draw all game elements
        window.clear();
        window.draw(background);
        window.draw(safeZone.getSprite());
        for (auto& b : bullets) window.draw(b.getShape());
        for (auto& e : enemies) window.draw(e.getSprite());
        window.draw(player.getSprite());
        window.draw(hudText);
        window.display();

        // Check if player is dead and show game over screen
        if (!player.isAlive()) {
            // Save final score and kills to file
            std::ofstream file("game_score.txt");
            file << "Final Score: " << score << "\nKills: " << killCount;
            file.close();

            // Display game over text on screen
            sf::Text gameOver("The Last Helldiver Fell\nFinal Score: " + std::to_string(score) + " | Kills: " + std::to_string(killCount), font, 32);
            gameOver.setFillColor(sf::Color::Red);
            gameOver.setPosition(WINDOW_WIDTH / 2 - gameOver.getLocalBounds().width / 2, WINDOW_HEIGHT / 2);
            window.clear();
            window.draw(gameOverBg);
            window.draw(gameOver);
            window.display();

            // Pause for 3 seconds before closing window
            sf::sleep(sf::seconds(3));
            window.close();
        }
    }
    return 0;
}
