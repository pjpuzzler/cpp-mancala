#include "mancala.hpp"

#include <iostream>
#include <numeric>
#include <random>
#include <chrono>

MancalaBoard::MancalaBoard(const n_stones pits[NUM_PLAYERS][NUM_PITS], player turn) : turn(turn)
{
    for (player p : {PLAYER_1, PLAYER_2})
    {
        for (pit_i i = 0; i < NUM_PITS; ++i)
        {
            this->pits[p][i] = pits[p][i];
        }
    }
}

n_stones MancalaBoard::getPit(player p, pit_i i) const
{
    return pits[p][i];
}

player MancalaBoard::getTurn() const
{
    return turn;
}

bool MancalaBoard::operator==(const MancalaBoard &b) const
{
    if (turn != b.turn)
        return false;
    for (player p : {PLAYER_1, PLAYER_2})
    {
        for (pit_i i = 0; i < NUM_PITS; ++i)
        {
            if (pits[p][i] != b.pits[p][i])
                return false;
        }
    }
    return true;
}

pit_i MancalaBoard::getLeftToRightPit(player p, pit_i i)
{
    return LEFT_TO_RIGHT_PITS[p][i];
}

Mancala::Mancala(const n_stones pits[NUM_PLAYERS][NUM_PITS], player turn, const n_stones mancalas[NUM_PLAYERS])
    : MancalaBoard(pits, turn)
{
    for (player p : {PLAYER_1, PLAYER_2})
    {
        this->mancalas[p] = mancalas[p];
    }
}

void Mancala::displayGame() const
{
    n_stones PLAYER_1_MANCALA = mancalas[PLAYER_1],
             PLAYER_2_MANCALA = mancalas[PLAYER_2];
    for (player p : {PLAYER_2, PLAYER_1})
    {
        std::cout << PLAYER_2_MANCALA << " ";
        for (pit_i i = 0; i < NUM_PITS; ++i)
        {
            std::cout << pits[p][LEFT_TO_RIGHT_PITS[p][i]] << " ";
        }
        std::cout << PLAYER_1_MANCALA << std::endl;
    }
    std::cout << (turn == PLAYER_1 ? "P1" : "P2") << std::endl;
}

void Mancala::move(pit_i move)
{
    if (move == NULL_MOVE)
    {
        turn = !turn;
        return;
    }
    player side = turn;
    pit_i i = move;
    n_stones stones = pits[side][i];
    pits[side][i] = 0;
    for (; stones > 0; --stones)
    {
        if (i == NUM_PITS || (i == NUM_PITS - 1 && side == !turn))
        {
            i = 0;
            side = !side;
        }
        else
        {
            ++i;
        }

        if (i == NUM_PITS)
            ++mancalas[side];
        else
            ++pits[side][i];
    }

    if (i != NUM_PITS)
    {
        if (side == turn && pits[side][i] == 1)
        {
            player oppSide = !side;
            pit_i j = LEFT_TO_RIGHT_PITS[0][i];
            if (pits[oppSide][j] > 0)
            {
                mancalas[side] += pits[oppSide][j] + 1;
                pits[side][i] = 0;
                pits[oppSide][j] = 0;
            }
        }
        turn = !turn;
    }
}

bool Mancala::isValidMove(pit_i move) const
{
    return pits[turn][move] > 0;
}

game_state Mancala::getGameState() const
{
    if (mancalas[PLAYER_1] >= STONES_TO_WIN || mancalas[PLAYER_2] >= STONES_TO_WIN)
        return MAJORITY_STONES;
    for (player side : {PLAYER_1, PLAYER_2})
    {
        if (std::accumulate(pits[side], pits[side] + NUM_PITS, 0) == 0)
            return side == PLAYER_1 ? EMPTY_SIDE_1 : EMPTY_SIDE_2;
    }
    return IN_PROGRESS;
}

