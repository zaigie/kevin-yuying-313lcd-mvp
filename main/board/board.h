#ifndef BOARD_H
#define BOARD_H

#include <string>

// Forward declarations
class Display;
class Backlight;

void *create_board();

class Board
{
private:
    Board(const Board &) = delete;            // 禁用拷贝构造函数
    Board &operator=(const Board &) = delete; // 禁用赋值操作

protected:
    Board();

public:
    static Board &GetInstance()
    {
        static Board *instance = static_cast<Board *>(create_board());
        return *instance;
    }

    virtual ~Board() = default;
    virtual std::string GetBoardType() = 0;
    virtual Backlight *GetBacklight() { return nullptr; }
    virtual Display *GetDisplay() { return nullptr; }
};

#define DECLARE_BOARD(BOARD_CLASS_NAME) \
    void *create_board()                \
    {                                   \
        return new BOARD_CLASS_NAME();  \
    }

#endif // BOARD_H