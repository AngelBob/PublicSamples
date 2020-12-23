// TicTacToe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <array>
#include <ctime>
#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

static const char FIRST_PLAYER_CHIT = 'X';
static const char SECOND_PLAYER_CHIT = 'O';

class Board
{
public:
    enum _CHECK_WINNER
    {
        NO_WINNER = 0,
        FIRST_PLAYER_WINS,
        SECOND_PLAYER_WINS,
        CAT_GAME,
    };

    enum _SQAURES
    {
        INVALID_SQUARE = -1,
        UPPER_LEFT = 0,
        UPPER_MID,
        UPPER_RIGHT,
        CENTER_LEFT,
        CENTER_MID,
        CENTER_RIGHT,
        LOWER_LEFT,
        LOWER_MID,
        LOWER_RIGHT,
        MAX_SQUARE,
    };

    static const int EMPTY_SQUARE = ' ';

    Board()
    {
        Reset();
    }

    void Reset( void )
    {
        for( int square = UPPER_LEFT; square < MAX_SQUARE; ++square )
        {
            m_Board[ square ] = EMPTY_SQUARE;
        }

        m_FirstSquare = INVALID_SQUARE;
        m_SquaresTaken = 0;
    }

    int GetMoveNumber( void )
    {
        // Move number is one more than the current number occupied squares
        return m_SquaresTaken + 1;
    }

    int GetFirstMove( void )
    {
        return m_FirstSquare;
    }

    bool IsGoodMove( const int squareId )
    {
        bool okay =
            ( ( UPPER_LEFT <= squareId ) && ( squareId < MAX_SQUARE )
                && IsSquareEmpty( squareId )
            );

        return okay;
    }

    bool IsSquareEmpty( const int squareId )
    {
        return GetSquareValue( squareId ) == EMPTY_SQUARE;
    }

    int IsWinner( const int squareId )
    {
        // square must be in range and empty to start with
        assert( IsGoodMove( squareId ) );

        int isWinner = NO_WINNER;

        SetSquareValue( squareId, FIRST_PLAYER_CHIT );
        isWinner = IsGameOver();
        SetEmptySquare( squareId );

        if( NO_WINNER == isWinner )
        {
            SetSquareValue( squareId, SECOND_PLAYER_CHIT );
            isWinner = IsGameOver();
            SetEmptySquare( squareId );
        }

        return isWinner;
    }

    void SetEmptySquare( const int squareId )
    {
        m_Board[ squareId ] = EMPTY_SQUARE;
        if( m_SquaresTaken > 0 )
        {
            // When reseting a square, decrement the taken squares count
            --m_SquaresTaken;
        }
    }

    char GetSquareValue( const int squareId )
    {
        return m_Board[ squareId ];
    }

    void SetSquareValue( const int squareId, const char chit )
    {
        m_Board[ squareId ] = chit;

        if( m_SquaresTaken == 0 )
        {
            // Store the first square taken
            m_FirstSquare = squareId;
        }

        // When setting a square, increment the set squares count
        ++m_SquaresTaken;
    }

    int IsGameOver( void )
    {
        // Look for winner
        int whoWon = NO_WINNER;
        int winningSquare = INVALID_SQUARE;

        // Need to have at least 5 moves before there can be a winner
        if( m_SquaresTaken >= 5 )
        {
            // Just go through the array of winning combinations
            for( auto &winningCombo : winningCombos )
            {
                const int a = winningCombo[ 0 ];
                const int b = winningCombo[ 1 ];
                const int c = winningCombo[ 2 ];

                if( !IsSquareEmpty( a )
                    && GetSquareValue( a ) == GetSquareValue( b )
                    && GetSquareValue( a ) == GetSquareValue( c )
                    )
                {
                    winningSquare = a;
                    break;
                }
            }

            if( winningSquare != INVALID_SQUARE )
            {
                // There's a winner!  Figure out which player won.
                if( FIRST_PLAYER_CHIT == GetSquareValue( winningSquare ) )
                {
                    whoWon = FIRST_PLAYER_WINS;
                }
                else
                {
                    whoWon = SECOND_PLAYER_WINS;
                }
            }
            else
            {
                // There's no winner, see if the board is full
                whoWon = CAT_GAME;
                for( int square = UPPER_LEFT; square < MAX_SQUARE; ++square )
                {
                    if( IsSquareEmpty( square ) )
                    {
                        whoWon = NO_WINNER;
                        break;
                    }
                }
            }
        }

        return whoWon;
    }

