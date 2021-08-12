#include "../mancala/cpp-mancala.cpp"
#include <limits>

class AI
{
public:
    int max_depth;
    Mancala::Board board;

    AI(int max_depth) : max_depth(max_depth) {}

    Mancala::Pit getBestMove(Mancala::Board &board)
    {
        this->board = board;
    }

private:
    double _negamax(int depth, double alpha, double beta)
    {
        if (depth == this->_curr_max_depth || this->board.isGameOver())
        {
            return this->_evaluate();
        }

        double bestScore = -std::numeric_limits<double>::infinity();
        for (Mancala::Pit pit : this->_getOrderedMoves())
        {
            this->board.makeMove(pit);
            double score = -this->_negamax(depth + 1, -alpha, -beta);
            this->board.pop();
        }
    }
};