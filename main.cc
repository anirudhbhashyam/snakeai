#include <cstdint>
#include <iostream>
#include <memory>
#include <print>

#include <random>
#include <ranges>
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

std::random_device rd;
std::mt19937 gen(rd());

constexpr Color hex_to_color(uint32_t hex) {
    return Color{
        .a = static_cast<unsigned char>((hex >> 8 * 3) & 0xFF),
        .r = static_cast<unsigned char>((hex >> 8 * 2) & 0xFF),
        .g = static_cast<unsigned char>((hex >> 8 * 1) & 0xFF),
        .b = static_cast<unsigned char>((hex >> 8 * 0) & 0xFF)
    };
}

constexpr uint32_t WINDOW_WIDTH = 1600;
constexpr uint32_t WINDOW_HEIGHT = 1200;
constexpr Color BACKGROUND_COLOR{ 14, 18, 25, 255 };
constexpr Color SNAKE_HEAD_COLOR = hex_to_color(0xFFFF1CA8);
constexpr Color SNAKE_BODY_COLOR = hex_to_color(0xFF3066BE);
constexpr uint16_t SNAKE_SIZE{ 20 };
constexpr float SNAKE_VELOCITY{ SNAKE_SIZE };
constexpr uint16_t FPS{ 60 };
constexpr Color FOOD_COLOR = hex_to_color(0xFF1CFFA4);

std::uniform_real_distribution<float> WIDTH_DIS(20, WINDOW_WIDTH - 20);
std::uniform_real_distribution<float> HEIGHT_DIS(20, WINDOW_HEIGHT - 20);

enum Direction {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    NO_DIRECTION
};

enum GameState {
    START = 0,
    RUNNING,
    END,
    PAUSE,
};
struct GameStateHash {
    size_t operator()(auto t) const {
        return static_cast<int32_t>(t);
    }
};

template <typename T>
concept Real = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <Real T>
struct Vec2 {
    public:
        constexpr Vec2() = default;
        constexpr Vec2(T x_, T y_) : x{ x_ }, y{ y_ } {}

        constexpr bool operator==(const Vec2& other) const {
            return this->x == other.x && this->y == other.y;
        }

        constexpr bool operator!=(const Vec2& other) const {
            return !((*this) == other);
        }

        constexpr Vec2 operator+(const Vec2& other) const {
            return Vec2 {
                this->x + other.x,
                this->y + other.y,
            };
        }

        constexpr Vec2 operator-(const Vec2& other) const {
            return Vec2 {
                this->x - other.x,
                this->y - other.y,
            };
        }

        constexpr Vec2 operator*(T f) const {
            return Vec2 {
                this->x * f,
                this->y * f,
            };
        }

        constexpr T dot(const Vec2& other) const {
            return this->x * other.x + this->y * other.y;
        }

        T x{ };
        T y{ };
};

struct Food {
    public:
        Food() = default;
        Food(Vec2<float> pos_) : pos{ pos_ } {}

        void draw() {
            DrawRectangle(pos.x, pos.y, size, size, FOOD_COLOR);
        }

        void reset() {
            pos = Vec2<float>{
                WIDTH_DIS(gen),
                HEIGHT_DIS(gen)
            };
        }
    private:
        Vec2<float> pos{ };
        uint16_t size{ 20 };

    friend struct Snake;
};

struct Snake {
    public:
        Snake() = default;
        Snake(Vec2<float> head_, uint16_t size_)
            : head{ head_ }, size{ size_ } {
            body.push_back(head);
        }

        void draw() const {
            DrawRectangle(
                (int32_t) head.x,
                (int32_t) head.y,
                size,
                size,
                SNAKE_HEAD_COLOR
            );

            DrawRectangleLines(
                (int32_t) head.x,
                (int32_t) head.y,
                size,
                size,
                WHITE
            );

            for (
                const auto& pos: body
                | std::views::drop(1)
            ) {
                DrawRectangle(
                    (int32_t) pos.x,
                    (int32_t) pos.y,
                    size,
                    size,
                    SNAKE_BODY_COLOR
                );
                DrawRectangleLines(
                    (int32_t) pos.x,
                    (int32_t) pos.y,
                    size,
                    size,
                    WHITE
                );
            }
        }

