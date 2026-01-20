#include "MonteCarlo.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <algorithm>
#include <future>
#include <map>

std::atomic<int> totalLoopCount = 0;

MCTNodeSelected::MCTNodeSelected(int selectedPiece, const Board& currentBoard)
    :selectedPiece(selectedPiece)
{
    for (int row = 0; row < BOARD_ROWS; row++)
    {
        for (int col = 0; col < BOARD_COLS; col++)
        {
            if (currentBoard.get(row, col) == -1)
            {
                unexploredMoves.push_back({ row, col });
            }
        }
    }
}

void MCTNodeSelected::expandChild(const std::array<int, 2>& selectedMove, const std::set<int>& availablePieces)
{
    children.emplace_back(selectedMove[0], selectedMove[1], availablePieces);
    auto iterForRemove = std::find(unexploredMoves.begin(), unexploredMoves.end(), selectedMove);
    unexploredMoves.erase(iterForRemove);
}


MCTNodePlaced::MCTNodePlaced(int selectedRow, int selectedCol, const std::set<int>& availablePieces)
    :selectedRow(selectedRow), selectedCol(selectedCol), unexploredMoves(availablePieces.begin(), availablePieces.end())
{
}

void MCTNodePlaced::expandChild(int selectedPiece, const Board& currentBoard)
{
    children.emplace_back(selectedPiece, currentBoard);
    auto iterForRemove = std::find(unexploredMoves.begin(), unexploredMoves.end(), selectedPiece);
    unexploredMoves.erase(iterForRemove);
}

MCSolver::MCSolver(const Board& board, const std::set<int>& availablePieces)
    : board(board), availablePieces(availablePieces)
{
}

std::map<int, double> MCSolver::selectPiece()
{
    root = std::make_unique<MCTNodePlaced>(-1, -1, availablePieces);
    MCTNodePlaced& rootCasted = dynamic_cast<MCTNodePlaced&>(*root);

    int loopCount = 0;
    using namespace std::chrono;
    startTime = steady_clock::now();
    // 5번째 piece 선택이 중요
    const int timeoutMs = board.getFilledCount() == 4 ? TIMEOUT_MS_LONG : TIMEOUT_MS;
    while (duration_cast<milliseconds>(steady_clock::now() - startTime).count() < timeoutMs)
    {
        selectNodeAndBackpropagate(rootCasted);
        loopCount++;
    }

    totalLoopCount += loopCount;

    std::map<int, double> result;
    for (const auto& child : rootCasted.children)
    {
        result[child.selectedPiece] = child.playoutCount;
    }

    return result;
}

std::map<std::array<int, 2>, double> MCSolver::placePiece(int selectedPiece)
{
    auto iterToRemove = availablePieces.find(selectedPiece);
    availablePieces.erase(iterToRemove);

    root = std::make_unique<MCTNodeSelected>(selectedPiece, board);
    MCTNodeSelected& rootCasted = dynamic_cast<MCTNodeSelected&>(*root);

    int loopCount = 0;
    using namespace std::chrono;
    startTime = steady_clock::now();
    // 4번째 piece place가 중요
    const int timeoutMs = board.getFilledCount() == 3 ? TIMEOUT_MS_LONG : TIMEOUT_MS;
    while (duration_cast<milliseconds>(steady_clock::now() - startTime).count() < timeoutMs)
    {
        selectNodeAndBackpropagate(rootCasted);
        loopCount++;
    }

    totalLoopCount += loopCount;

    std::map<std::array<int, 2>, double> result;
    for (const auto& child : rootCasted.children)
    {
        result[{child.selectedRow, child.selectedCol}] = child.playoutCount;
    }

    return result;
}

constexpr double UCB1Constant = 1.41;
double UCB1(double playoutCount, double score, double parentPlayoutCount)
{
    return score / playoutCount + UCB1Constant * std::sqrt(std::log(parentPlayoutCount) / playoutCount);
}