    const std::array<std::array<int, 3>,8>& GetWinningCombinations( void )
    {
        return winningCombos;
    }

    void PrintBoard( std::ostream &out ) const
    {
        // Print the column header
        out << "   1   2   3\n";

        // Print the board
        for( long long int row = 0; row < 3; ++row )
        {
            // Print the row header
            out << ( row + 1 ) << " ";

            // Print the board
            for( long long int col = 0; col < 3; ++col )
            {
                out << " ";
                out << m_Board[ ( row * 3 ) + col ];
                if( col < 2 )
                {
                    out << " |";
                }
            }

            if( row < 2 )
            {
                out << "\n  ---+---+---\n";
            }
        }
        out << "\n" << std::endl;
    };

private:
    // Here's an array of all possible winning square combinations
    // Array is ordered to favor winning combindations that use the center square, and
    // corners taken precidence over edges.
    // Each entry is ordered similarly, with center square in index 0, then corners and edges
    const std::array< std::array<int, 3>, 8> winningCombos{ {
        { CENTER_MID,  UPPER_LEFT,  LOWER_RIGHT  },
        { CENTER_MID,  LOWER_LEFT,  UPPER_RIGHT  },
        { CENTER_MID,  CENTER_LEFT, CENTER_RIGHT },
        { CENTER_MID,  UPPER_MID,   LOWER_MID    },
        { UPPER_LEFT,  UPPER_RIGHT, UPPER_MID    },
        { UPPER_LEFT,  CENTER_LEFT, LOWER_LEFT   },
        { UPPER_RIGHT, LOWER_RIGHT, CENTER_RIGHT },
        { LOWER_LEFT,  LOWER_RIGHT, LOWER_MID    },
    } };

    std::array<char, 9> m_Board;

    int  m_FirstSquare;
    int  m_SquaresTaken;
};

std::ostream &operator<<( std::ostream &out, const Board& board )
{
    board.PrintBoard( out );

    return out;
};

class AI
{
public:
    enum _USER_LEVEL
    {
        LEVEL_BEGINNER = 1,
        LEVEL_INTERMEDIATE,
        LEVEL_EXPERT,
    };

    AI( const char chit )
        : m_Chit( chit )
    {
        Reset();
    }

    int TakeWinningSquare( std::unique_ptr<Board>& board, const float missWinPct, const float missBlockPct )
    {
        bool haveMyWinner = false;
        bool haveTheirWinner = false;
        int theirWinner = Board::INVALID_SQUARE;
        int winSquare = Board::INVALID_SQUARE;

        for( auto& winner : board->GetWinningCombinations() )
        {
            const int a = winner[ 0 ];
            const int b = winner[ 1 ];
            const int c = winner[ 2 ];

            // If any two squares are empty there's no winner
            if(    ( board->IsSquareEmpty( a ) && board->IsSquareEmpty( b ) )
                || ( board->IsSquareEmpty( a ) && board->IsSquareEmpty( c ) )
                || ( board->IsSquareEmpty( b ) && board->IsSquareEmpty( c ) )
              )
            {
                continue;
            }

            // If any two of the squares on the game board at indices a, b, or c match
            // and the third is empty, then the third is a winning square.
            if( board->GetSquareValue( a ) == board->GetSquareValue( b ) && board->IsSquareEmpty( c ) )
            {
                haveMyWinner = ( board->GetSquareValue( a ) == m_Chit );
                winSquare = c;
            }
            else if( board->GetSquareValue( a ) == board->GetSquareValue( c ) && board->IsSquareEmpty( b ) )
            {
                haveMyWinner = ( board->GetSquareValue( a ) == m_Chit );
                winSquare = b;
            }
            else if( board->GetSquareValue( b ) == board->GetSquareValue( c ) && board->IsSquareEmpty( a ) )
            {
                haveMyWinner = ( board->GetSquareValue( b ) == m_Chit );
                winSquare = a;
            }

            if( winSquare != Board::INVALID_SQUARE )
            {
                haveTheirWinner = !haveMyWinner;
                if( haveMyWinner )
                {
                    break;
                }
                else
                {
                    theirWinner = winSquare;
                }
            }
        }

        if( haveTheirWinner && !haveMyWinner )
        {
            // It's a blocking move
            winSquare = theirWinner;
        }

        if( winSquare != Board::INVALID_SQUARE )
        {
            // There's a winning or blocking square, does the AI take it or miss it?
            float percentChance = ( static_cast< float >( rand() ) / static_cast< float >( RAND_MAX ) ) * 100;
            float compChance;
            if( haveMyWinner )
            {
                // At beginner and intermediate levels, the AI has some chance of missing it's own
                // winning move.
                compChance = missWinPct;
            }
            else
            {
                // If it's not the AI winner, it must be a blocking move
                assert( haveTheirWinner );

                // At beginner and intermediate levels, the AI has some chance of missing it's own
                // winning move.
                compChance = missBlockPct;
            }

            if( percentChance < compChance )
            {
                // AI misses the win or block
                winSquare = Board::INVALID_SQUARE;
            }
        }

        return winSquare;
    }

