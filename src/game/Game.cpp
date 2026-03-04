#include "Game.hpp"

Game::Game()
    : window_(sf::VideoMode(1024, 512), "Farm Simulator", sf::Style::Close)
    , player_(5, 4)
    , state_(GameState::PLAYING)
    , lastMinigame_{0, 0, 0, ""}
    , renderer_(window_, textures_)
{
    window_.setFramerateLimit(60);

    // assets 로드 (실패해도 컬러 폴백으로 동작)
    textures_.loadAll("assets/tiles");
    textures_.loadFont("assets/tiles/font.ttf");

    // 맵 초기화 (기존 main.cpp와 동일)
    baseMap_[0] = "############";
    baseMap_[1] = "#H........M#";
    baseMap_[2] = "#..........#";
    baseMap_[3] = "#..........#";
    baseMap_[4] = "#..........#";
    baseMap_[5] = "#..........#";
    baseMap_[6] = "#G.........#";
    baseMap_[7] = "############";

    // 재해 큐
    disasterEvents_.push("가뭄");
    disasterEvents_.push("전염병");
    disasterEvents_.push("태풍");

    // 시작 작물
    farm_.push_back(new Crop(2, 2));
}

Game::~Game() {
    for (auto* e : farm_) delete e;
}

void Game::run() {
    while (window_.isOpen() && state_ != GameState::GAME_OVER) {
        handleEvents();
        render();
    }
}

// ---- 이벤트 처리 ----

void Game::handleEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            state_ = GameState::GAME_OVER;
            window_.close();
        }
        if (event.type == sf::Event::KeyPressed) {
            if (state_ == GameState::PLAYING)
                handleKeyInPlaying(event.key.code);
            else
                handleKeyInDialog(event.key.code);
        }
    }
}

void Game::handleKeyInPlaying(sf::Keyboard::Key key) {
    int dx = 0, dy = 0;
    if      (key == sf::Keyboard::W) dy = -1;
    else if (key == sf::Keyboard::S) dy = +1;
    else if (key == sf::Keyboard::A) dx = -1;
    else if (key == sf::Keyboard::D) dx = +1;
    else if (key == sf::Keyboard::Q) {
        state_ = GameState::GAME_OVER;
        window_.close();
        return;
    }
    else return;

    tryMove(dx, dy);
}

void Game::tryMove(int dx, int dy) {
    int nx = player_.x + dx;
    int ny = player_.y + dy;

    // 경계 및 벽 체크
    if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) return;
    if (baseMap_[ny][nx] == '#') return;

    // 엔티티 상호작용 체크
    for (auto* e : farm_) {
        if (e->x == nx && e->y == ny) {
            notifyMsg_ = e->interact(player_, storage_);
            state_     = GameState::DIALOG_NOTIFY;
            return;
        }
    }

    // 특수 타일 처리
    char tile = baseMap_[ny][nx];
    if (tile == 'H') {
        state_ = GameState::DIALOG_SLEEP;
        return;
    } else if (tile == 'M') {
        state_ = GameState::DIALOG_MARKET_MENU;
        return;
    } else if (tile == 'G') {
        lastMinigame_ = playMinigame(player_);
        state_        = GameState::DIALOG_MINIGAME;
        return;
    }

    // 이동
    player_.x = nx;
    player_.y = ny;
}

// ---- 다이얼로그 키 처리 ----

void Game::handleKeyInDialog(sf::Keyboard::Key key) {
    switch (state_) {

    case GameState::DIALOG_NOTIFY:
        state_ = GameState::PLAYING;
        break;

    case GameState::DIALOG_SLEEP:
        if (key == sf::Keyboard::Y) {
            doNextDay();
        } else {
            state_ = GameState::PLAYING;
        }
        break;

    case GameState::DIALOG_MARKET_MENU:
        if (key == sf::Keyboard::Num1) {
            int itemCount = market_.sellAll(player_, storage_);
            notifyMsg_ = "[시장] 모두 팔아 " + std::to_string(itemCount * 30) + "G를 벌었습니다!";
            state_     = GameState::DIALOG_NOTIFY;
        } else if (key == sf::Keyboard::Num2) {
            state_ = GameState::DIALOG_MARKET_BUY;
        } else {
            state_ = GameState::PLAYING;
        }
        break;

    case GameState::DIALOG_MARKET_BUY:
        if (key == sf::Keyboard::Num1) {
            bool ok = market_.buyCrop(player_, farm_);
            notifyMsg_ = ok ? "[구입] 감자 씨앗을 심었습니다! (-100G)"
                            : "[오류] 잔액 부족 또는 빈 자리 없음";
            state_ = GameState::DIALOG_NOTIFY;
        } else if (key == sf::Keyboard::Num2) {
            bool ok = market_.buyLivestock(player_, farm_);
            notifyMsg_ = ok ? "[구입] 병아리를 들였습니다! (-200G)"
                            : "[오류] 잔액 부족 또는 빈 자리 없음";
            state_ = GameState::DIALOG_NOTIFY;
        } else {
            state_ = GameState::PLAYING;
        }
        break;

    case GameState::DIALOG_MINIGAME:
        state_ = GameState::PLAYING;
        break;

    default:
        state_ = GameState::PLAYING;
        break;
    }
}

// ---- 하루 넘기기 ----

void Game::doNextDay() {
    player_.nextDay();
    for (auto* e : farm_) e->grow();

    std::string msg = "새 아침이 밝았습니다. (DAY " + std::to_string(player_.getDay()) + ")";

    if (rand() % 100 < 20 && !disasterEvents_.empty()) {
        std::string disaster = disasterEvents_.front();
        disasterEvents_.pop();
        for (auto* e : farm_) e->hitByDisaster();
        msg += "\n[재해] " + disaster + " 발생! 모든 작물/가축이 피해를 입었습니다.";
    }

    notifyMsg_ = msg;
    state_     = GameState::DIALOG_NOTIFY;
}

// ---- 렌더링 ----

void Game::render() {
    renderer_.drawAll(baseMap_, farm_, player_, state_, notifyMsg_, lastMinigame_, storage_);
}
