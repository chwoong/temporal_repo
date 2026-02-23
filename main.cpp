#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <cstdlib>
#include <ctime>

// =========================================================
// [OS 호환성 처리] Windows / Mac / Linux 구분
// =========================================================
#ifdef _WIN32
    #include <conio.h>
    #define CLEAR_SCREEN "cls"
#else
    #include <termios.h>
    #include <unistd.h>
    #define CLEAR_SCREEN "clear"
    
    // Mac/Linux 환경을 위한 _getch() 직접 구현
    int _getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif

using namespace std;

// OS에 상관없이 작동하는 일시정지 함수
void pauseGame() {
    cout << "\n(계속하려면 아무 키나 누르세요...)\n";
    _getch();
}

// =========================================================
// [C++ 조건] Template, STL (stack)
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
    int count() const { return items.size(); }
};

class Player;
class FarmEntity;

// =========================================================
// [C++ 조건] Friend 활용을 위한 시장 클래스 선언
// =========================================================
class Market {
public:
    void sellProduce(Player& player, Inventory<string>& inv);
    void buyEntity(Player& player, vector<FarmEntity*>& farm);
};

// =========================================================
// [C++ 조건] Operator Overloading을 포함한 Player 클래스
// =========================================================
class Player {
private:
    int money;
    int level;
    int exp;
    int day;
    friend class Market; 

public:
    int x, y; // 맵 상의 위치

    Player(int startX, int startY) : money(500), level(1), exp(0), day(1), x(startX), y(startY) {}

    void nextDay() { day++; }
    int getDay() const { return day; }
    int getMoney() const { return money; }

    void gainExp(int amount) {
        exp += amount;
        if (exp >= level * 100) {
            exp -= level * 100;
            level++;
        }
    }

    Player& operator+=(int amount) {
        this->money += amount;
        return *this;
    }
    Player& operator-=(int amount) {
        this->money -= amount;
        return *this;
    }

    friend ostream& operator<<(ostream& os, const Player& p) {
        os << "=================================================\n"
           << " 🗓️  DAY: " << p.day << " | 👤 LV: " << p.level 
           << " (EXP: " << p.exp << "/" << p.level * 100 << ") | 💰 " << p.money << " G\n"
           << "=================================================";
        return os;
    }
};

// =========================================================
// [C++ 조건] Virtual Class & Inheritance
// =========================================================
class FarmEntity {
protected:
    string name;
    int age;
    bool readyToHarvest;
public:
    int x, y;
    char symbol;

    FarmEntity(string n, int startX, int startY, char sym) 
        : name(n), age(0), readyToHarvest(false), x(startX), y(startY), symbol(sym) {}
    
    virtual ~FarmEntity() {} 
    
    virtual void grow() = 0; 
    virtual void interact(Player& p, Inventory<string>& inv) = 0;
    
    virtual void hitByDisaster() {
        age = 0;
        readyToHarvest = false;
    }
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
    
    void interact(Player& p, Inventory<string>& inv) override {
        cout << "\n[밭(" << x << "," << y << ")] ";
        if (readyToHarvest) {
            cout << "감자 수확 완료!\n";
            inv.add("감자");
            p.gainExp(20);
            age = 0;
            readyToHarvest = false;
        } else {
            cout << "아직 자라는 중입니다. (성장도: " << age << "/3)\n";
        }
        pauseGame();
    }
};

class Livestock : public FarmEntity {
public:
    Livestock(int x, int y) : FarmEntity("닭", x, y, 'L') {}
    
    void grow() override {
        age++;
        readyToHarvest = true;
    }
    
    void interact(Player& p, Inventory<string>& inv) override {
        cout << "\n[축사(" << x << "," << y << ")] ";
        if (readyToHarvest) {
            cout << "달걀을 얻었습니다!\n";
            inv.add("달걀");
            p.gainExp(15);
            readyToHarvest = false;
        } else {
            cout << "오늘은 더 이상 달걀이 없습니다.\n";
        }
        pauseGame();
    }
};

// =========================================================
// Market 구현부
// =========================================================
void Market::sellProduce(Player& player, Inventory<string>& inv) {
    int count = 0;
    while (inv.use()) {
        player.money += 30; 
        count++;
    }
    cout << "\n[시장] 모든 생산물을 팔아 " << count * 30 << "G를 벌었습니다!\n";
    pauseGame();
}

void Market::buyEntity(Player& player, vector<FarmEntity*>& farm) {
    cout << "\n--- 🛒 상점 ---\n";
    cout << "1. 감자 씨앗 심기 (100G)\n";
    cout << "2. 병아리 입양 (200G)\n";
    cout << "0. 취소\n선택: ";
    int choice; cin >> choice;

    int newX = -1, newY = -1;
    for (int i = 2; i <= 6; i++) {
        for (int j = 2; j <= 8; j++) {
            bool occupied = false;
            for (auto e : farm) {
                if (e->x == j && e->y == i) occupied = true;
            }
            if (!occupied) { newX = j; newY = i; break; }
        }
        if (newX != -1) break;
    }

    if (newX == -1) {
        cout << "농장에 빈 자리가 없습니다!\n";
        pauseGame();
        return;
    }

    if (choice == 1 && player.money >= 100) {
        player -= 100; 
        farm.push_back(new Crop(newX, newY));
        cout << "[구입 완료] (" << newX << "," << newY << ")에 감자를 심었습니다!\n";
    } else if (choice == 2 && player.money >= 200) {
        player -= 200;
        farm.push_back(new Livestock(newX, newY));
        cout << "[구입 완료] (" << newX << "," << newY << ")에 닭을 들였습니다!\n";
    } else if (choice != 0) {
        cout << "잔액이 부족하거나 잘못된 입력입니다.\n";
    }
    pauseGame();
}

