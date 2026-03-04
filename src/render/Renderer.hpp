#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include "../game/GameLogic.hpp"
#include "../game/GameState.hpp"
#include "TextureManager.hpp"

// =========================================================
// 맵/엔티티/플레이어/UI/다이얼로그를 렌더링하는 클래스
//
// 창 레이아웃:
//   [0 ~ 767px]  게임 맵 (12x8 타일, 64px/타일)
//   [768 ~ 1023] UI 패널 (256px 너비)
// =========================================================
class Renderer {
public:
    static constexpr int TILE    = 64;
    static constexpr int MAP_W   = 12;
    static constexpr int MAP_H   = 8;
    static constexpr int PANEL_X = MAP_W * TILE;  // 768

    Renderer(sf::RenderWindow& window, TextureManager& tex)
        : window_(window), tex_(tex) {}

    void drawAll(const std::string    baseMap[],
                 const std::vector<FarmEntity*>& farm,
                 const Player&        player,
                 GameState            state,
                 const std::string&   notifyMsg,
                 const MinigameResult& mgResult,
                 const Inventory<std::string>& storage)
    {
        window_.clear(sf::Color(20, 20, 30));

        // 1. 맵 타일
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                char c = baseMap[y][x];
                drawMapTile(c, x, y);
            }
        }

        // 2. 농장 엔티티
        for (auto* e : farm) {
            drawEntity(e);
        }

        // 3. 플레이어
        tex_.drawTileOrShape(window_, "player",
            (float)player.x * TILE, (float)player.y * TILE,
            TILE, sf::Color(0, 200, 255));

        // 4. UI 패널
        drawUIPanel(player, storage);

        // 5. 다이얼로그 오버레이
        drawDialogOverlay(state, notifyMsg, mgResult);

        window_.display();
    }

