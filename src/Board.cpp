#include "Board.h"
#include <algorithm>
#include <iostream>

std::bitset<4> getSamePieceTraits(const std::vector<int>& pieces)
{
    std::bitset<4> result = 0b1111;
    for (int i = 1; i < pieces.size(); i++)
    {
        result &= ~(pieces[i - 1] ^ pieces[i]);
    }
    return result;
}

Board::Board()
{
    for (auto& row : board)
    {
        for (auto& element : row)
        {
            element = -1;
        }
    }
}

void Board::print() const
{
    for (const auto& row : board)
    {
        for (int element : row)
        {
            std::cout << element << ' ';
        }
        std::cout << '\n';
    }
}

std::vector<std::vector<int>> Board::copyRotateRight(const std::vector<std::vector<int>>& originalBoard) {
    std::vector<std::vector<int>> result{ BOARD_ROWS, std::vector<int>(BOARD_COLS) };
    for (int rowIndex = 0; rowIndex < BOARD_ROWS; rowIndex++)
    {
        for (int colIndex = 0; colIndex < BOARD_COLS; colIndex++)
        {
            result[colIndex][BOARD_ROWS - 1 - rowIndex] = originalBoard[rowIndex][colIndex];
        }
    }
    return result;
}

std::vector<std::vector<int>> Board::copyMirrorUpDown(const std::vector<std::vector<int>>& originalBoard) {
    std::vector<std::vector<int>> result{ BOARD_ROWS, std::vector<int>(BOARD_COLS) };
    for (int i = 0; i < 4; i++)
    {
        std::copy(originalBoard[i].begin(), originalBoard[i].end(), result[3 - i].begin());
    }
    return result;
}

int Board::getBoardPlaced16Bit(const Matrix& board)
{
    int result = 0;
    for (const auto& row : board)
    {
        for (int element : row)
        {
            result << 1;
            if (element != -1)
                result++;
        }
    }
    return result;
}

std::vector<int> Board::permutation4Bit(int original)
{
    std::vector<int> result;
    permutation4BitRecursive(0, 0, original, 0, result);
    return result;
}

void Board::permutation4BitRecursive(std::bitset<4> current, int nextPlacedIndex, std::bitset<4> original, std::bitset<4> used, std::vector<int>& result)
{
    if (nextPlacedIndex == 4)
    {
        result.push_back(current.to_ulong());
    }
    for (int i = 0; i < 4; i++)
    {
        if (used[i] == false)
        {
            std::bitset<4> temp = current;
            std::bitset<4> usedCopy = used;
            usedCopy[i] = true;
            temp[nextPlacedIndex] = original[i];
            permutation4BitRecursive(temp, nextPlacedIndex + 1, original, usedCopy, result);
        }
    }
}

long long Board::getCompactExpression(const std::bitset<17 * 5>& board)
{
    std::vector<std::bitset<5>> boardSplited(17);
    for (int splitIndex = 0; splitIndex < 17; splitIndex++)
    {
        for (int traitIndex = 0; traitIndex < 5; traitIndex++)
        {
            boardSplited[16 - splitIndex][traitIndex] = board[splitIndex * 5 + traitIndex];
        }
    }

    long long result = 0;
    // write 17bit flag
    for (const auto& aSplit : boardSplited)
    {
        result <<= 1;
        if (!aSplit[4])
            result++;
    }

    int writtenBitCount = 17;

    // write piece type;
    std::vector<unsigned long> remainPieces(15);
    bool isZeroPieceUsed = false;
    int neededBitCountForPiece = 4;
    for (int i = 0; i < 15; i++)
    {
        remainPieces[i] = i + 1;
    }
    for (int i = 0; i < 17; i++)
    {
        if (!boardSplited[i][4])
        {
            if (!isZeroPieceUsed)
            {
                isZeroPieceUsed = true;
            }
            else if (remainPieces.size() > 1)
            {
                unsigned long currentPieceToExpress = boardSplited[i].to_ulong();
                auto foundIndexIter = std::find(remainPieces.begin(), remainPieces.end(), currentPieceToExpress);
                unsigned long foundIndex = foundIndexIter - remainPieces.begin();
                remainPieces.erase(foundIndexIter);

                result <<= neededBitCountForPiece;
                result += foundIndex;
                writtenBitCount += neededBitCountForPiece;

                switch (remainPieces.size())
                {
                case 8:
                    neededBitCountForPiece = 3;
                    break;
                case 4:
                    neededBitCountForPiece = 2;
                    break;
                case 2:
                    neededBitCountForPiece = 1;
                    break;
                }
            }
        }
    }

    result <<= 64 - writtenBitCount;
    return result;
}