// =========================================================
// 미니게임
// =========================================================
void playMinigame(Player& p) {
    cout << "\n🎲 [마을 회관] 촌장님과의 주사위 도박 🎲\n";
    if (p.getMoney() < 50) {
        cout << "촌장: \"자네, 돈이 없구만! (50G 필요)\"\n";
        pauseGame();
        return;
    }
    
    p -= 50;
    int playerDice = (rand() % 6) + 1;
    int mayorDice = (rand() % 6) + 1;
    
    cout << "나의 주사위: " << playerDice << " | 촌장님의 주사위: " << mayorDice << "\n";
    
    if (playerDice > mayorDice) {
        cout << "촌장: \"허허, 자네가 이겼네. 150G를 가져가게!\"\n";
        p += 150;
    } else if (playerDice < mayorDice) {
        cout << "촌장: \"내가 이겼네! 돈은 잘 쓰겠네 하하!\"\n";
    } else {
        cout << "촌장: \"비겼구만. 판돈 50G는 돌려주지.\"\n";
        p += 50;
    }
    pauseGame();
}

// =========================================================
// 메인 함수
// =========================================================
const int MAP_WIDTH = 12;
const int MAP_HEIGHT = 8;
const char WALL = '#';

int main() {
    srand((unsigned int)time(NULL));

    Player player(5, 4);
    Market market;
    Inventory<string> storage; 
    vector<FarmEntity*> farm; 
    
    queue<string> disasterEvents;
    disasterEvents.push("가뭄");
    disasterEvents.push("전염병");
    disasterEvents.push("태풍");

    farm.push_back(new Crop(2, 2));

    string baseMap[MAP_HEIGHT] = {
        "############",
        "#H........M#",
        "#..........#",
        "#..........#",
        "#..........#",
        "#..........#",
        "#G.........#",
        "############"
    };

    while (true) {
        system(CLEAR_SCREEN); 
        
        cout << player << "\n";
        cout << " 📦 인벤토리 아이템: " << storage.count() << " 개\n";
        cout << " [안내] 이동: W/A/S/D | Q: 게임 종료\n";
        cout << " (H: 집, M: 시장, G: 촌장, C: 작물, L: 가축)\n\n";

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                bool drawn = false;
                
                if (x == player.x && y == player.y) {
                    cout << "@ ";
                    drawn = true;
                } else {
                    for (auto e : farm) {
                        if (e->x == x && e->y == y) {
                            cout << e->symbol << " ";
                            drawn = true;
                            break;
                        }
                    }
                }
                
                if (!drawn) {
                    cout << baseMap[y][x] << " ";
                }
            }
            cout << "\n";
        }

        int key = _getch();
        int nextX = player.x;
        int nextY = player.y;

        // W, A, S, D 이동 처리 (대소문자 모두 허용)
        if (key == 'w' || key == 'W') nextY--;
        else if (key == 's' || key == 'S') nextY++;
        else if (key == 'a' || key == 'A') nextX--;
        else if (key == 'd' || key == 'D') nextX++;
        else if (key == 'q' || key == 'Q') {
            cout << "\n게임을 종료합니다. 감사합니다!\n";
            break;
        } else {
            continue; // 다른 키 입력 무시
        }

        char targetTile = baseMap[nextY][nextX];
        
        if (targetTile == WALL) continue;

        bool entityInteracted = false;
        for (auto e : farm) {
            if (e->x == nextX && e->y == nextY) {
                e->interact(player, storage);
                entityInteracted = true;
                break;
            }
        }
        if (entityInteracted) continue;

        if (targetTile == 'H') {
            cout << "\n[침대] 하루를 마무리 하시겠습니까? (y/n): ";
            char ans; cin >> ans;
            if (ans == 'y' || ans == 'Y') {
                player.nextDay();
                for (auto e : farm) e->grow();
                cout << "\n... 새 아침이 밝았습니다 ...\n";
                
                if (rand() % 100 < 20 && !disasterEvents.empty()) {
                    string disaster = disasterEvents.front();
                    disasterEvents.pop(); 
                    cout << "⚠️ [경고] 자연재해 [" << disaster << "] 발생!! ⚠️\n";
                    for (auto e : farm) e->hitByDisaster();
                }
                pauseGame();
            }
            continue;
        }
        else if (targetTile == 'M') {
            cout << "\n[시장] 무엇을 하시겠습니까?\n1. 모두 팔기 | 2. 구매하기 | 0. 취소\n선택: ";
            int ans; cin >> ans;
            if (ans == 1) market.sellProduce(player, storage);
            else if (ans == 2) market.buyEntity(player, farm);
            continue;
        }
        else if (targetTile == 'G') {
            playMinigame(player);
            continue;
        }

        player.x = nextX;
        player.y = nextY;
    }

    for (auto e : farm) delete e;
    farm.clear();

    return 0;
}
