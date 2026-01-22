# machines_p2.py

class P2:
    """
    Human player driven by GUI clicks.
    - During select_piece phase: click an available piece below the board.
    - During place_piece phase: click a board cell.
    """

    # class-level buffers (because main loop instantiates P2 repeatedly)
    _picked_piece = None     # tuple like (0,1,0,1)
    _picked_pos = None       # (row, col)

    def __init__(self, board, available_pieces):
        self.board = board
        self.available_pieces = available_pieces

    # ---- called by main when mouse is clicked ----
    @classmethod
    def handle_mouse_click(cls, mouse_pos, flag, available_pieces, board, width, square_size, piece_size):
        x, y = mouse_pos

        if flag == "select_piece":
            # Available pieces are displayed in the area y >= width
            if y < width:
                return

            rel_y = y - width
            col = x // square_size
            row = rel_y // piece_size
            idx = row * 4 + col

            if 0 <= idx < len(available_pieces):
                cls._picked_piece = available_pieces[idx]

        elif flag == "place_piece":
            # Board area is y < width
            if y >= width:
                return

            col = x // square_size
            row = y // square_size

            if 0 <= row < board.shape[0] and 0 <= col < board.shape[1]:
                cls._picked_pos = (row, col)

    # ---- required interface ----
    def select_piece(self):
        """
        Return a piece tuple when user has clicked one; otherwise None.
        """
        piece = P2._picked_piece
        if piece is None:
            return None

        # consume buffer
        P2._picked_piece = None
        return piece

    def place_piece(self, selected_piece):
        """
        Return (row, col) when user has clicked a cell; otherwise None.
        selected_piece is unused here but kept to match interface.
        """
        pos = P2._picked_pos
        if pos is None:
            return None

        # consume buffer
        P2._picked_pos = None
        return pos

    # ---- helpers for resetting buffers ----
    @classmethod
    def reset_inputs(cls):
        cls._picked_piece = None
        cls._picked_pos = None

    @classmethod
    def reset_select_only(cls):
        cls._picked_piece = None

    @classmethod
    def reset_place_only(cls):
        cls._picked_pos = None