void Mancala::cleanUp(player side)
{
    for (n_stones &n : pits[side])
    {
        if (n > 0)
        {
            mancalas[side] += n;
            n = 0;
        }
    }
}

outcome Mancala::getOutcome() const
{
    n_stones player1Score = mancalas[PLAYER_1],
             player2Score = mancalas[PLAYER_2];
    if (player1Score == player2Score)
        return DRAW;
    return player1Score > player2Score ? PLAYER_1_WIN : PLAYER_2_WIN;
}

n_stones Mancala::getMancala(player p) const
{
    return mancalas[p];
}

void init_zobrist()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<zobrist_hash> dis;
    zobristPlayer2Turn = dis(gen);
    for (int i = 0; i < NUM_PLAYERS; ++i)
    {
        for (int j = 0; j < NUM_PITS; ++j)
        {
            for (int k = 0; k < ZOBRIST_PITS; ++k)
            {
                zobristPits[i][j][k] = dis(gen);
            }
        }
    }
}

zobrist_hash compute_hash(const MancalaBoard &b)
{
    zobrist_hash hash = 0;
    if (b.getTurn() == PLAYER_2)
        hash ^= zobristPlayer2Turn;
    for (player p : {PLAYER_1, PLAYER_2})
    {
        for (pit_i i = 0; i < NUM_PITS; ++i)
        {
            hash ^= zobristPits[p][i][b.getPit(p, i)];
        }
    }
    return hash;
}

AI::AI(int depth) : maxDepth(depth)
{
    init_zobrist();
}

pit_i AI::getBestMove(const Mancala &m)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    usePVS = true;
    eval_score bestScore;
    pit_i bestMove;
    nodes = hits = prunes = researches = reductions = noPVMove = ttUpdates = 0;
    for (int depth = (maxDepth % 2 == 0 ? 2 : 1); depth <= maxDepth; depth += 2)
    {
        curDepth = searchDepth = depth;
        auto [curBestScore, curBestMove] = negamax(m, -std::numeric_limits<eval_score>::infinity(), std::numeric_limits<eval_score>::infinity(), 0, true);
        bestScore = curBestScore;
        bestMove = curBestMove;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "t:" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;
    std::cout << "eval:" << std::showpos << (m.getTurn() == PLAYER_1 ? bestScore : -bestScore) << std::noshowpos << std::endl;
    std::cout << "hits/nodes:" << hits << "/" << nodes << std::endl;
    std::cout << nodes / std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() << " nodes/s" << std::endl;
    std::cout << "prunes:" << prunes << std::endl;
    std::cout << "re-searches:" << researches << std::endl;
    std::cout << "reductions:" << reductions << std::endl;
    // std::pair<Mancala, tt_entry> pv[maxDepth + 1];
    // Mancala pvM = m;
    // for (int i = 0; i < maxDepth; ++i)
    // {
    //     tt_entry entry = tt.at(pvM);
    //     pv[i] = {pvM, entry};
    //     pvM.move(entry.bestMove);
    // }
    return bestMove;
}