        void move() {
            if (_is_boundary_collision()) return;
            _set_direction();
            switch(dir) {
                case UP:
                    head.y -= vel;
                    break;
                case DOWN:
                    head.y += vel;
                    break;
                case LEFT:
                    head.x -= vel;
                    break;
                case RIGHT:
                    head.x += vel;
                    break;
                default:
                    return;
            }

            body.pop_back();
            body.insert(body.begin(), head);
        }

        void eat(Food& food) {
            if (!_is_food_collision(food)) return;
            food.reset();
            _add_segment();
        }

        size_t get_length() const {
            return body.size();
        }

    private:

        inline void _set_direction() {
            // Would IsKeyPressed work?
            if (IsKeyDown(KEY_W)) {
                dir = UP;
            }
            if (IsKeyDown(KEY_S)) {
                dir = DOWN;
            }
            if (IsKeyDown(KEY_A)) {
                dir = LEFT;
            }
            if (IsKeyDown(KEY_D)) {
                dir = RIGHT;
            }
        }

        inline bool _is_boundary_collision() {
             return (
                (head.x <= 2 || head.x >= WINDOW_WIDTH - size - 2)
                || (head.y <= 2 || head.y >= WINDOW_HEIGHT - size - 2)
            );
        }

        inline bool _is_food_collision(const Food& food) {
            bool x_condition = (
                head.x < (food.pos.x + food.size)
                && food.pos.x < (head.x + size)
            );
            bool y_condition = (
                head.y < (food.pos.y + food.size)
                && food.pos.y < (head.y + size)
            );
            return x_condition && y_condition;
        }

        void _add_segment() {
            Vec2<float> new_segment{
                body.back()
            };
            switch(dir) {
                case UP:
                    new_segment.y += size;
                    break;
                case DOWN:
                    new_segment.y -= size;
                    break;
                case LEFT:
                    new_segment.x -= size;
                    break;
                case RIGHT:
                    new_segment.x += size;
                    break;
                default:
                    return;
            }
            body.push_back(new_segment);
        }

    private:
        Vec2<float> head{ };
        std::vector<Vec2<float>> body{ };
        uint16_t size{ };
        float vel{ SNAKE_VELOCITY };
        Direction dir{ NO_DIRECTION };
};

struct Level {
    public:
        Level() = default;
        virtual ~Level() = default;
        virtual void render() = 0;
        virtual void update(GameState*) = 0;
};

struct StartLevel : public Level {
    public:
        StartLevel() = default;
        void render() override {
            ClearBackground(BACKGROUND_COLOR);
            DrawText(
                "Play",
                GetScreenWidth() / 2,
                GetScreenHeight() / 2,
                50,
                WHITE
            );
        }
        void update(GameState* state) override {
            if (IsKeyPressed(KEY_SPACE)) {
                *state = RUNNING;
            }
        }
};

struct RunningLevel : public Level {
    public:
        RunningLevel() = default;

        void render() override {
            ClearBackground(BACKGROUND_COLOR);
            snake.draw();
            food.draw();
        }

        void update(GameState* state) override {
            snake.move();
            snake.eat(food);
            (void) state;
        }

    private:
        Snake snake{
            Vec2<float> { 100, 100 },
            SNAKE_SIZE
        };

        Food food{
            Vec2<float> { 300, 300 }
        };
};

struct Game {
    public:
        Game() {
            InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "SNAKERL");
        }

        ~Game() {
            CloseWindow();
        }

        void run() {
            while (!WindowShouldClose()) {
                SetTargetFPS(fps);

                level_cache[state]->update(&state);

                BeginDrawing();
                    level_cache[state]->render();

                    // update_score();
                    // DrawText(score.c_str(), 10, 10, 30, WHITE);

                EndDrawing();
            }
        }

        // void update_score() {
        //     score = std::to_string(snake.get_length() - 1);
        // }

    private:
        GameState state{ START };

        uint16_t fps{ FPS };

        std::string score{ };

        std::unordered_map<
            GameState,
            std::shared_ptr<Level>
        > level_cache{
           { START, std::make_shared<StartLevel>() },
           { RUNNING, std::make_shared<RunningLevel>() }
        };
};

int32_t main() {
    SetTraceLogLevel(LOG_ERROR);
    Game game{ };
    game.run();
    return 0;
}