long long Board::getNormalized(int select) const
{
    // make board place symmetrics
    std::vector<std::pair<int, Matrix>> placeSymetrics;
    placeSymetrics.reserve(8);
    placeSymetrics.push_back({ getBoardPlaced16Bit(board),board });
    for (int i = 0; i < 3; i++)
    {
        Matrix rotated = copyRotateRight(placeSymetrics[i].second);
        int rotatedBoardPlace16Bit = getBoardPlaced16Bit(rotated);
        placeSymetrics.push_back({ rotatedBoardPlace16Bit, std::move(rotated) });
    }
    for (int i = 0; i < 4; i++)
    {
        Matrix mirrored = copyMirrorUpDown(placeSymetrics[i].second);
        int mirroredBoardPlace16Bit = getBoardPlaced16Bit(mirrored);
        placeSymetrics.push_back({ mirroredBoardPlace16Bit, std::move(mirrored) });
    }

    std::sort(placeSymetrics.begin(), placeSymetrics.end(), [](const auto& element1, const auto& element2)
        {
            return element1.first < element2.first;
        });

    std::vector<Matrix> candidates;
    int largestPlace16Bit = placeSymetrics.back().first;
    candidates.push_back(placeSymetrics.back().second);
    placeSymetrics.pop_back();
    while (!placeSymetrics.empty() && placeSymetrics.back().first == largestPlace16Bit)
    {
        candidates.push_back(placeSymetrics.back().second);
        placeSymetrics.pop_back();
    }

    // piece XOR
    for (auto& candidate : candidates)
    {
        int firstPiece = -1;
        if (select != -1)
        {
            firstPiece = select;
        }
        for (auto& row : candidate)
        {
            for (auto& piece : row)
            {
                if (piece != -1) {
                    if (firstPiece == -1)
                    {
                        firstPiece = piece;
                        piece ^= firstPiece;
                    }
                    else
                    {
                        piece ^= firstPiece;
                    }
                }
            }
        }
    }

    // permutate Piece
    std::vector<std::bitset<17 * 5>> permutatedStoredTotal;
    for (auto& candidate : candidates)
    {
        std::vector<std::bitset<17 * 5>> permutatedStored(24);
        for (auto& aStored : permutatedStored)
        {
            aStored <<= 5;
            if (select == -1)
                aStored[4] = true;
        }
        for (auto& row : candidate)
        {
            for (int element : row)
            {
                if (element == -1)
                {
                    for (int i = 0; i < 24; i++)
                    {
                        permutatedStored[i] <<= 5;
                        permutatedStored[i][4] = true;
                    }
                }
                else {
                    auto permutatedElement = permutation4Bit(element);
                    for (int i = 0; i < 24; i++)
                    {
                        permutatedStored[i] <<= 5;
                        for (int j = 0; j < 4; j++)
                        {
                            permutatedStored[i][j] = (permutatedElement[i] & (0b1 << j)) > 0;
                        }
                    }
                }
            }
        }
        permutatedStoredTotal.insert(permutatedStoredTotal.end(), permutatedStored.begin(), permutatedStored.end());
    }

    auto uncompactResult = std::max_element(permutatedStoredTotal.begin(), permutatedStoredTotal.end(), [](const auto& par1, const auto& par2)
        {
            for (int i = 84; i >= 0; i--)
            {
                if (par1[i] < par2[i])
                    return true;
                else if (par1[i] > par2[i])
                    return false;
            }
            return false;
        });
    return getCompactExpression(*uncompactResult);
}

