import numpy as np
import subprocess
from itertools import product

import time

CPP_PROGRAM_PATH = "./QuartoCppCode.out"
# CPP_PROGRAM_PATH = "./QuartoCppCode.out" #linux

class P2():
    def __init__(self, board, available_pieces):
        self.pieces = [(i, j, k, l) for i in range(2) for j in range(2) for k in range(2) for l in range(2)]  # All 16 pieces
        self.board = board # Include piece indices. 0:empty / 1~16:piece
        self.available_pieces = available_pieces # Currently available pieces in a tuple type (e.g. (1, 0, 1, 0))
    
    def select_piece(self):
        # Make your own algorithm here

        # if len(self.available_pieces) >= 13:
        #     return random.choice(self.available_pieces)

        result = subprocess.run(CPP_PROGRAM_PATH, text=True, stdout=subprocess.PIPE, input=self.makeInput()).stdout
        return self.pieces[int(result)]


    def place_piece(self, selected_piece):
        # selected_piece: The selected piece that you have to place on the board (e.g. (1, 0, 1, 0)).
        # if len(self.available_pieces) >= 13:
        #     available_locs = [(row, col) for row, col in product(range(4), range(4)) if self.board[row][col]==0]
        #     return random.choice(available_locs)

        result = subprocess.run(CPP_PROGRAM_PATH, text=True, stdout=subprocess.PIPE, input=self.makeInput(selected_piece)).stdout
        result = tuple(map(int, result.split(',')))
        return result

    def makeInput(self, selected_piece = None):
        boardMinus1 = np.vectorize(lambda x : x-1)(self.board)
        resultString = '\n'.join([' '.join(map(str, row)) for row in boardMinus1])
        resultString += '\n' + str(len(self.available_pieces))
        resultString += '\n' + ' '.join([str(self.pieces.index(availablePiece)) for availablePiece in self.available_pieces])
        if selected_piece==None:
            resultString += '\n0'
        else:
            resultString += '\n1 ' + str(self.pieces.index(selected_piece))
        return resultString