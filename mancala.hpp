#ifndef MANCALA_H
#define MANCALA_H

#include <unordered_map>
#include <utility>

using n_stones = int;
using pit_i = int;
using player = bool;
using eval_score = float;
using zobrist_hash = unsigned int;

const int NUM_PLAYERS = 2,
          NUM_PITS = 6,
          ZOBRIST_PITS = 49;
const n_stones STARTING_STONES = 4,
               STARTING_PITS[NUM_PLAYERS][NUM_PITS] = {{STARTING_STONES, STARTING_STONES, STARTING_STONES, STARTING_STONES, STARTING_STONES, STARTING_STONES},
                                                       {STARTING_STONES, STARTING_STONES, STARTING_STONES, STARTING_STONES, STARTING_STONES, STARTING_STONES}},
               STARTING_MANCALAS[NUM_PLAYERS] = {0, 0},
               STONES_TO_WIN = 25;
const pit_i NULL_MOVE = -1, ONE_LAP = NUM_PITS * 2 + 1;
const player PLAYER_1 = true,
             PLAYER_2 = false,
             STARTING_PLAYER = PLAYER_1;
const eval_score WIN_SCORE = std::numeric_limits<eval_score>::infinity();

enum game_state
{
    IN_PROGRESS,
    MAJORITY_STONES,
    EMPTY_SIDE_1,
    EMPTY_SIDE_2
};

enum outcome
{
    PLAYER_1_WIN,
    PLAYER_2_WIN,
    DRAW
};

enum tt_flag
{
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
};

struct tt_entry
{
    int relDepth;
    tt_flag flag;
    eval_score relScore;
    pit_i bestMove;
};

class MancalaBoard
{
protected:
    n_stones pits[NUM_PLAYERS][NUM_PITS];
    player turn;

    static constexpr pit_i LEFT_TO_RIGHT_PITS[NUM_PLAYERS][NUM_PITS] = {{5, 4, 3, 2, 1, 0},
                                                                        {0, 1, 2, 3, 4, 5}};

public:
    MancalaBoard(const n_stones pits[NUM_PLAYERS][NUM_PITS], player turn);

    n_stones getPit(player p, pit_i i) const;
    player getTurn() const;

    bool operator==(const MancalaBoard &b) const;

    static pit_i getLeftToRightPit(player p, pit_i i);
};

class Mancala : public MancalaBoard
{
private:
    n_stones mancalas[NUM_PLAYERS];

public:
    Mancala(const n_stones pits[NUM_PLAYERS][NUM_PITS] = STARTING_PITS, player turn = STARTING_PLAYER, const n_stones mancalas[NUM_PLAYERS] = STARTING_MANCALAS);

    void displayGame() const;
    void move(pit_i i);
    bool isValidMove(pit_i i) const;
    game_state getGameState() const;
    void cleanUp(player side);
    outcome getOutcome() const;
    n_stones getMancala(player p) const;
};

zobrist_hash zobristPlayer2Turn, zobristPits[NUM_PLAYERS][NUM_PITS][ZOBRIST_PITS];

void init_zobrist();
zobrist_hash compute_hash(const MancalaBoard &b);

template <>
struct std::hash<MancalaBoard>
{
    std::size_t operator()(const MancalaBoard &b) const
    {
        return compute_hash(b);
    }
};

class AI
{
private:
    int maxDepth, curDepth, searchDepth;
    bool usePVS;
    int nodes, hits, prunes, researches, reductions, noPVMove, ttUpdates;
    int depthCutoffs[100] = {}, nodeTypes[3] = {};
    std::unordered_map<MancalaBoard, tt_entry> tt;

    std::pair<eval_score, pit_i> negamax(const Mancala &currM, eval_score a, eval_score b, int depth, bool prevPV = false);
    bool betterMove(const Mancala &currM, pit_i i, pit_i j, pit_i pvMove) const;

    static std::pair<eval_score, game_state> evaluate(const Mancala &currM);
    static int getDepthReduction(bool isPV, int depth, int i, int numMoves);

public:
    AI(int depth);

    pit_i getBestMove(const Mancala &m);
};

#endif