int Board::get(int row, int col) const
{
    return board[row][col];
}

void Board::setBoardFromStdin()
{
    for (int row = 0; row < BOARD_ROWS; row++)
    {
        for (int col = 0; col < BOARD_ROWS; col++)
        {
            int input;
            std::cin >> input;
            set(row, col, input);
        }
    }
}

void Board::set(int row, int col, int select)
{
    if (board[row][col] == select)
        return;

    checkLines(row, col, select);
    if (select == -1)
    {
        filledCount--;
    }
    else if (select != -1 && board[row][col] == -1)
    {
        filledCount++;
    }
    board[row][col] = select;
}

bool Board::isFull() const
{
    //for (const auto& row : board)
    //    for (const auto& element : row)
    //        if (element == -1)
    //            return false;
    return filledCount == PIECE_COUNT;
}

void Board::checkLines(int changedRow, int changedCol, int select)
{
    // check row
    (select == -1) ? checkRemoveInLine(board[changedRow], board[changedRow][changedCol]) : checkAddInLine(board[changedRow], select);

    // check col
    std::vector<int> lineCol;
    lineCol.reserve(BOARD_ROWS);
    for (int i = 0; i < BOARD_ROWS; i++)
        lineCol.push_back(board[i][changedCol]);
    (select == -1) ? checkRemoveInLine(std::move(lineCol), board[changedRow][changedCol])
        : checkAddInLine(std::move(lineCol), select);

    // 대각선
    std::vector<int> lineDiag;
    lineDiag.reserve(BOARD_COLS);
    if (changedRow == changedCol)
    {
        for (int i = 0; i < BOARD_ROWS; i++)
            lineDiag.push_back(board[i][i]);

        (select == -1) ? checkRemoveInLine(std::move(lineDiag), board[changedRow][changedCol])
            : checkAddInLine(std::move(lineDiag), select);
    }
    else if (changedRow + changedCol == BOARD_ROWS - 1)
    {
        for (int i = 0; i < BOARD_ROWS; i++)
            lineDiag.push_back(board[i][BOARD_ROWS - i - 1]);

        (select == -1) ? checkRemoveInLine(std::move(lineDiag), board[changedRow][changedCol])
            : checkAddInLine(std::move(lineDiag), select);
    }

    // 2x2 체크
    check2x2(changedRow, changedCol, -1, -1, select);
    check2x2(changedRow, changedCol, 1, -1, select);
    check2x2(changedRow, changedCol, -1, 1, select);
    check2x2(changedRow, changedCol, 1, 1, select);
}

bool Board::check2x2(int changedRow, int changedCol, int dRow, int dCol, int select)
{
    if (0 <= changedRow + dRow && changedRow + dRow < BOARD_ROWS
        && 0 <= changedCol + dCol && changedCol + dCol < BOARD_COLS) {
        if (select != -1)
            checkAddInLine({ board[changedRow][changedCol],
                board[changedRow + dRow][changedCol],
                board[changedRow][changedCol + dCol],
                board[changedRow + dRow][changedCol + dCol] }, select);
        else
            checkRemoveInLine({ board[changedRow][changedCol],
                board[changedRow + dRow][changedCol],
                board[changedRow][changedCol + dCol],
                board[changedRow + dRow][changedCol + dCol] }, board[changedRow][changedCol]);
    }
    return false;
}

