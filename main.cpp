#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 700;
const float PLAYER_SPEED = 3.0f;
const float ENEMY_SPEED = 1.0f;
const float BULLET_SPEED = 7.0f;
const float SAFE_ZONE_SHRINK_RATE = 0.1f;
const float BULLET_COOLDOWN = 0.3f;

class Entity {
protected:
    sf::Sprite sprite;
    sf::Vector2f velocity;
    float speed;
    int health;

public:
    Entity(float x, float y, const sf::Texture& texture, float speed, int health)
        : speed(speed), health(health) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    }

    void move() { sprite.move(velocity); }
    bool isAlive() const { return health > 0; }
    void takeDamage(int amount) { health -= amount; }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::Sprite& getSprite() { return sprite; }
    int getHealth() const { return health; }
};

class Player : public Entity {
public:
    Player(const sf::Texture& texture) : Entity(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, texture, PLAYER_SPEED, 100) {}

    void handleInput() {
        velocity = {0, 0};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) velocity.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) velocity.y += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) velocity.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) velocity.x += speed;
    }
};

class Enemy : public Entity {
public:
    Enemy(float x, float y, const sf::Texture& texture)
        : Entity(x, y, texture, ENEMY_SPEED + static_cast<float>(rand() % 20) / 10.0f, 50) {}

    void update(const sf::Vector2f& playerPos) {
        sf::Vector2f direction = playerPos - sprite.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
            velocity = direction * speed;
        }
    }
};

class Bullet {
private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed;

public:
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

    void move() { shape.move(velocity); }
    sf::Vector2f getPosition() const { return shape.getPosition(); }
    float getRadius() const { return shape.getRadius(); }
    sf::CircleShape& getShape() { return shape; }
};

class SafeZone {
private:
    sf::Sprite sprite;
    float shrinkRate;
    float minRadius;
    float radius;

public:
    SafeZone(const sf::Texture& texture)
        : shrinkRate(SAFE_ZONE_SHRINK_RATE), minRadius(50.0f) {
        sprite.setTexture(texture);
        radius = std::min(WINDOW_WIDTH, WINDOW_HEIGHT) * 0.5f;
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        float scaleFactor = radius / (texture.getSize().x / 2);
        sprite.setScale(scaleFactor, scaleFactor);
    }

    void update() {
        if (radius > minRadius) {
            radius -= shrinkRate;
            float scaleFactor = radius / (sprite.getTexture()->getSize().x / 2);
            sprite.setScale(scaleFactor, scaleFactor);
        }
    }

    bool isInside(const sf::Vector2f& position) const {
        sf::Vector2f center(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        float distance = std::sqrt(std::pow(position.x - center.x, 2) + std::pow(position.y - center.y, 2));
        return distance < radius;
    }

    sf::Sprite& getSprite() { return sprite; }
};

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
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "The Last Helldiver");
    window.setFramerateLimit(60);

    // Background
    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("background.jpeg");
    sf::Sprite background(backgroundTexture);
    background.setScale(
        float(WINDOW_WIDTH) / backgroundTexture.getSize().x,
        float(WINDOW_HEIGHT) / backgroundTexture.getSize().y);

    // Load textures
    sf::Texture playerTexture, enemyTexture, zoneTexture, gameOverTexture;
    playerTexture.loadFromFile("player.png");
    enemyTexture.loadFromFile("enemy.png");
    zoneTexture.loadFromFile("zone_fire.png");
    gameOverTexture.loadFromFile("gameover.jpg");
    sf::Sprite gameOverBg(gameOverTexture);
    gameOverBg.setScale(
        float(WINDOW_WIDTH) / gameOverTexture.getSize().x,
        float(WINDOW_HEIGHT) / gameOverTexture.getSize().y);

    // Font & HUD
    sf::Font font;
    font.loadFromFile("arial.ttf");
    showStartScreen(window, font);

    Player player(playerTexture);
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    SafeZone safeZone(zoneTexture);

    for (int i = 0; i < 5; ++i) {
        enemies.emplace_back(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, enemyTexture);
    }

    sf::Text hudText("", font, 20);
    hudText.setFillColor(sf::Color::White);
    hudText.setPosition(10, 10);

    sf::SoundBuffer shootBuffer;
    shootBuffer.loadFromFile("shoot.wav");
    sf::Sound shootSound(shootBuffer);

    sf::Clock clock, bulletClock;
    float timeSinceLastSpawn = 0.0f;
    int score = 0, killCount = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && bulletClock.getElapsedTime().asSeconds() > BULLET_COOLDOWN) {
                sf::Vector2f playerPos = player.getPosition();
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                bullets.emplace_back(playerPos.x, playerPos.y, mousePos.x - playerPos.x, mousePos.y - playerPos.y);
                shootSound.play();
                bulletClock.restart();
            }
        }

        float deltaTime = clock.restart().asSeconds();
        timeSinceLastSpawn += deltaTime;
        player.handleInput();
        player.move();

        for (auto it = bullets.begin(); it != bullets.end(); ) {
            it->move();
            if (it->getPosition().x < 0 || it->getPosition().x > WINDOW_WIDTH || it->getPosition().y < 0 || it->getPosition().y > WINDOW_HEIGHT) {
                it = bullets.erase(it);
            } else ++it;
        }

        for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ) {
            enemyIt->update(player.getPosition());
            enemyIt->move();

            bool dead = false;
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

            if (dead) enemyIt = enemies.erase(enemyIt);
            else {
                float dx = player.getPosition().x - enemyIt->getPosition().x;
                float dy = player.getPosition().y - enemyIt->getPosition().y;
                if (std::sqrt(dx * dx + dy * dy) < 20) player.takeDamage(1);
                ++enemyIt;
            }
        }

        safeZone.update();
        bool out = !safeZone.isInside(player.getPosition());
        if (out) player.takeDamage(1);

        if (timeSinceLastSpawn > 3.0f && enemies.size() < 10) {
            enemies.emplace_back(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, enemyTexture);
            timeSinceLastSpawn = 0.0f;
        }

        hudText.setString("The Last Helldiver | Score: " + std::to_string(score) + " | Kills: " + std::to_string(killCount) + " | Health: " + std::to_string(player.getHealth()));

        window.clear();
        window.draw(background);
        window.draw(safeZone.getSprite());
        for (auto& b : bullets) window.draw(b.getShape());
        for (auto& e : enemies) window.draw(e.getSprite());
        window.draw(player.getSprite());
        window.draw(hudText);
        window.display();

        if (!player.isAlive()) {
            std::ofstream file("game_score.txt");
            file << "Final Score: " << score << "\nKills: " << killCount;
            file.close();

            sf::Text gameOver("The Last Helldiver Fell\nFinal Score: " + std::to_string(score) + " | Kills: " + std::to_string(killCount), font, 32);
            gameOver.setFillColor(sf::Color::Red);
            gameOver.setPosition(WINDOW_WIDTH / 2 - gameOver.getLocalBounds().width / 2, WINDOW_HEIGHT / 2);
            window.clear();
            window.draw(gameOverBg);
            window.draw(gameOver);
            window.display();
            sf::sleep(sf::seconds(3));
            window.close();
        }
    }
    return 0;
}
