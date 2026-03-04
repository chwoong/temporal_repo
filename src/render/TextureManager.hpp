#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <vector>

// PNG 파일을 로드하고, 없으면 컬러 도형으로 폴백하는 텍스처 관리자
class TextureManager {
public:
    // assets/tiles/ 디렉토리에서 모든 타일 텍스처 로드 시도
    // 없는 파일은 조용히 건너뜀 (폴백 컬러로 대체)
    void loadAll(const std::string& dir) {
        struct Entry { std::string key; std::string file; };
        std::vector<Entry> entries = {
            {"ground",    "ground.png"},
            {"wall",      "wall.png"},
            {"player",    "player.png"},
            {"home",      "home.png"},
            {"market",    "market.png"},
            {"mayor",     "mayor.png"},
            {"crop0",     "crop_stage0.png"},
            {"crop1",     "crop_stage1.png"},
            {"crop2",     "crop_stage2.png"},
            {"livestock", "livestock.png"}
        };
        for (auto& e : entries) {
            sf::Texture tex;
            if (tex.loadFromFile(dir + "/" + e.file)) {
                textures_[e.key] = std::move(tex);
            }
        }
    }

    // 폰트 로드 (한국어 지원 TTF 필요)
    bool loadFont(const std::string& path) {
        return font_.loadFromFile(path);
    }

    const sf::Texture* get(const std::string& key) const {
        auto it = textures_.find(key);
        if (it != textures_.end()) return &it->second;
        return nullptr;
    }

    sf::Font&       getFont()       { return font_; }
    const sf::Font& getFont() const { return font_; }

    // 텍스처가 있으면 스프라이트, 없으면 컬러 사각형으로 그리기
    void drawTileOrShape(sf::RenderWindow& w, const std::string& key,
                         float px, float py, int tileSize,
                         sf::Color fallback) const {
        const sf::Texture* tex = get(key);
        if (tex) {
            sf::Sprite spr(*tex);
            spr.setPosition(px, py);
            float sx = (float)tileSize / (float)tex->getSize().x;
            float sy = (float)tileSize / (float)tex->getSize().y;
            spr.setScale(sx, sy);
            w.draw(spr);
        } else {
            sf::RectangleShape rect(sf::Vector2f((float)tileSize, (float)tileSize));
            rect.setPosition(px, py);
            rect.setFillColor(fallback);
            w.draw(rect);
        }
    }

private:
    std::map<std::string, sf::Texture> textures_;
    sf::Font font_;
};
