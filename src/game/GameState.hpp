#pragma once

enum class GameState {
    PLAYING,            // WASD 이동 중
    DIALOG_NOTIFY,      // 알림 메시지 표시 (엔티티 상호작용, 재해 등) - 아무 키 닫기
    DIALOG_SLEEP,       // H 타일: 수면 확인 (Y/N)
    DIALOG_MARKET_MENU, // M 타일: 시장 메뉴 (1.판매 2.구매 0.취소)
    DIALOG_MARKET_BUY,  // 구매 서브메뉴 (1.감자씨앗 2.닭 0.취소)
    DIALOG_MINIGAME,    // G 타일: 주사위 도박 결과 표시 - 아무 키 닫기
    GAME_OVER
};