private:
    sf::RenderWindow& window_;
    TextureManager&   tex_;

    // ---- 타일 1개 그리기 ----
    void drawMapTile(char c, int mx, int my) {
        std::string key;
        sf::Color   col;
        switch (c) {
            case '#': key = "wall";   col = sf::Color(80,  60,  40);  break;
            case 'H': key = "home";   col = sf::Color(240, 200, 80);  break;
            case 'M': key = "market"; col = sf::Color(80,  200, 80);  break;
            case 'G': key = "mayor";  col = sf::Color(200, 80,  200); break;
            default:  key = "ground"; col = sf::Color(180, 140, 80);  break;
        }
        tex_.drawTileOrShape(window_, key,
            (float)mx * TILE, (float)my * TILE, TILE, col);
    }

    // ---- 엔티티 그리기 (작물 성장 단계 반영) ----
    void drawEntity(const FarmEntity* e) {
        std::string key;
        sf::Color   col;
        if (e->symbol == 'C') {
            if (e->isReady())        { key = "crop2"; col = sf::Color(60,  180, 60);  }
            else if (e->getAge() >= 1){ key = "crop1"; col = sf::Color(100, 160, 60); }
            else                      { key = "crop0"; col = sf::Color(100, 80,  40); }
        } else {
            key = "livestock";
            col = sf::Color(220, 200, 150);
        }
        tex_.drawTileOrShape(window_, key,
            (float)e->x * TILE, (float)e->y * TILE, TILE, col);
    }

    // ---- 오른쪽 UI 패널 ----
    void drawUIPanel(const Player& player, const Inventory<std::string>& storage) {
        // 패널 배경
        sf::RectangleShape bg(sf::Vector2f(256.f, (float)(MAP_H * TILE)));
        bg.setPosition((float)PANEL_X, 0.f);
        bg.setFillColor(sf::Color(40, 40, 60));
        window_.draw(bg);

        // 구분선
        sf::RectangleShape line(sf::Vector2f(2.f, (float)(MAP_H * TILE)));
        line.setPosition((float)PANEL_X, 0.f);
        line.setFillColor(sf::Color(100, 100, 160));
        window_.draw(line);

        const sf::Font& font = tex_.getFont();
        if (font.getInfo().family.empty()) return;

        float px  = (float)PANEL_X + 12.f;
        float py  = 16.f;
        float lh  = 28.f;

        auto draw = [&](const std::string& s, unsigned sz = 17,
                        sf::Color col = sf::Color::White) {
            sf::Text t(sf::String::fromUtf8(s.begin(), s.end()), font, sz);
            t.setPosition(px, py);
            t.setFillColor(col);
            window_.draw(t);
            py += lh;
        };

        draw("=== Farm Simulator ===", 15, sf::Color(200, 220, 255));
        py += 4;
        draw("DAY   : " + std::to_string(player.getDay()));
        draw("Level : " + std::to_string(player.getLevel()));
        draw("EXP   : " + std::to_string(player.getExp()) +
             "/" + std::to_string(player.getExpNeeded()));
        draw("Gold  : " + std::to_string(player.getMoney()) + " G",
             17, sf::Color(255, 220, 100));
        draw("Items : " + std::to_string(storage.count()),
             17, sf::Color(150, 220, 150));

        py += 10;
        draw("--- Controls ---", 14, sf::Color(180, 180, 180));
        lh = 24.f;
        draw("WASD   : Move",       14, sf::Color(200, 200, 200));
        draw("H tile : Sleep",      14, sf::Color(200, 200, 200));
        draw("M tile : Market",     14, sf::Color(200, 200, 200));
        draw("G tile : Minigame",   14, sf::Color(200, 200, 200));
        draw("C/L    : Interact",   14, sf::Color(200, 200, 200));
        draw("Q      : Quit",       14, sf::Color(200, 200, 200));
    }

    // ---- 다이얼로그 오버레이 ----
    void drawDialogOverlay(GameState state, const std::string& notifyMsg,
                           const MinigameResult& mg) {
        switch (state) {
        case GameState::DIALOG_NOTIFY: {
            // notifyMsg에 \n이 포함될 수 있으므로 분리
            std::vector<std::string> lines;
            std::istringstream ss(notifyMsg);
            std::string ln;
            while (std::getline(ss, ln)) {
                if (!ln.empty()) lines.push_back(ln);
            }
            lines.emplace_back("(아무 키나 누르세요)");
            drawModalBox(lines);
            break;
        }
        case GameState::DIALOG_SLEEP:
            drawModalBox({
                "[침대] 하루를 마무리 하시겠습니까?",
                "  Y : 잠자기   N : 취소"
            });
            break;
        case GameState::DIALOG_MARKET_MENU:
            drawModalBox({
                "[시장] 무엇을 하시겠습니까?",
                "  1 : 모두 팔기",
                "  2 : 구매하기",
                "  0 : 취소"
            });
            break;
        case GameState::DIALOG_MARKET_BUY:
            drawModalBox({
                "[상점] 무엇을 구매하시겠습니까?",
                "  1 : 감자 씨앗  (100G)",
                "  2 : 병아리     (200G)",
                "  0 : 취소"
            });
            break;
        case GameState::DIALOG_MINIGAME:
            if (mg.playerDice == 0) {
                drawModalBox({
                    "[마을 회관] 주사위 도박",
                    mg.message,
                    "(아무 키나 누르세요)"
                });
            } else {
                drawModalBox({
                    "[마을 회관] 주사위 도박",
                    "내 주사위: " + std::to_string(mg.playerDice) +
                    "   촌장: "  + std::to_string(mg.mayorDice),
                    mg.message,
                    "(아무 키나 누르세요)"
                });
            }
            break;
        default:
            break;
        }
    }

    // ---- 반투명 박스 + 텍스트 ----
    void drawModalBox(const std::vector<std::string>& lines) {
        // 맵 영역에만 오버레이
        sf::RectangleShape overlay(sf::Vector2f((float)(MAP_W * TILE), (float)(MAP_H * TILE)));
        overlay.setFillColor(sf::Color(0, 0, 0, 160));
        window_.draw(overlay);

        float lineH = 38.f;
        float boxW  = 520.f;
        float boxH  = (float)lines.size() * lineH + 40.f;
        float boxX  = ((float)(MAP_W * TILE) - boxW) / 2.f;
        float boxY  = ((float)(MAP_H * TILE) - boxH) / 2.f;

        sf::RectangleShape box(sf::Vector2f(boxW, boxH));
        box.setPosition(boxX, boxY);
        box.setFillColor(sf::Color(50, 50, 80, 230));
        box.setOutlineColor(sf::Color(180, 180, 255));
        box.setOutlineThickness(2.f);
        window_.draw(box);

        const sf::Font& font = tex_.getFont();
        if (font.getInfo().family.empty()) return;

        for (int i = 0; i < (int)lines.size(); i++) {
            sf::Text t(sf::String::fromUtf8(lines[i].begin(), lines[i].end()), font, 20);
            t.setPosition(boxX + 20.f, boxY + 15.f + (float)i * lineH);
            t.setFillColor(sf::Color::White);
            window_.draw(t);
        }
    }
};