    int TakeCornerSquare( std::unique_ptr<Board>& board )
    {
        int corner = Board::INVALID_SQUARE;

        // Look at the corners and take one, if available, preferring opposite corners
        if( !board->IsSquareEmpty( Board::UPPER_LEFT ) && board->IsSquareEmpty( Board::LOWER_RIGHT ) )
        {
            corner = Board::LOWER_RIGHT;
        }
        else if( board->IsSquareEmpty( Board::UPPER_LEFT ) && !board->IsSquareEmpty( Board::LOWER_RIGHT ) )
        {
            corner = Board::UPPER_LEFT;
        }
        else if( !board->IsSquareEmpty( Board::LOWER_LEFT ) && board->IsSquareEmpty( Board::UPPER_RIGHT ) )
        {
            corner = Board::UPPER_RIGHT;
        }
        else if( board->IsSquareEmpty( Board::LOWER_LEFT ) && !board->IsSquareEmpty( Board::UPPER_RIGHT ) )
        {
            corner = Board::LOWER_LEFT;
        }
        else if( board->IsSquareEmpty( Board::UPPER_LEFT ) )
        {
            corner = Board::UPPER_LEFT;
        }
        else if( board->IsSquareEmpty( Board::UPPER_RIGHT ) )
        {
            corner = Board::UPPER_RIGHT;
        }
        else if( board->IsSquareEmpty( Board::LOWER_LEFT ) )
        {
            corner = Board::LOWER_LEFT;
        }
        else if( board->IsSquareEmpty( Board::LOWER_RIGHT ) )
        {
            corner = Board::LOWER_RIGHT;
        }

        return corner;
    }

    int TakeAdjacentSquare( std::unique_ptr<Board>& board, int playerSquare )
    {
        // This function is called at most 1 time after the player's first turn.
        // Because there can only ever be a single filled square (the player's
        // square) it's guaranteed that the adjacent square is open.  Randomly
        // choose one of the adjacent squares
        int adjacent = Board::INVALID_SQUARE;

        bool whichAdjacent = static_cast< bool >( rand() & 0x1 );

        // If the player took a adjacent, choose an adjacent edge
        if( playerSquare == Board::UPPER_LEFT )
        {
            // Look at the edges and take and open one
            if( whichAdjacent )
            {
                adjacent = Board::CENTER_LEFT;
            }
            else
            {
                adjacent = Board::UPPER_MID;
            }
        }
        else if( playerSquare == Board::UPPER_RIGHT )
        {
            // Look at the edges and take and open one
            if( whichAdjacent )
            {
                adjacent = Board::UPPER_MID;
            }
            else
            {
                adjacent = Board::CENTER_RIGHT;
            }
        }
        else if( playerSquare == Board::LOWER_RIGHT )
        {
            if( whichAdjacent )
            {
                adjacent = Board::CENTER_RIGHT;
            }
            else
            {
                adjacent = Board::LOWER_MID;
            }
        }
        else if( playerSquare == Board::LOWER_LEFT )
        {
            if( whichAdjacent )
            {
                adjacent = Board::LOWER_MID;
            }
            else
            {
                adjacent = Board::CENTER_MID;
            }
        }

        return adjacent;
    }

