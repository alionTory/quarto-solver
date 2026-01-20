#pragma once
#include <array>
#include <bitset>
#include <vector>
constexpr int BOARD_ROWS = 4;
constexpr int BOARD_COLS = 4;
constexpr int PIECE_COUNT = 16;
constexpr int TRAIT_COUNT = 4;

using Matrix = std::vector<std::vector<int>>;
class Board
{
private:
    std::array<std::array<int, 2>, TRAIT_COUNT> terminatorPlaceCount{};
    std::vector<std::vector<int>> board{ BOARD_ROWS, std::vector<int>(BOARD_COLS) };
    int filledCount = 0;
    bool m_isWinnerExist = false;

    static std::vector<std::vector<int>> copyRotateRight(const std::vector<std::vector<int>>& originalBoard);
    static std::vector<std::vector<int>> copyMirrorUpDown(const std::vector<std::vector<int>>& originalBoard);
    static int getBoardPlaced16Bit(const Matrix& board);
    static std::vector<int> permutation4Bit(int original);
    static void permutation4BitRecursive(std::bitset<4> current, int nextPlacedIndex, std::bitset<4> original, std::bitset<4> used, std::vector<int>& result);
    static long long getCompactExpression(const std::bitset<17 * 5>& board);

    void checkLines(int changedRow, int changedCol, int select);
    bool check2x2(int changedRow, int changedCol, int dRow, int dCol, int select);
    // called before board field change
    void checkAddInLine(std::vector<int> line, int pieceToAdd);
    // called before board field change
    void checkRemoveInLine(std::vector<int> line, int pieceToRemove);

public:
    Board();
    void print() const;

    long long getNormalized(int select) const;

    int get(int row, int col) const;
    void setBoardFromStdin();
    void set(int row, int col, int select);

    bool isFull() const;
    bool hasTerminatorTrait(int piece) const;
    std::array<int, 2> getTerminatingPlace(int terminatingPiece);
    bool isWinnerExist() const;
    int getFilledCount() const;
};
