#pragma once
#include <set>
#include <unordered_map>
#include "Board.h"

enum Utility :signed char { UTILITY_MIN = -2, LOSS = -1, DRAW = 0, WIN = 1, UTILITY_MAX = 2 };

struct CacheValue
{
    Utility lowerBound;
    Utility upperBound;
};

class Solver
{
private:
    Board board;
    std::set<int> availablePieces;
    const size_t CACHE_MEMORY_SIZE = 1024 * 1024 * 1024;
    static constexpr int unNomarlizedDepth = 0;
    std::unordered_map<long long, CacheValue> caches;
    inline static const std::string CACHE_FILE_NAME = "cacheFile";
    static constexpr bool SAVE_CACHE_FILE = false;
    static constexpr bool LOAD_CACHE_FILE = false;

    bool readCache(int select, long long& normalizedBoard, Utility& alpha, Utility& beta, Utility& bestChildMinimax);
    void saveCache(long long normalizedBoard, Utility bestChildMinimax, Utility alphaOrig, Utility beta);

    Utility negamaxSelect(Utility alpha, Utility beta);
    Utility negamaxPlace(int selectedPiece, Utility alpha, Utility beta);

public:
    Solver(const Board& board, const std::set<int>& availablePieces);
    ~Solver();
    void init(const Board& board, const std::set<int>& availablePieces);

    void saveCacheFile();
    void loadCacheFile();

    int selectPiece();
    std::pair<int, int> placePiece(int selectedPiece);
};