    int TakeCenterSquare( std::unique_ptr<Board>& board )
    {
        int square = Board::INVALID_SQUARE;
        if( board->IsSquareEmpty( Board::CENTER_MID ) )
        {
            square = Board::CENTER_MID;
        }

        return square;
    }

    int TakeAnySquare( std::unique_ptr<Board>& board )
    {
        // Player is playing randomly or poorly, so just pick any remaining open square
        int square;
        for( square = Board::UPPER_LEFT; square < Board::MAX_SQUARE; ++square )
        {
            if( board->IsSquareEmpty( square ) )
            {
                break;
            }
        }

        return square;
    }
    int TakeCounteringSquare( std::unique_ptr<Board>& board, int playerSquare )
    {
        int square = Board::INVALID_SQUARE;

        // On the first move, the appropriate response is to remove the square that
        // provides the player with multiple paths to victory; case are:
        // 1) player takes center - ai wants any corner,
        // 2) player takes corner - ai wants an adjacent edge,
        // 3) player takes edge   - ai wants the center square.
        if( playerSquare == Board::CENTER_MID )
        {
            // Player takes the center, AI wants any corner
            square = TakeCornerSquare( board );
        }
        else if( playerSquare == Board::UPPER_LEFT || playerSquare == Board::UPPER_RIGHT || playerSquare == Board::LOWER_RIGHT || playerSquare == Board::LOWER_LEFT )
        {
            // Player took a corner, AI wants an adjacent edge
            square = TakeAdjacentSquare( board, playerSquare );
        }
        else
        {
            // Player took an edge, center square is the appropriate block
            square = Board::CENTER_MID;
        }

        return square;
    }

    int TakeRandomSquare( std::unique_ptr<Board>& board )
    {
        bool IsGoodMove;
        int isWinner;
        int square = Board::INVALID_SQUARE;
        do
        {
            isWinner = Board::NO_WINNER;

            square = m_RandomSquares.back();
            m_RandomSquares.pop_back();

            IsGoodMove = board->IsGoodMove( square );
            if( IsGoodMove )
            {
                // Space is open, check for winner for either player
                isWinner = board->IsWinner( square );
            }
        } while( !IsGoodMove || ( Board::NO_WINNER != isWinner ) );

        return square;
    }

    const char GetAIChit( void )
    {
        return m_Chit;
    }

    void Reset( void )
    {
        m_MoveNumber = 0;
        m_PlayersFirstSquare = Board::INVALID_SQUARE;

        // Create the list of random square options
        if( m_RandomSquares.size() == 0 )
        {
            m_RandomSquares.reserve( Board::MAX_SQUARE );
        }
        else
        {
            m_RandomSquares.clear();
        }

        std::vector<int> squares;
        for( int idx = 0; idx < Board::MAX_SQUARE; ++idx )
        {
            squares.emplace_back( idx );
        }

        do
        {
            std::vector<int>::iterator iter = squares.begin();

            int index = rand() % squares.size();
            m_RandomSquares.emplace_back( squares[ index ] );

            iter += index;
            squares.erase( iter );
        } while( 0 != squares.size() );
    }

    virtual int GetAIMove( std::unique_ptr<Board>& Board ) = 0;
    virtual int GetAILevel( void ) = 0;

protected:
    char m_Chit;
    int  m_MoveNumber;
    int  m_PlayersFirstSquare;
    std::vector<int> m_RandomSquares;
};

class AI_Beginner : public AI
{
    // Knobs for tuning the AI
    inline static const float MISS_WIN_PCT = 20.0f;
    inline static const float MISS_BLOCK_PCT = 50.0f;

public:
    AI_Beginner( const char chit )
        : AI( chit )
    {
    }