double MCSolver::selectNodeAndBackpropagate(MCTNodeSelected& selectedNode)
{
    double playoutResult;
    if (selectedNode.playoutCount == 0)
    {
        playoutResult = -playoutPlace(selectedNode.selectedPiece);
        selectedNode.score += playoutResult;
        selectedNode.playoutCount++;
        return playoutResult;
    }


    MCTNodePlaced* nextNode;

    if (!selectedNode.unexploredMoves.empty())
    {
        std::vector<std::array<int, 2>> randomSelectResource(1);
        std::sample(selectedNode.unexploredMoves.begin(), selectedNode.unexploredMoves.end(), randomSelectResource.begin(), 1, randomEngine);
        std::array<int, 2> selectedMove = randomSelectResource[0];
        selectedNode.expandChild(selectedMove, availablePieces);
        nextNode = &selectedNode.children.back();
    }
    else
    {
        double maxUCB1 = std::numeric_limits<double>::lowest();
        MCTNodePlaced* maxUCB1Child = nullptr;
        for (auto& child : selectedNode.children)
        {
            double currentUCB1 = UCB1(child.playoutCount, child.score, selectedNode.playoutCount);
            if (currentUCB1 > maxUCB1)
            {
                maxUCB1 = currentUCB1;
                maxUCB1Child = &child;
            }
        }
        nextNode = maxUCB1Child;
    }

    board.set(nextNode->selectedRow, nextNode->selectedCol, selectedNode.selectedPiece);
    playoutResult = -selectNodeAndBackpropagate(*nextNode);
    board.set(nextNode->selectedRow, nextNode->selectedCol, -1);

    selectedNode.score += playoutResult;
    selectedNode.playoutCount++;
    return playoutResult;
}

double MCSolver::selectNodeAndBackpropagate(MCTNodePlaced& selectedNode)
{
    double playoutResult;

    if (board.isWinnerExist())
    {
        playoutResult = 1;
    }
    else if (board.isFull())
    {
        playoutResult = 0;
    }
    else if (selectedNode.playoutCount == 0)
    {
        auto removeIter = std::remove_if(selectedNode.unexploredMoves.begin(), selectedNode.unexploredMoves.end(),
            [this](int unexploredMove) {return board.hasTerminatorTrait(unexploredMove); });
        selectedNode.unexploredMoves.erase(removeIter, selectedNode.unexploredMoves.end());

        playoutResult = playoutSelect();
    }
    else if (selectedNode.unexploredMoves.empty() && selectedNode.children.empty())
    {
        // unexploredmove와 child가 둘 다 비어 있다면, 선택할 수 있는 piece가 모두 terminator라는 뜻.
        // 따라서, 어떤 piece를 선택해도 패배하게 된다.
        playoutResult = -1;
    }
    else {
        MCTNodeSelected* nextNode;
        if (!selectedNode.unexploredMoves.empty())
        {
            std::vector<int> randomSelectResource(1);
            std::sample(selectedNode.unexploredMoves.begin(), selectedNode.unexploredMoves.end(), randomSelectResource.begin(), 1, randomEngine);
            int selectedMove = randomSelectResource[0];
            selectedNode.expandChild(selectedMove, board);
            nextNode = &selectedNode.children.back();
        }
        else
        {
            // (!selectedNode.children.empty())
            double maxUCB1 = std::numeric_limits<double>::lowest();
            MCTNodeSelected* maxUCB1Child = nullptr;
            for (auto& child : selectedNode.children)
            {
                double currentUCB1 = UCB1(child.playoutCount, child.score, selectedNode.playoutCount);
                if (currentUCB1 > maxUCB1)
                {
                    maxUCB1 = currentUCB1;
                    maxUCB1Child = &child;
                }
            }
            nextNode = maxUCB1Child;
        }

        auto iterToRemove = availablePieces.find(nextNode->selectedPiece);
        availablePieces.erase(iterToRemove);
        playoutResult = selectNodeAndBackpropagate(*nextNode);
        availablePieces.insert(nextNode->selectedPiece);
    }

    selectedNode.score += playoutResult;
    selectedNode.playoutCount++;
    return playoutResult;
}

double MCSolver::playoutSelect()
{
    if (board.isWinnerExist())
        return 1;
    if (board.isFull())
        return 0;

    std::vector<int> nonTerminatorPieces;
    for (int piece : availablePieces)
    {
        if (!board.hasTerminatorTrait(piece))
        {
            nonTerminatorPieces.push_back(piece);
        }
    }

    if (nonTerminatorPieces.empty())
        return -1;

    std::sample(nonTerminatorPieces.begin(), nonTerminatorPieces.end(), nonTerminatorPieces.begin(), 1, randomEngine);
    auto removeIter = std::find(availablePieces.begin(), availablePieces.end(), nonTerminatorPieces[0]);
    availablePieces.erase(removeIter);
    double result = -playoutPlace(nonTerminatorPieces[0]);
    availablePieces.insert(nonTerminatorPieces[0]);
    return result;
}

