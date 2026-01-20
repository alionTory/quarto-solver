#include "negamax.h"
#include "MonteCarlo.h"
#include <iostream>
#include <set>
#include <array>
#include <unordered_map>
#include <fstream>

struct InputData
{
    bool isPiecePlaceStep;
    int selectedPiece;
};

InputData readInput(Board& board, std::set<int>& availablePieces)
{
    board.setBoardFromStdin();

    int availablePieceCount;
    std::cin >> availablePieceCount;
    for (int i = 0; i < availablePieceCount; i++)
    {
        int availablePiece;
        std::cin >> availablePiece;
        availablePieces.insert(availablePiece);
    }

    InputData input;
    std::cin >> input.isPiecePlaceStep;
    if (input.isPiecePlaceStep)
    {
        std::cin >> input.selectedPiece;
    }
    return input;
}

void start()
{
    static constexpr int NEGAMAX_START_DEPTH = 9;

    Board board;
    std::set<int> availablePieces;

    InputData inputData = readInput(board, availablePieces);

    if (board.getFilledCount() == 0)
    {
        if (inputData.isPiecePlaceStep)
            std::cout << "0, 1";
        else
            std::cout << 0;
    }
    else if (board.getFilledCount() * 2 + inputData.isPiecePlaceStep < NEGAMAX_START_DEPTH)
    {
        if (inputData.isPiecePlaceStep)
        {
            auto place = placePieceParallel(board, availablePieces, inputData.selectedPiece);
            std::cout << place[0] << ", " << place[1];
        }
        else
        {
            int solverSelect = selectPieceParallel(board, availablePieces);
            std::cout << solverSelect;
        }
    }
    else
    {
        Solver solver(board, availablePieces);
        using namespace std::chrono;
        steady_clock::time_point starttime, endtime;
        if (inputData.isPiecePlaceStep)
        {
            starttime = steady_clock::now();
            auto place = solver.placePiece(inputData.selectedPiece);
            endtime = steady_clock::now();
            std::cout << place.first << ", " << place.second;
        }
        else
        {
            starttime = steady_clock::now();
            int solverSelect = solver.selectPiece();
            endtime = steady_clock::now();
            std::cout << solverSelect;
        }
        std::cerr << "minimax time : " << duration_cast<milliseconds>(endtime - starttime).count() << '\n';
    }
}

void MCTSStart()
{
    Board board;
    std::set<int> availablePieces;

    InputData inputData = readInput(board, availablePieces);

    if (board.getFilledCount() == 0)
    {
        if (inputData.isPiecePlaceStep)
            std::cout << "0, 1";
        else
            std::cout << 0;
        return;
    }

    if (inputData.isPiecePlaceStep)
    {
        auto place = placePieceParallel(board, availablePieces, inputData.selectedPiece);
        std::cout << place[0] << ", " << place[1];
    }
    else
    {
        int solverSelect = selectPieceParallel(board, availablePieces);
        std::cout << solverSelect;
    }
}

int main()
{
    //MCTSStart();
    start();
    //takeSecondTurnCase();
    //system("pause");
}
