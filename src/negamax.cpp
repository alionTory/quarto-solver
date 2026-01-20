#include "negamax.h"

#include <iostream>
#include <fstream>


Solver::Solver(const Board& board, const std::set<int>& availablePieces)
    :board(board), availablePieces(availablePieces)
{
    if (LOAD_CACHE_FILE)
        loadCacheFile();
}

Solver::~Solver()
{
    if (SAVE_CACHE_FILE)
        saveCacheFile();
}

void Solver::init(const Board& board, const std::set<int>& availablePieces)
{
    this->board = board;
    this->availablePieces = availablePieces;
}

void Solver::saveCacheFile()
{
    std::cerr << "saving cache\n";
    std::cerr << "saving cache count : " << caches.size() << '\n';
    std::ofstream file{ CACHE_FILE_NAME, std::ios_base::binary };
    for (const auto& cacheData : caches)
    {
        file.write(reinterpret_cast<const char*>(&cacheData.first), sizeof(cacheData.first));
        file.write(reinterpret_cast<const char*>(&cacheData.second.lowerBound), sizeof(cacheData.second.lowerBound));
        file.write(reinterpret_cast<const char*>(&cacheData.second.upperBound), sizeof(cacheData.second.upperBound));
    }
    std::cerr << "cache saved\n";
}

void Solver::loadCacheFile()
{
    std::cerr << "loading cache\n";
    std::ifstream file{ CACHE_FILE_NAME, std::ios_base::binary };
    while (file)
    {
        long long key;
        CacheValue cacheValue;
        file.read(reinterpret_cast<char*>(&key), sizeof(key));
        file.read(reinterpret_cast<char*>(&cacheValue.lowerBound), sizeof(cacheValue.lowerBound));
        file.read(reinterpret_cast<char*>(&cacheValue.upperBound), sizeof(cacheValue.upperBound));
        caches[key] = cacheValue;
    }
    std::cerr << "cache loaded\n";
    std::cerr << "loaded cache count : " << caches.size() << '\n';
}

bool Solver::readCache(int select, long long& normalizedBoard, Utility& alpha, Utility& beta, Utility& bestChildMinimax)
{
    normalizedBoard = board.getNormalized(select);
    auto cacheFound = caches.find(normalizedBoard);
    const auto& cacheValue = cacheFound->second;
    if (cacheFound != caches.end()) {
        if (cacheValue.lowerBound == cacheValue.upperBound) {
            bestChildMinimax = cacheValue.lowerBound;
            return true;
        }
        else if (beta <= cacheValue.lowerBound) {
            bestChildMinimax = cacheValue.lowerBound;
            return true;
        }
        else if (cacheValue.upperBound <= alpha) {
            bestChildMinimax = cacheValue.upperBound;
            return true;
        }
        else {
            alpha = std::max(alpha, cacheValue.lowerBound);
            beta = std::min(beta, cacheValue.upperBound);
            return false;
        }
    }
    return false;
}

void Solver::saveCache(long long normalizedBoard, Utility bestChildMinimax, Utility alphaOrig, Utility beta)
{
    CacheValue newCacheValue{ UTILITY_MIN, UTILITY_MAX };

    auto cacheIter = caches.find(normalizedBoard);
    if (cacheIter != caches.end())
    {
        newCacheValue.lowerBound = cacheIter->second.lowerBound;
        newCacheValue.upperBound = cacheIter->second.upperBound;
    }

    if (bestChildMinimax <= alphaOrig)
        newCacheValue.upperBound = std::min(newCacheValue.upperBound, bestChildMinimax);
    else if (beta <= bestChildMinimax)
        newCacheValue.lowerBound = std::max(newCacheValue.lowerBound, bestChildMinimax);
    else
        newCacheValue.upperBound = newCacheValue.lowerBound = bestChildMinimax;

    if (cacheIter == caches.end())
        caches.insert({ normalizedBoard, newCacheValue });
    else
        cacheIter->second = newCacheValue;
}

Utility Solver::negamaxSelect(Utility alpha, Utility beta)
{
    // check terminal state
    if (board.isWinnerExist())
        return WIN;
    if (availablePieces.empty())
        return DRAW;

    Utility bestChildMinimax = UTILITY_MIN;

    // read cache
    long long normalizedBoard;
    bool cacheFound = false;
    if (board.getFilledCount() * 2 < unNomarlizedDepth) {
        if (readCache(-1, normalizedBoard, alpha, beta, bestChildMinimax)) {
            return bestChildMinimax;
        }
    }

    Utility alphaOrig = alpha;

    // calculate child minimax
    for (auto iter = availablePieces.begin(); iter != availablePieces.end(); ++iter)
    {
        int availablePiece = *iter;
        Utility childMinimax;
        if (board.hasTerminatorTrait(availablePiece))
        {
            childMinimax = LOSS;
        }
        else {
            childMinimax = static_cast<Utility>(-negamaxPlace(availablePiece, static_cast<Utility>(-beta), static_cast<Utility>(-alpha)));
            // iter 무효화 방지
            iter = availablePieces.find(availablePiece);
        }

        if (childMinimax > bestChildMinimax)
        {
            bestChildMinimax = childMinimax;
            if (bestChildMinimax >= beta)
                break;
            alpha = std::max(alpha, bestChildMinimax);
        }
    }

    // save cache
    if (board.getFilledCount() * 2 < unNomarlizedDepth)
        saveCache(normalizedBoard, bestChildMinimax, alphaOrig, beta);

    return bestChildMinimax;
}

