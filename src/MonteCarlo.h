#pragma once
#include <chrono>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <vector>
#include "Board.h"

struct MCTNode
{
    double playoutCount = 0;
    double score = 0;
    virtual ~MCTNode() = default;
};

struct MCTNodePlaced;

struct MCTNodeSelected :MCTNode
{
    int selectedPiece;
    std::vector<MCTNodePlaced> children;
    std::vector<std::array<int, 2>> unexploredMoves;

    MCTNodeSelected(int selectedPiece, const Board& currentBoard);
    void expandChild(const std::array<int, 2>& selectedMove, const std::set<int>& availablePieces);
};

struct MCTNodePlaced :MCTNode
{
    int selectedRow;
    int selectedCol;
    std::vector<MCTNodeSelected> children;
    std::vector<int> unexploredMoves;

    MCTNodePlaced(int selectedRow, int selectedCol, const std::set<int>& availablePieces);
    void expandChild(int selectedPiece, const Board& currentBoard);
};

class MCSolver
{
private:
    static inline std::random_device randomDevice;
    std::mt19937 randomEngine{randomDevice()};
    std::unique_ptr<MCTNode> root;
    Board board;
    std::set<int> availablePieces;
    static const int TIMEOUT_MS = 1000 * 5;
    static const int TIMEOUT_MS_LONG = 1000 * 20;
    std::chrono::steady_clock::time_point startTime;
public:
    MCSolver(const Board& board, const std::set<int>& availablePieces);

    std::map<int, double> selectPiece();
    std::map<std::array<int, 2>, double> placePiece(int selectedPiece);

    double selectNodeAndBackpropagate(MCTNodeSelected& selectedNode);
    double selectNodeAndBackpropagate(MCTNodePlaced& selectedNode);

    double playoutSelect();
    double playoutPlace(int selectedPiece);
};

extern std::atomic<int> totalLoopCount;
const int MCTS_THREAD_COUNT = 24;
int selectPieceParallel(const Board& board, const std::set<int>& availablePieces);
std::array<int, 2> placePieceParallel(const Board& board, const std::set<int>& availablePieces, int selectedPiece);