    int GetAIMove( std::unique_ptr<Board>& board )
    {
        // AI move percentages are tunable, so the percentages and values here
        // may not be reflective of the actual AI operation.

        // At beginner level:
        //     The AI will not see an opponent's winning move 50% of the time, and
        //     will not see it's own winning move 20% of the time.
        //     Other AI moves are random.

        // Step 1 - try to win (or block, it's the same logic):
        int square = Board::INVALID_SQUARE;
        square = TakeWinningSquare( board, MISS_WIN_PCT, MISS_BLOCK_PCT );

        // Step 2 - take a random square.
        if( Board::INVALID_SQUARE == square )
        {
            square = TakeRandomSquare( board );
        }

        // User gets a move, then it's AIs turn again
        m_MoveNumber += 2;

        return square;
    }

    int GetAILevel( void )
    {
        return LEVEL_BEGINNER;
    }
};

class AI_Intermediate : public AI
{
    // Knobs for tuning the AI
    inline static const float MISS_WIN_PCT = 5.0f;
    inline static const float MISS_BLOCK_PCT = 20.0f;

public:
    AI_Intermediate( const char chit )
        : AI( chit )
    {}

    int GetAIMove( std::unique_ptr<Board> &board )
    {
        // AI move percentages are tunable, so the percentages and values here
        // may not be reflective of the actual AI operation.

        // At the intermediate level:
        //     The AI will not see an opponent's winning move 20% of the time, and
        //     will not see it's own winning move 5% of the time.
        //     TODO:  figure out the rest of this level.  Currently it's just "beginner"
        //            random moves with a better chance of noticing the win/block.

        // Step 1 - try to win (or block, it's the same logic):
        int square = Board::INVALID_SQUARE;
        square = TakeWinningSquare( board, MISS_WIN_PCT, MISS_BLOCK_PCT );

        if( Board::INVALID_SQUARE == square )
        {
            square = TakeRandomSquare( board );
        }

        // User gets a move, then it's AIs turn again
        m_MoveNumber += 2;

        return square;
    }

    int GetAILevel( void )
    {
        return LEVEL_INTERMEDIATE;
    }
};

class AI_Expert : public AI
{
    // Knobs for tuning the AI
    inline static const float MISS_WIN_PCT = 0.0f;
    inline static const float MISS_BLOCK_PCT = 0.0f;

public:
    AI_Expert( const char chit )
        : AI( chit )
    {}

    int GetAIMove( std::unique_ptr<Board> &board )
    {
        // AI move percentages are tunable, so the percentages and values here
        // may not be reflective of the actual AI operation.

        // At expert level:
        //     The AI will always block an opponent's winning move, and will
        //     never miss it's own winning move.
        //
        //     Other AI moves depend on who went first.
        //     If the AI goes first then it's move is to take a corner.
        //     If the AI goes second then it's move depends on what the opponent did:
        //         a) if the oppenent took a corner, AI picks an edge square
        //            adjacent to the opponent's move, else
        //         b) if the oppenent took an edge, AI picks the center, else
        //         c) the player took the center, then
        //            ai takes a corner.

        // After the second move, it's either:
        // a) block or
        // b) a cat game, so square choice is irrelevant

        // Step 1 - try to win (or block, it's the same logic):
        int square = Board::INVALID_SQUARE;
        square = TakeWinningSquare( board, MISS_WIN_PCT, MISS_BLOCK_PCT );

        // Step 2 - take a corner or an edge, depending on who goes first
        // First move response is the critical one, if it's not a response
        // to the first move, then just try taking a corner (or use a
        // corner as the opening salvo.

        if( Board::INVALID_SQUARE == square )
        {
            int moveNumber = board->GetMoveNumber();
            if( 2 == moveNumber )
            {
                // Player moved first, make the correct countering move
                square = TakeCounteringSquare( board, board->GetFirstMove() );
            }
            else if( 4 == moveNumber )
            {
                // AI's second move should be to take the center square
                // if it's available, otherwise take a corner
                square = TakeCenterSquare( board );
            }
        }

        if( Board::INVALID_SQUARE == square )
        {
            // By default, the AI wants a corner square
            square = TakeCornerSquare( board );
        }

        // If the player is not trying to win, we could end up here.
        if( Board::INVALID_SQUARE == square )
        {
            // On the way to a cat game, pick any open space
            square = TakeAnySquare( board );
            assert( ( Board::UPPER_LEFT <= square ) && ( square < Board::MAX_SQUARE ) );
        }

        return square;
    }

