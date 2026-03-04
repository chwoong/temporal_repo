#pragma once
// =========================================================
// 기존 게임 로직 클래스 (터미널 I/O 제거, SFML GUI용으로 수정)
// - cin/cout/pauseGame 제거
// - interact() → std::string 반환으로 변경
// - FarmEntity에 getAge(), isReady() 접근자 추가
// - Market 순수 로직 메서드로 분리
// - playMinigame() → MinigameResult 구조체 반환
// =========================================================

#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <cstdlib>
#include <sstream>

using namespace std;

// =========================================================
// Template, STL (stack)
// =========================================================
template <typename T>
class Inventory {
private:
    stack<T> items;
public:
    void add(T item) { items.push(item); }
    bool use() {
        if (items.empty()) return false;
        items.pop();
        return true;
    }
    int count() const { return (int)items.size(); }
};

class Player;
class FarmEntity;

// =========================================================
// 시장 클래스 선언 (Friend 활용)
// =========================================================
class Market {
public:
    // 인벤토리 전부 판매 → 판매한 아이템 수 반환
    int sellAll(Player& player, Inventory<string>& inv);
    // 감자 씨앗 구매 → 성공 여부 반환
    bool buyCrop(Player& player, vector<FarmEntity*>& farm);
    // 병아리 구매 → 성공 여부 반환
    bool buyLivestock(Player& player, vector<FarmEntity*>& farm);

private:
    bool findEmptySlot(const vector<FarmEntity*>& farm, int& outX, int& outY) const;
};

// =========================================================
// Operator Overloading을 포함한 Player 클래스
// =========================================================
class Player {
private:
    int money;
    int level;
    int exp;
    int day;
    friend class Market;

public:
    int x, y;

    Player(int startX, int startY)
        : money(500), level(1), exp(0), day(1), x(startX), y(startY) {}

    void nextDay()          { day++; }
    int  getDay()   const   { return day; }
    int  getMoney() const   { return money; }
    int  getLevel() const   { return level; }
    int  getExp()   const   { return exp; }
    int  getExpNeeded() const { return level * 100; }

    void gainExp(int amount) {
        exp += amount;
        if (exp >= level * 100) {
            exp -= level * 100;
            level++;
        }
    }

    Player& operator+=(int amount) { money += amount; return *this; }
    Player& operator-=(int amount) { money -= amount; return *this; }

    friend ostream& operator<<(ostream& os, const Player& p) {
        os << "DAY " << p.day << " | LV " << p.level
           << " (EXP " << p.exp << "/" << p.level * 100 << ") | " << p.money << "G";
        return os;
    }
};

// =========================================================
// Virtual Class & Inheritance
// =========================================================
class FarmEntity {
protected:
    string name;
    int age;
    bool readyToHarvest;
public:
    int  x, y;
    char symbol;

    FarmEntity(string n, int startX, int startY, char sym)
        : name(n), age(0), readyToHarvest(false), x(startX), y(startY), symbol(sym) {}

    virtual ~FarmEntity() {}

    virtual void   grow()    = 0;
    virtual string interact(Player& p, Inventory<string>& inv) = 0;

    virtual void hitByDisaster() {
        age = 0;
        readyToHarvest = false;
    }

    // 렌더러용 접근자
    int         getAge()  const { return age; }
    bool        isReady() const { return readyToHarvest; }
    const string& getName() const { return name; }
};

class Crop : public FarmEntity {
public:
    Crop(int x, int y) : FarmEntity("감자", x, y, 'C') {}

    void grow() override {
        if (!readyToHarvest) {
            age++;
            if (age >= 3) readyToHarvest = true;
        }
    }

    string interact(Player& p, Inventory<string>& inv) override {
        if (readyToHarvest) {
            inv.add("감자");
            p.gainExp(20);
            age = 0;
            readyToHarvest = false;
            return "[밭(" + to_string(x) + "," + to_string(y) + ")] 감자 수확! (+20 EXP)";
        } else {
            return "[밭(" + to_string(x) + "," + to_string(y) + ")] 아직 자라는 중 (성장도: " + to_string(age) + "/3)";
        }
    }
};

class Livestock : public FarmEntity {
public:
    Livestock(int x, int y) : FarmEntity("닭", x, y, 'L') {}

    void grow() override {
        age++;
        readyToHarvest = true;
    }

    string interact(Player& p, Inventory<string>& inv) override {
        if (readyToHarvest) {
            inv.add("달걀");
            p.gainExp(15);
            readyToHarvest = false;
            return "[축사(" + to_string(x) + "," + to_string(y) + ")] 달걀을 얻었습니다! (+15 EXP)";
        } else {
            return "[축사(" + to_string(x) + "," + to_string(y) + ")] 오늘은 더 이상 달걀이 없습니다.";
        }
    }
};

// =========================================================
// Market 구현부 (inline)
// =========================================================
inline bool Market::findEmptySlot(const vector<FarmEntity*>& farm, int& outX, int& outY) const {
    for (int i = 2; i <= 6; i++) {
        for (int j = 2; j <= 8; j++) {
            bool occupied = false;
            for (auto* e : farm) {
                if (e->x == j && e->y == i) { occupied = true; break; }
            }
            if (!occupied) { outX = j; outY = i; return true; }
        }
    }
    return false;
}

inline int Market::sellAll(Player& player, Inventory<string>& inv) {
    int count = 0;
    while (inv.use()) {
        player.money += 30;
        count++;
    }
    return count;
}

inline bool Market::buyCrop(Player& player, vector<FarmEntity*>& farm) {
    int nx = -1, ny = -1;
    if (!findEmptySlot(farm, nx, ny)) return false;
    if (player.getMoney() < 100) return false;
    player -= 100;
    farm.push_back(new Crop(nx, ny));
    return true;
}

inline bool Market::buyLivestock(Player& player, vector<FarmEntity*>& farm) {
    int nx = -1, ny = -1;
    if (!findEmptySlot(farm, nx, ny)) return false;
    if (player.getMoney() < 200) return false;
    player -= 200;
    farm.push_back(new Livestock(nx, ny));
    return true;
}

// =========================================================
// 미니게임 (결과 구조체 반환)
// =========================================================
struct MinigameResult {
    int    playerDice;
    int    mayorDice;
    int    goldDelta;
    string message;
};

inline MinigameResult playMinigame(Player& p) {
    MinigameResult r{0, 0, 0, ""};
    if (p.getMoney() < 50) {
        r.message = "촌장: \"자네, 돈이 없구만! (50G 필요)\"";
        return r;
    }
    p -= 50;
    r.playerDice = (rand() % 6) + 1;
    r.mayorDice  = (rand() % 6) + 1;

    if (r.playerDice > r.mayorDice) {
        r.goldDelta = +150;
        r.message   = "자네가 이겼네! +150G";
        p += 150;
    } else if (r.playerDice < r.mayorDice) {
        r.goldDelta = -50;
        r.message   = "촌장이 이겼네! -50G";
    } else {
        r.goldDelta = 0;
        r.message   = "비겼구만. 50G 반환.";
        p += 50;
    }
    return r;
}