double MCSolver::playoutPlace(int selectedPiece)
{
    std::vector<std::array<int, 2>> emptyPlaces;
    for (int row = 0; row < BOARD_ROWS; row++)
    {
        for (int col = 0; col < BOARD_COLS; col++)
        {
            if (board.get(row, col) == -1)
            {
                emptyPlaces.push_back({ row, col });
            }
        }
    }

    std::sample(emptyPlaces.begin(), emptyPlaces.end(), emptyPlaces.begin(), 1, randomEngine);
    board.set(emptyPlaces[0][0], emptyPlaces[0][1], selectedPiece);
    double result = playoutSelect();
    board.set(emptyPlaces[0][0], emptyPlaces[0][1], -1);
    return result;
}


int selectPieceParallel(const Board& board, const std::set<int>& availablePieces)
{
    std::vector<MCSolver> MCSSolvers;
    std::vector<std::thread> threads;
    std::vector<std::future<std::map<int, double>>> futures;
    MCSSolvers.reserve(MCTS_THREAD_COUNT);
    threads.reserve(MCTS_THREAD_COUNT);
    futures.reserve(MCTS_THREAD_COUNT);
    for (int i = 0; i < MCTS_THREAD_COUNT; i++)
    {
        MCSSolvers.emplace_back(board, availablePieces);
        std::packaged_task<std::map<int, double>(MCSolver*)> task{ &MCSolver::selectPiece };
        futures.emplace_back(task.get_future());
        threads.emplace_back(std::move(task), &MCSSolvers.back());
    }

    auto startTime = std::chrono::steady_clock::now();

    std::map<int, double> threadResultsSum;
    for (int i = 0; i < MCTS_THREAD_COUNT; i++)
    {
        auto threadResult = futures[i].get();
        for (const auto [piece, playoutCount] : threadResult)
        {
            if (threadResultsSum.find(piece) == threadResultsSum.end())
                threadResultsSum[piece] = playoutCount;
            else
                threadResultsSum[piece] += playoutCount;
        }
        threads[i].join();
    }

    std::cerr << "totalLoopCount : " << totalLoopCount << '\n';
    std::cerr << "spend time(ms) : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() << '\n';

    int bestPiece = -1;
    double maxVisitCount = 0;
    for (const auto& [piece, playoutCount] : threadResultsSum)
    {
        if (playoutCount > maxVisitCount)
        {
            maxVisitCount = playoutCount;
            bestPiece = piece;
        }
    }

    // terminator piece만 남았을 때 -1이 리턴되는 것 방지
    if (bestPiece == -1)
    {
        std::mt19937 randomEngine{ std::random_device{}() };
        std::array<int, 1> sampleOutput;
        std::sample(availablePieces.begin(), availablePieces.end(), sampleOutput.begin(), 1, randomEngine);
        bestPiece = sampleOutput[0];
    }

    return bestPiece;
}

std::array<int, 2> placePieceParallel(const Board& board, const std::set<int>& availablePieces, int selectedPiece)
{
    std::vector<MCSolver> MCSSolvers;
    std::vector<std::thread> threads;
    std::vector<std::future<std::map<std::array<int, 2>, double>>> futures;
    MCSSolvers.reserve(MCTS_THREAD_COUNT);
    threads.reserve(MCTS_THREAD_COUNT);
    futures.reserve(MCTS_THREAD_COUNT);
    for (int i = 0; i < MCTS_THREAD_COUNT; i++)
    {
        MCSSolvers.emplace_back(board, availablePieces);
        std::packaged_task<std::map<std::array<int, 2>, double>(MCSolver*, int)> task{ &MCSolver::placePiece };
        futures.emplace_back(task.get_future());
        threads.emplace_back(std::move(task), &MCSSolvers.back(), selectedPiece);
    }

    auto startTime = std::chrono::steady_clock::now();

    std::map<std::array<int, 2>, double> threadResultsSum;
    for (int i = 0; i < MCTS_THREAD_COUNT; i++)
    {
        auto threadResult = futures[i].get();
        for (const auto [place, playoutCount] : threadResult)
        {
            if (threadResultsSum.find(place) == threadResultsSum.end())
                threadResultsSum[place] = playoutCount;
            else
                threadResultsSum[place] += playoutCount;
        }
        threads[i].join();
    }

    std::cerr << "totalLoopCount : " << totalLoopCount << '\n';
    std::cerr << "spend time(ms) : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() << '\n';

    std::array<int, 2> bestPlace = { -1, -1 };
    double maxVisitCount = 0;
    for (const auto& [place, playoutCount] : threadResultsSum)
    {
        if (playoutCount > maxVisitCount)
        {
            maxVisitCount = playoutCount;
            bestPlace = place;
        }
    }

    return bestPlace;
}
