#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <string>
#include "GameLogic.hpp"
#include "GameState.hpp"
#include "../render/TextureManager.hpp"
#include "../render/Renderer.hpp"

// =========================================================
// Game: SFML 창 및 게임 루프를 소유하는 최상위 클래스
// 멤버 선언 순서 = 초기화 순서 (window_ → textures_ → renderer_)
// =========================================================
class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // ---- SFML 창 ----
    sf::RenderWindow window_;

    // ---- 게임 객체 (GameLogic.hpp의 기존 클래스) ----
    Player                   player_;
    Market                   market_;
    Inventory<std::string>   storage_;
    std::vector<FarmEntity*> farm_;
    std::queue<std::string>  disasterEvents_;

    // ---- 상태 머신 ----
    GameState        state_;
    std::string      notifyMsg_;
    MinigameResult   lastMinigame_;

    // ---- 맵 데이터 ----
    static constexpr int MAP_W = 12;
    static constexpr int MAP_H = 8;
    std::string baseMap_[MAP_H];

    // ---- 서브시스템 (window_/textures_보다 나중에 선언) ----
    TextureManager textures_;
    Renderer       renderer_;

    // ---- 프레임 메서드 ----
    void handleEvents();
    void handleKeyInPlaying(sf::Keyboard::Key key);
    void handleKeyInDialog(sf::Keyboard::Key key);
    void render();

    // ---- 게임 로직 헬퍼 ----
    void tryMove(int dx, int dy);
    void doNextDay();
};