Utility Solver::negamaxPlace(int selectedPiece, Utility alpha, Utility beta)
{
    Utility bestChildMinimax = UTILITY_MIN;

    // read cache
    long long normalizedBoard;
    if (board.getFilledCount() * 2 + 1 < unNomarlizedDepth) {
        if (readCache(selectedPiece, normalizedBoard, alpha, beta, bestChildMinimax))
            return bestChildMinimax;
    }

    Utility alphaOrig = alpha;

    availablePieces.erase(selectedPiece);
    // erase 했으므로 insert로 복구하기 전까지 return되면 안됨

    for (int row = 0; row < BOARD_ROWS; row++)
    {
        for (int col = 0; col < BOARD_COLS; col++)
        {
            if (board.get(row, col) == -1)
            {
                board.set(row, col, selectedPiece);
                Utility childMinimax = negamaxSelect(alpha, beta);
                board.set(row, col, -1);

                if (childMinimax > bestChildMinimax)
                {
                    bestChildMinimax = childMinimax;
                    if (bestChildMinimax >= beta)
                        goto loopBreak;
                    alpha = std::max(alpha, bestChildMinimax);
                }
            }
        }
    }
loopBreak:

    availablePieces.insert(selectedPiece);

    // save cache
    if (board.getFilledCount() * 2 + 1 < unNomarlizedDepth) {
        saveCache(normalizedBoard, bestChildMinimax, alphaOrig, beta);
    }
    return bestChildMinimax;
}

int Solver::selectPiece()
{
    int bestPiece;
    Utility bestChildMinimax = UTILITY_MIN;
    Utility alpha = LOSS;
    Utility beta = WIN;

    for (auto iter = availablePieces.begin(); iter != availablePieces.end(); ++iter)
    {
        int availablePiece = *iter;
        Utility childMinimax;
        if (board.hasTerminatorTrait(availablePiece))
        {
            childMinimax = LOSS;
        }
        else {
            childMinimax = static_cast<Utility>(-negamaxPlace(availablePiece, static_cast<Utility>(-beta), static_cast<Utility>(-alpha)));
            iter = availablePieces.find(availablePiece);
        }

        std::cerr << "availablePiece : " << availablePiece << ", minimax : " << static_cast<int>(childMinimax) << '\n';

        if (childMinimax > bestChildMinimax)
        {
            bestChildMinimax = childMinimax;
            bestPiece = availablePiece;
            if (bestChildMinimax == WIN)
                break;
            alpha = std::max(alpha, bestChildMinimax);
        }
    }

    return bestPiece;
}

std::pair<int, int> Solver::placePiece(int selectedPiece)
{
    availablePieces.erase(selectedPiece);

    std::pair bestPlace = { 0,0 };
    Utility bestChildMinimax = UTILITY_MIN;
    Utility alpha = LOSS;
    Utility beta = WIN;
    //if (board.getFilledCount() <= 3)
    //    beta = DRAW;

    if (board.hasTerminatorTrait(selectedPiece)) {
        std::array<int, 2> terminatorPlace = board.getTerminatingPlace(selectedPiece);
        bestPlace.first = terminatorPlace[0];
        bestPlace.second = terminatorPlace[1];
        bestChildMinimax = WIN;
        goto loopBreak;
    }
    for (int row = 0; row < BOARD_ROWS; row++)
    {
        for (int col = 0; col < BOARD_COLS; col++)
        {
            if (board.get(row, col) == -1)
            {
                board.set(row, col, selectedPiece);
                Utility childMinimax = negamaxSelect(alpha, beta);
                board.set(row, col, -1);

                std::cerr << "row : " << row << ", col : " << col << ", minimax : " << static_cast<int>(childMinimax) << '\n';

                if (childMinimax > bestChildMinimax)
                {
                    bestChildMinimax = childMinimax;
                    bestPlace.first = row;
                    bestPlace.second = col;
                    if (bestChildMinimax == WIN)
                        return bestPlace;
                    alpha = std::max(alpha, bestChildMinimax);
                }
            }
        }
    }
loopBreak:
    return bestPlace;
}