    int GetAILevel( void )
    {
        return LEVEL_EXPERT;
    }
};

class Menu
{
public:
    static void PrintHeader( void )
    {
        std::cout << "Let's play Tic-Tac-Toe\n";
        std::cout << "The first to get three in a row horizontally, vertically, or diagonaly is the winner\n";
        std::cout << std::endl;
    }

    static bool DoMainMenu( bool playedOnce )
    {
        char startChoice = '\0';
        do
        {
            std::cout << ( startChoice == '\0' ? "What would you like to do:" : "Please choose one of P or Q:" ) << std::endl;
            std::cout << "P)lay" << ( playedOnce ? " again or" : " or" ) << std::endl;
            std::cout << "Q)uit: ";

            std::cin >> startChoice;
            std::cin.clear();
            std::cin.ignore( 15, '\n' );

            std::cout << std::endl;
        } while( startChoice != 'p' && startChoice != 'P' &&
            startChoice != 'q' && startChoice != 'Q' );

        // Check the choice
        bool play = ( startChoice == 'p' || startChoice == 'P' );

        return play;
    }

    static int DoUserLevelMenu( void )
    {
        char playerLevel = '\0';
        do
        {
            std::cout << ( playerLevel == '\0' ? "Choose your Tic-Tac-Toe knowledge level:\n" : "Please choose one of B, I, or E:\n" );
            std::cout << "B)eginner\n";
            std::cout << "I)ntermediate\n";
            std::cout << "E)xpert: ";

            std::cin >> playerLevel;
            std::cin.clear();
            std::cin.ignore( 15, '\n' );

            std::cout << std::endl;
        } while( playerLevel != 'b' && playerLevel != 'B' &&
            playerLevel != 'i' && playerLevel != 'I' &&
            playerLevel != 'e' && playerLevel != 'E' );

        // Have a valid player level choice
        int aiLevel = AI::LEVEL_BEGINNER;
        switch( playerLevel )
        {
        case 'b':
        case 'B':
            aiLevel = AI::LEVEL_BEGINNER;
            break;
        case 'i':
        case 'I':
            aiLevel = AI::LEVEL_INTERMEDIATE;
            break;
        case 'e':
        case 'E':
            aiLevel = AI::LEVEL_EXPERT;
            break;
        default:
            assert( !"Shouldn't ever get here... has there been another player level added? " );
            break;
        }

        return aiLevel;
    }

    static void PrintPlayOrder( bool playerFirst )
    {
        std::cout << ( playerFirst ? "You go" : "The computer goes" ) << " first\n";
        if( playerFirst )
        {
            std::cout << "You play " << FIRST_PLAYER_CHIT << " and the computer plays " << SECOND_PLAYER_CHIT << "\n";
        }
        else
        {
            std::cout << "The computer plays " << FIRST_PLAYER_CHIT << " and you play " << SECOND_PLAYER_CHIT << "\n";
        }
        std::cout << std::endl;
    }

    static void PrintWrapUp( std::unique_ptr<Board>& board, int whoWon, char aiChit )
    {
        switch( whoWon )
        {
        case Board::FIRST_PLAYER_WINS:
            if( aiChit == FIRST_PLAYER_CHIT )
            {
                std::cout << "Computer wins, but I got to go first.\n";
            }
            else
            {
                std::cout << "Player wins!  Congratulations, well played.\n";
            }
            std::cout << std::endl;
            break;

        case Board::SECOND_PLAYER_WINS:
            if( aiChit == SECOND_PLAYER_CHIT )
            {
                std::cout << "Computer wins!  Phew, it's a lot harder to win going second.\n";
            }
            else
            {
                std::cout << "Player wins!  You came from behind.\n";
            }
            std::cout << std::endl;
            break;

        case Board::CAT_GAME:
            std::cout << "Wow!  Great game, nobody was able to come out on top.\n";
            std::cout << std::endl;
            break;

        default:
            break;
        }
    }
};

