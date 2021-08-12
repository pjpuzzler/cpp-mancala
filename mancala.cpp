#include <iostream>
#include <string>
#include <stdexcept>
#include <optional>
#include <vector>

namespace Mancala
{
    typedef bool Player;
    Player Player1 = true, Player2 = false;

    typedef int Pit;

    Pit oppositePit(Pit pit)
    {
        if (pit >= 6)
            return pit - 6;
        return pit + 6;
    }

    class Board
    {
    public:
        Player turn;
        int pits[12];
        int stores[2];

        Board() : turn(Player1), pits{4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, stores{} {}

        bool isLegal(Pit pit) const
        {
            return pit >= 0 && pit < 12 && (this->turn == Player1 && pit >= 6 || this->turn == Player2 && pit < 6) && this->pits[pit] > 0;
        }

        std::vector<Pit> getLegalMoves() const
        {
            std::vector<Pit> legalMoves;
            for (Pit pit = 0; pit < 12; pit++)
            {
                if (this->isLegal(pit))
                    legalMoves.push_back(pit);
            }
            return legalMoves;
        }

        void makeMove(Pit pit)
        {
            if (!this->isLegal(pit))
                throw std::invalid_argument("Invalid Pit");

            bool onStore = false;
            Pit i = pit;
            int stones = this->pits[pit];
            this->pits[pit] = 0;
            while (stones-- > 0)
            {
                if (this->turn == Player1 && i == 11)
                {
                    onStore = true;
                    i = 5;
                    this->stores[Player1]++;
                }
                else if (this->turn == Player2 && i == 0)
                {
                    onStore = true;
                    i = 6;
                    this->stores[Player2]++;
                }
                else
                {
                    if (!onStore)
                    {
                        if (i == 0)
                            i = 6;
                        else if (i == 11)
                            i = 5;
                        else
                            i += i < 6 ? -1 : 1;
                    }
                    this->pits[i]++;
                    onStore = false;
                }
            }

            if (!onStore)
                this->turn = !this->turn;

            if (this->pits[i] == 1 && (this->turn == Player1 && i >= 6 || this->turn == Player2 && i < 6))
            {
                Pit j = oppositePit(i);
                this->pits[i] += this->pits[j];
                this->pits[j] = 0;
            }
        }

        bool isGameOver() const
        {
            return this->_isSideEmpty(Player1) || this->_isSideEmpty(Player2);
        }

        std::optional<Player> getWinner()
        {
            if (!isGameOver())
                return std::nullopt;

            int player1Score = this->stores[Player1] + this->_stoneCount(Player1),
                player2Score = this->stores[Player2] + this->_stoneCount(Player2);

            if (player1Score > player2Score)
                return Player1;
            if (player1Score < player2Score)
                return Player2;
            return std::nullopt;
        }

    private:
        bool _stoneCount(Player turn) const
        {
            int count = 0;
            for (int i = turn == Player1 ? 6 : 0; i < (turn == Player1 ? 12 : 6); i++)
                count += this->pits[i];
            return count;
        }

        bool _isSideEmpty(Player turn) const
        {
            return this->_stoneCount(turn) == 0;
        }
    };

    std::ostream &operator<<(std::ostream &os, const Board &board)
    {
        int leftBuffer = 0;
        for (int i = 0; i < 12; i++)
        {
            if (i == 5 || i == 11)
                os << board.pits[5] << std::endl;
            else
            {
                os << board.pits[i] << " ";
                if (((i == 2 || i == 8) && board.stores[0] >= 10 || (i == 3 || i == 9) && board.stores[1] >= 10 || board.pits[oppositePit(i)] >= 10) && board.pits[i] < 10)
                {
                    if (i <= 1 || i > 5 && i <= 7)
                        leftBuffer++;
                    os << " ";
                }
            }
        }
        for (int i = 0; i < leftBuffer + 4; i++)
            os << " ";
        os << board.stores[0] << " ";
        if (board.stores[0] < 10 && board.pits[8] >= 10)
            os << " ";
        os << board.stores[1];
        return os;
    }
};

int main()
{
    Mancala::Board board;
    while (!board.isGameOver())
    {
        std::cout << board << std::endl;
        std::string move;
        while (true)
        {
            std::cin >> move;
            Mancala::Pit pit = std::stoi(move);
            if (!board.isLegal(pit))
                std::cout << "Invalid Move" << std::endl;
            else
            {
                board.makeMove(pit);
                break;
            }
        }
    }

    std::optional<Mancala::Player> winner = board.getWinner();
    if (winner == std::nullopt)
        std::cout << "Draw!";
    else if (*winner == Mancala::Player1)
        std::cout << "Player 1 Wins!";
    else
        std::cout << "Player 2 Wins!";
}