// called before board field change
void Board::checkAddInLine(std::vector<int> line, int pieceToAdd)
{
    // remove -1
    auto removeIter = std::remove_if(line.begin(), line.end(), [](int lineElement) {return lineElement == -1; });
    line.erase(removeIter, line.end());

    // 2 -> 3으로 추가되는 경우 terminator trait가 증가함
    if (line.size() == 2)
    {
        //int equalBits = 0b1111;
        //equalBits &= ~(line[0] ^ line[1]);
        //equalBits &= ~(line[1] ^ pieceToAdd);
        std::bitset<4> equalBits = getSamePieceTraits({ line[0], line[1], pieceToAdd });
        for (int bitPlace = 0; bitPlace < TRAIT_COUNT; bitPlace++)
        {
            if (equalBits[bitPlace] == true) {
                bool pickedBitValue = (line[0] & (0b1 << bitPlace)) > 0;
                terminatorPlaceCount[bitPlace][pickedBitValue]++;
            }
        }
    }
    // 3->4로 추가되는 경우 terminator trait가 감소할 수 있음
    else if (line.size() == 3)
    {
        //int equalBits = 0b1111;
        //for (int i = 1; i < line.size(); i++)
        //{
        //    equalBits &= ~(line[i - 1] ^ line[i]);
        //}
        std::bitset<4> equalBits = getSamePieceTraits(line);

        for (int bitPlace = 0; bitPlace < TRAIT_COUNT; bitPlace++)
        {
            if (equalBits[bitPlace]) {
                bool pickedBitValue = (line[0] & (0b1 << bitPlace)) > 0;
                terminatorPlaceCount[bitPlace][pickedBitValue]--;
            }
        }

        // terminated state인지 확인
        equalBits &= ~std::bitset<4>(line[0] ^ pieceToAdd);
        if (equalBits.any())
            m_isWinnerExist = true;
    }
}

// called before board field change
void Board::checkRemoveInLine(std::vector<int> line, int pieceToRemove)
{
    // remove -1
    auto removeIter = std::remove_if(line.begin(), line.end(), [](int lineElement) {return lineElement == -1; });
    line.erase(removeIter, line.end());

    // 3->2로 감소하는 경우 terminator trait가 감소함
    if (line.size() == 3)
    {
        std::bitset<4> equalBits = getSamePieceTraits(line);
        for (int bitPlace = 0; bitPlace < TRAIT_COUNT; bitPlace++)
        {
            if (equalBits[bitPlace]) {
                bool pickedBitValue = (line[0] & (0b1 << bitPlace)) > 0;
                terminatorPlaceCount[bitPlace][pickedBitValue]--;
            }
        }
    }
    // 4 -> 3으로 감소되는 경우 terminator trait가 증가함
    else if (line.size() == 4)
    {
        // line에 세 piece만 남았을 때의 terminator trait 계산
        line.erase(std::find(line.begin(), line.end(), pieceToRemove));

        std::bitset<4> equalBits = getSamePieceTraits(line);

        for (int bitPlace = 0; bitPlace < TRAIT_COUNT; bitPlace++)
        {
            if (equalBits[bitPlace]) {
                bool pickedBitValue = (line[0] & (0b1 << bitPlace)) > 0;
                terminatorPlaceCount[bitPlace][pickedBitValue]++;
            }
        }

        // terminated state를 false로 바꿀 지 확인
        equalBits &= ~std::bitset<4>(line[0] ^ pieceToRemove);
        if (equalBits.any())
            m_isWinnerExist = false;
    }
}

bool Board::hasTerminatorTrait(int piece) const
{
    for (int bitPlace = 0, bitPicker = 0b1; bitPlace < TRAIT_COUNT; bitPlace++, bitPicker *= 2)
    {
        bool pickedBitValue = (piece & bitPicker) > 0;
        if (terminatorPlaceCount[bitPlace][pickedBitValue] > 0)
            return true;
    }
    return false;
}

std::array<int, 2> Board::getTerminatingPlace(int terminatingPiece)
{
    for (int rowIndex = 0; rowIndex < BOARD_ROWS; rowIndex++)
    {
        for (int colIndex = 0; colIndex < BOARD_COLS; colIndex++)
        {
            if (board[rowIndex][colIndex] == -1)
            {
                set(rowIndex, colIndex, terminatingPiece);
                if (m_isWinnerExist)
                {
                    set(rowIndex, colIndex, -1);
                    return { rowIndex, colIndex };
                }
                set(rowIndex, colIndex, -1);
            }
        }
    }
    return { -1,-1 };
}


bool Board::isWinnerExist() const
{
    return m_isWinnerExist;
}

int Board::getFilledCount() const
{
    return filledCount;
}