int ParseUserInput( std::stringstream &userInput )
{
    // Parse it out and validate, start by assuming the input is bad...
    // Should have the format ( x , y ) or x , y; spaces and parenthesis are optional, the comma is required.
    enum _STATE_MACHINE
    {
        UNINITIALIZED = 0,
        HAVE_OPEN_PAREN,
        HAVE_ROW,
        HAVE_SEP,
        HAVE_COL,
        HAVE_CLOSE_PAREN,
        COMPLETE_PARSE,
        INVALID = 0xf
    };

    int row = 0;
    int col = 0;

    bool needParens = false;
    int state = UNINITIALIZED;
    while( state != INVALID && state != COMPLETE_PARSE )
    {
        char c = userInput.get();

        // Any white space is acceptible in any state, and should just be skipped, but end of line is finished
        if( ' ' == c || '\t' == c )
        {
            continue;
        }
        else if( '\n' == c )
        {
            break;
        }

        switch( state )
        {
        case UNINITIALIZED:
            // Beginning state, looking for the open paren.
            // Anything other than a '(' is bad in this state
            if( c == '(' )
            {
                needParens = true;
                state = HAVE_OPEN_PAREN;
            }
            else if( '1' <= c && c <= '3' )
            {
                row = ( c - '0' ) - 1;
                state = HAVE_ROW;
            }
            else
            {
                state = INVALID;
            }
            break;
        case HAVE_OPEN_PAREN:
            // Anything other than a number in the range [1,3] is bad
            if( '1' <= c && c <= '3' )
            {
                row = ( c - '0' ) - 1;
                state = HAVE_ROW;
            }
            else
            {
                state = INVALID;
            }
            break;
        case HAVE_ROW:
            // Anything other than a comma is bad
            if( ',' == c )
            {
                state = HAVE_SEP;
            }
            else
            {
                state = INVALID;
            }
            break;
        case HAVE_SEP:
            // Anything other than a number in the range [1,3] is bad
            if( '1' <= c && c <= '3' )
            {
                col = ( c - '0' ) - 1;
                state = HAVE_COL;
            }
            else
            {
                state = INVALID;
            }
            break;
        case HAVE_COL:
            // If there was no opening paren, then we're done, otherwise
            // anything other than a closing paren is bad
            if( false == needParens )
            {
                state = COMPLETE_PARSE;
            }
            else if( ')' == c )
            {
                state = HAVE_CLOSE_PAREN;
            }
            else
            {
                state = INVALID;
            }
            break;
        case HAVE_CLOSE_PAREN:
            state = COMPLETE_PARSE;
            break;
        }
    }

    // If the parse was valid, then set the square
    int square = Board::INVALID_SQUARE;
    if( COMPLETE_PARSE == state )
    {
        square = ( row * 3 ) + col;
    }

    return square;
}

int GetPlayerMove( char playerChit )
{
    std::string input;

    bool haveGoodInput = false;
    int square = 0;
    do
    {
        // Prompt the player for a selection
        std::cout << "You're playing " << playerChit << "\n";

        if( 0 == square )
        {
            std::cout << "Please enter your choice in ";
        }
        else
        {
            std::cout << "Rember to use ";
        }
        std::cout << "(row, column) format (e.g., 2,3)\n";
        std::cout << "Which square would you like? ";

        // Get the user input
        getline( std::cin, input );

        std::stringstream userInput( input );

        // Sqaure will either be set to a valid index or left as Board::INVALID_SQUARE on input parse
        square = ParseUserInput( userInput );
        if( Board::INVALID_SQUARE == square )
        {
            std::cout << "\nI'm sorry, I didn't understand that.  Please try again.\n" << std::endl;
        }
    } while( Board::INVALID_SQUARE == square );

    return square;
}