std::pair<eval_score, pit_i> AI::negamax(const Mancala &currM, eval_score a, eval_score b, int depth, bool prevPV)
{
    if (curDepth == maxDepth)
        ++nodes;
    auto [eval, gS] = evaluate(currM);
    if (gS != IN_PROGRESS)
        return {eval, NULL_MOVE};

    int relDepth = curDepth - depth;
    eval_score a0 = a;
    bool hasEntry = false;
    tt_entry *entry;
    if (tt.find(currM) != tt.end())
    {
        hasEntry = true;
        entry = &tt[currM];
        if (entry->relDepth >= relDepth)
        {
            if (curDepth == maxDepth)
                ++hits;
            eval_score entryScore = eval + entry->relScore;
            if (entry->flag == EXACT)
                return {entryScore, entry->bestMove};
            else if (entry->flag == LOWER_BOUND)
                a = std::max(a, entryScore);
            else if (entry->flag == UPPER_BOUND)
                b = std::min(b, entryScore);

            if (a >= b)
                return {entryScore, entry->bestMove};
        }
    }

    if (depth >= searchDepth)
    {
        if (curDepth == maxDepth)
            ++depthCutoffs[searchDepth];
        return {eval, NULL_MOVE};
    }

    int startingDepth = searchDepth;
    eval_score bestScore;
    pit_i bestMove = NULL_MOVE;
    pit_i moves[NUM_PITS];
    int numMoves = 0;
    for (pit_i move = 0; move < NUM_PITS; ++move)
    {
        if (currM.isValidMove(move))
            moves[numMoves++] = move;
    }
    pit_i pvMove = hasEntry ? entry->bestMove : NULL_MOVE;
    if (curDepth == maxDepth && pvMove == NULL_MOVE)
        ++noPVMove;
    std::sort(moves, moves + numMoves, [this, &currM, pvMove](pit_i moveA, pit_i moveB)
              { return betterMove(currM, moveA, moveB, pvMove); });
    for (int i = 0; i < numMoves; ++i)
    {
        pit_i move = moves[i];
        bool isPV = move == pvMove;
        int reduction = getDepthReduction(prevPV && isPV, depth, i, numMoves);
        if (curDepth == maxDepth)
            reductions += reduction;
        searchDepth = startingDepth - reduction;
        player currTurn = currM.getTurn();
        Mancala nextM = currM;
        nextM.move(move);
        eval_score score;
        if (!usePVS || isPV)
        {
            if (nextM.getTurn() == currTurn)
                score = negamax(nextM, a, b, depth, true).first;
            else
                score = -negamax(nextM, -b, -a, depth + 1, true).first;
        }
        else
        {
            if (nextM.getTurn() == currTurn)
                score = negamax(nextM, a, a + 1, depth).first;
            else
                score = -negamax(nextM, -a - 1, -a, depth + 1).first;
            if (a < score && score < b)
            {
                if (curDepth == maxDepth)
                    ++researches;
                searchDepth = startingDepth;
                if (nextM.getTurn() == currTurn)
                    score = negamax(nextM, score, b, depth).first;
                // score = negamax(nextM, a, b, depth).first;
                else
                    score = -negamax(nextM, -b, -score, depth + 1).first;
                // score = -negamax(nextM, -b, -a, depth + 1).first;
            }
        }
        if (bestMove == NULL_MOVE || score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
        a = std::max(a, score);
        if (a >= b)
        {
            if (curDepth == maxDepth)
                ++prunes;
            break;
        }
    }

    if (!hasEntry || relDepth >= entry->relDepth)
    {
        if (!hasEntry)
        {
            tt[currM] = tt_entry{};
            entry = &tt[currM];
        }
        if (bestScore <= a0)
        {
            if (curDepth == maxDepth)
                ++nodeTypes[2];
            entry->flag = UPPER_BOUND;
        }
        else if (bestScore >= b)
        {
            if (curDepth == maxDepth)
                ++nodeTypes[1];
            entry->flag = LOWER_BOUND;
        }
        else
        {
            if (curDepth == maxDepth)
                ++nodeTypes[0];
            entry->flag = EXACT;
        }
        entry->relScore = bestScore - eval;
        entry->bestMove = bestMove;
        entry->relDepth = relDepth;
        ++ttUpdates;
    }

    return {bestScore, bestMove};
}

bool AI::betterMove(const Mancala &currM, pit_i moveA, pit_i moveB, pit_i pvMove) const
{
    if (moveA == pvMove)
        return true;
    if (moveB == pvMove)
        return false;

    player turn = currM.getTurn();
    n_stones stonesA = currM.getPit(turn, moveA),
             stonesB = currM.getPit(turn, moveB);
    pit_i endingPitA = moveA + stonesA,
          endingPitB = moveB + stonesB;
    n_stones stonesACapture = currM.getPit(!turn, MancalaBoard::getLeftToRightPit(PLAYER_2, endingPitA)), stonesBCapture = currM.getPit(!turn, MancalaBoard::getLeftToRightPit(PLAYER_2, endingPitB));

    if (endingPitA < NUM_PITS && currM.getPit(turn, endingPitA) == 0 && stonesACapture > 0)
    {
        if (endingPitB < NUM_PITS && currM.getPit(turn, endingPitB) == 0 && stonesBCapture > 0)
            return stonesACapture > stonesBCapture;
        return true;
    }
    if (endingPitB < NUM_PITS && currM.getPit(turn, endingPitB) == 0 && stonesBCapture > 0)
        return false;

    if (endingPitA % ONE_LAP == NUM_PITS)
    {
        if (endingPitB % ONE_LAP == NUM_PITS)
            return moveA > moveB;
        return true;
    }
    if (endingPitB % ONE_LAP == NUM_PITS)
        return false;

    return moveA > moveB;
}

std::pair<eval_score, game_state> AI::evaluate(const Mancala &currM)
{
    game_state gS = currM.getGameState();
    player turn = currM.getTurn();
    if (gS != IN_PROGRESS)
    {
        outcome o;
        if (gS == EMPTY_SIDE_1)
        {
            n_stones p1Stones = currM.getMancala(PLAYER_1),
                     p2Stones = currM.getMancala(PLAYER_2);
            for (pit_i i = 0; i < NUM_PITS; ++i)
            {
                p2Stones += currM.getPit(PLAYER_2, i);
            }
            o = p1Stones == p2Stones ? DRAW : (p1Stones > p2Stones ? PLAYER_1_WIN : PLAYER_2_WIN);
        }
        else if (gS == EMPTY_SIDE_2)
        {
            n_stones p1Stones = currM.getMancala(PLAYER_1),
                     p2Stones = currM.getMancala(PLAYER_2);
            for (pit_i i = 0; i < NUM_PITS; ++i)
            {
                p1Stones += currM.getPit(PLAYER_1, i);
            }
            o = p1Stones == p2Stones ? DRAW : (p1Stones > p2Stones ? PLAYER_1_WIN : PLAYER_2_WIN);
        }
        else
            o = currM.getOutcome();
        if (o == DRAW)
            return {0, gS};
        return {o == (turn == PLAYER_1 ? PLAYER_1_WIN : PLAYER_2_WIN) ? WIN_SCORE : -WIN_SCORE, gS};
    }
    return {currM.getMancala(turn) - currM.getMancala(!turn), IN_PROGRESS};
}

int AI::getDepthReduction(bool isPV, int depth, int i, int numMoves)
{
    if (isPV || i <= 2 || depth < 6)
        return 0;
    return 1;
}

int main()
{
    player AI_PLAYER = PLAYER_1;

    Mancala m;
    int depth;
    std::cout << "Depth:";
    std::cin >> depth;
    AI ai(depth);

    auto start = std::chrono::high_resolution_clock::now();
    game_state gS = m.getGameState();
    while (gS == IN_PROGRESS)
    {
        m.displayGame();
        if (m.getTurn() == AI_PLAYER || true)
        {
            pit_i move = ai.getBestMove(m);
            std::cout << "move:" << move << std::endl
                      << std::endl;
            m.move(move);
        }
        else
        {
            pit_i move;
            std::cout << "move:";
            std::cin >> move;
            std::cout << std::endl;
            m.move(move);
        }
        gS = m.getGameState();
    }
    if (gS == EMPTY_SIDE_1)
        m.cleanUp(PLAYER_2);
    else if (gS == EMPTY_SIDE_2)
        m.cleanUp(PLAYER_1);
    m.displayGame();
    outcome o = m.getOutcome();
    std::cout << (o == DRAW ? "DRAW " : o == PLAYER_1_WIN ? "P1 wins "
                                                          : "P2 wins ")
              << "(" << m.getMancala(PLAYER_1) << "-" << m.getMancala(PLAYER_2) << ")" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}