void DoMove( std::unique_ptr<Board>& board, std::unique_ptr<AI>& ai, bool playerTurn )
{
    bool IsGoodMove;
    do
    {
        const char aiChit = ai->GetAIChit();
        const char playerChit = aiChit == SECOND_PLAYER_CHIT ? FIRST_PLAYER_CHIT : SECOND_PLAYER_CHIT;

        int square;
        if( playerTurn )
        {
            square = GetPlayerMove( playerChit );
        }
        else
        {
            square = ai->GetAIMove( board );
        }

        IsGoodMove = board->IsGoodMove( square );
        if( IsGoodMove )
        {
            board->SetSquareValue( square, playerTurn ? playerChit : aiChit );
            if( !playerTurn )
            {
                int row = ( square / 3 ) + 1;
                int col = ( square % 3 ) + 1;

                std::cout << "Computer's turn.\nComputer chooses: ";
                std::cout << "(" << row << "," << col << ")" << std::endl;
            }
        }
        else if( playerTurn )
        {
            std::cout << "That square is already taken, please pick a different square" << std::endl;
            std::cout << ( *board );
        }
    } while( !IsGoodMove );
}

void GameLoop( std::unique_ptr<AI>& ai, bool playerFirst )
{
    // Reset the game board
    std::unique_ptr<Board> board = std::make_unique<Board>();

    bool playerTurn = playerFirst;
    bool oneTurn = false;
    int whoWon = Board::NO_WINNER;

    do
    {
        // If the computer goes first, skip the drawing of the empty board.
        // Once a move has been made, draw the board prior to each move.
        if( oneTurn || playerFirst )
        {
            std::cout << ( *board );
        }
        oneTurn = true;

        DoMove( board, ai, playerTurn );

        // Switch players
        playerTurn = !playerTurn;

        // Is the game over?
        whoWon = board->IsGameOver();
        if( Board::NO_WINNER != whoWon )
        {
            std::cout << ( *board );
            Menu::PrintWrapUp( board, whoWon, ai->GetAIChit() );
        }
    } while( Board::NO_WINNER == whoWon );
}

int main()
{
    // Init the randomizer
    srand( static_cast< unsigned int >( time( nullptr ) ) );

    Menu::PrintHeader();

    // Play again loop
    int  playerLevel = AI::LEVEL_BEGINNER;
    bool playerFirst = true;
    bool playedOnce = false;

    std::unique_ptr<AI> ai = nullptr;
    do
    {
        // See if the user wants to play or not
        if( !Menu::DoMainMenu( playedOnce ) )
        {
            // User wants out.
            break;
        }

        // We have a player!
        if( !playedOnce )
        {
            playerLevel = Menu::DoUserLevelMenu();
            if( AI::LEVEL_BEGINNER == playerLevel )
            {
                playerFirst = true;
            }
            else
            {
                playerFirst = ( rand() & 0x1 ) == 0;
            }
        }
        else if( AI::LEVEL_BEGINNER != playerLevel )
        {
            // After the first game, alternate who goes first
            // unless the player is a beginner.  Beginners always
            // get to go first.
            playerFirst = !playerFirst;
        }

        // Whoever goes first plays 'X', second plays 'O'
        char aiChit;
        if( playerFirst )
        {
            aiChit = SECOND_PLAYER_CHIT;
        }
        else
        {
            aiChit = FIRST_PLAYER_CHIT;
        }

        // Tell the player the order
        Menu::PrintPlayOrder( playerFirst );

        // Create the AI
        if( !playedOnce )
        {
            switch( playerLevel )
            {
            case AI::LEVEL_BEGINNER:
                ai = std::unique_ptr<AI>( new AI_Beginner( aiChit ) );
                break;

            case AI::LEVEL_INTERMEDIATE:
                ai = std::unique_ptr<AI>( new AI_Intermediate( aiChit ) );
                break;

            case AI::LEVEL_EXPERT:
                ai = std::unique_ptr<AI>( new AI_Expert( aiChit ) );
                break;
            }
        }

        // Play a game of tic-tac-toe
        GameLoop( ai, playerFirst );

        // Signal complete game
        playedOnce = true;

        // Reset the AI
        ai->Reset();
    } while( 1 );

    ai = nullptr;
}
