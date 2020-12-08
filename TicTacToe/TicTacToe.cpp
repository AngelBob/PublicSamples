// TicTacToe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <ctime>
#include <cassert>

using namespace std;

static const char EMPTY_SQUARE = ' ';
static const char FIRST_PLAYER_CHIT = 'X';
static const char SECOND_PLAYER_CHIT = 'O';

enum _CHECK_WINNER
{
    NO_WINNER = 0,
    FIRST_PLAYER_WINS,
    SECOND_PLAYER_WINS,
    CAT_GAME,
};

enum _SQAURES
{
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

// Here's an array of all possible winning square combinations
// Array is ordered to favor winning combindations that use the center square, and
// corners taken precidence over edges.
// Each entry is ordered similarly, with center square in index 0, then corners and edges
static const int winners[ 8 ][ 3 ] = { { CENTER_MID, UPPER_LEFT, LOWER_RIGHT },
                                       { CENTER_MID, LOWER_LEFT, UPPER_RIGHT },
                                       { CENTER_MID, CENTER_LEFT, CENTER_RIGHT },
                                       { CENTER_MID, UPPER_MID, LOWER_MID },
                                       { UPPER_LEFT, UPPER_RIGHT, UPPER_MID },
                                       { UPPER_LEFT, CENTER_LEFT, LOWER_LEFT },
                                       { UPPER_RIGHT, LOWER_RIGHT, CENTER_RIGHT },
                                       { LOWER_LEFT, LOWER_RIGHT, LOWER_MID },
                                     };
static const int winner_count = sizeof( winners ) / sizeof( winners[ 0 ] );

bool MainMenu( bool playedOnce )
{
    bool play = false;
    char startChoice = '\0';
    while( startChoice != 'p' && startChoice != 'P' &&
           startChoice != 'q' && startChoice != 'Q' )
    {
        cout << ( startChoice == '\0' ? "What would you like to do:" : "Please choose one of P or Q:" ) << endl;
        cout << "P)lay" << ( playedOnce ? " again or" : " or" ) << endl;
        cout << "Q)uit: ";

        cin >> startChoice;
        cin.clear();
        cin.ignore( 15, '\n' );

        // Check the choice
        if( startChoice == 'p' || startChoice == 'P' )
        {
            // User wants to play.
            play = true;
        }

        cout << endl;
    }

    return play;
}

void PrintBoard( char ( &gameBoard )[ MAX_SQUARE ] )
{
    // Print the column header
    cout << "   1   2   3\n";

    // Print the board
    for( int row = 0; row < 3; ++row )
    {
        // Print the row header
        cout << ( row + 1 ) << " ";

        // Print the board
        for( int col = 0; col < 3; ++col )
        {
            cout << " ";
            cout << gameBoard[ ( row * 3 ) + col ];
            if( col < 2 )
            {
                cout << " |";
            }
        }
        
        if( row < 2 )
        {
            cout << "\n  ---+---+---\n";
        }
    }
    cout << "\n" << endl;
}

bool GoodMove( char( &gameBoard )[ MAX_SQUARE ], int squareId )
{
    bool goodMove = (   ( UPPER_LEFT <= squareId )
                     && ( squareId < MAX_SQUARE )
                     && ( gameBoard[ squareId ] == EMPTY_SQUARE ) );
    return goodMove;
}

int ParseUserInput( stringstream &userInput )
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
    int square = -1;
    if( COMPLETE_PARSE == state )
    {
        square = ( row * 3 ) + col;
    }

    return square;
}

int GetPlayerMove( char playerChit )
{
    string input;

    bool haveGoodInput = false;
    int square = 0;
    do
    {
        // Prompt the player for a selection
        cout << "You're playing " << playerChit << "\n";

        if( 0 == square )
        {
            cout << "Please enter your choice in ";
        }
        else
        {
            cout << "Rember to use ";
        }
        cout << "(row, column) format (e.g., 2,3)\n";
        cout << "Which square would you like? ";

        // Get the user input
        getline( cin, input );

        stringstream userInput( input );

        // Sqaure will either be set to a valid index or left as -1 on input parse
        square = ParseUserInput( userInput );
        if( -1 == square )
        {
            cout << "\nI'm sorry, I didn't understand that.  Please try again.\n" << endl;
        }
    } while( -1 == square );

    return square;
}

int TakeWinningSquare( char( &gameBoard )[ MAX_SQUARE ], char aiChit )
{
    bool haveMyWinner = false;
    bool haveTheirWinner = false;
    int theirWinner = -1;
    int winner = -1;

    for( int idx = 0; idx < winner_count; ++idx )
    {
        const int a = winners[ idx ][ 0 ];
        const int b = winners[ idx ][ 1 ];
        const int c = winners[ idx ][ 2 ];

        // If all squares are empty, skip it
        if( gameBoard[ a ] == EMPTY_SQUARE && gameBoard[ b ] == EMPTY_SQUARE && gameBoard[ c ] == EMPTY_SQUARE )
        {
            continue;
        }

        // If any two of the squares on the game board at indices a, b, or c match
        // and the third is empty, then the third is a winning square.
        if( gameBoard[ a ] == gameBoard[ b ] && gameBoard[ c ] == EMPTY_SQUARE )
        {
            haveMyWinner = ( gameBoard[ a ] == aiChit );
            winner = c;
        }
        else if( gameBoard[ a ] == gameBoard[ c ] && gameBoard[ b ] == EMPTY_SQUARE )
        {
            haveMyWinner = ( gameBoard[ a ] == aiChit );
            winner = b;
        }
        else if( gameBoard[ b ] == gameBoard[ c ] && gameBoard[ a ] == EMPTY_SQUARE )
        {
            haveMyWinner = ( gameBoard[ b ] == aiChit );
            winner = a;
        }

        if( winner >= 0 )
        {
            haveTheirWinner = !haveMyWinner;
            if( haveMyWinner )
            {
                break;
            }
            else
            {
                theirWinner = winner;
            }
        }
    }

    if( haveTheirWinner && !haveMyWinner )
    {
        // It's a blocking move
        winner = theirWinner;
    }

    return winner;
}

int TakeCornerSquare( char( &gameBoard )[ MAX_SQUARE ] )
{
    int corner = -1;

    // Look at the corners and take one, if available, preferring opposite corners
    if( gameBoard[ UPPER_LEFT ] != EMPTY_SQUARE && gameBoard[ LOWER_RIGHT ] == EMPTY_SQUARE )
    {
        corner = LOWER_RIGHT;
    }
    else if( gameBoard[ UPPER_LEFT ] == EMPTY_SQUARE &&  gameBoard[ LOWER_RIGHT ] != EMPTY_SQUARE )
    {
        corner = UPPER_LEFT;
    }
    else if( gameBoard[ LOWER_LEFT ] != EMPTY_SQUARE && gameBoard[ UPPER_RIGHT ] == EMPTY_SQUARE )
    {
        corner = UPPER_RIGHT;
    }
    else if( gameBoard[ LOWER_LEFT ] == EMPTY_SQUARE && gameBoard[ UPPER_RIGHT ] != EMPTY_SQUARE )
    {
        corner = LOWER_LEFT;
    }
    else if( gameBoard[ UPPER_LEFT ] == EMPTY_SQUARE )
    {
        corner = UPPER_LEFT;
    }
    else if( gameBoard[ UPPER_RIGHT ] == EMPTY_SQUARE )
    {
        corner = UPPER_RIGHT;
    }
    else if( gameBoard[ LOWER_LEFT ] == EMPTY_SQUARE )
    {
        corner = LOWER_LEFT;
    }
    else if( gameBoard[ LOWER_RIGHT ] == EMPTY_SQUARE )
    {
        corner = LOWER_RIGHT;
    }

    return corner;
}

int TakeAdjacentSquare( char( &gameBoard )[ MAX_SQUARE ], int playerSquare )
{
    // This function is called at most 1 time after the player's first turn.
    // Because there can only ever be a single filled square (the player's
    // square) it's guaranteed that the adjacent square is open.  Randomly
    // choose one of the adjacent squares
    int adjacent = -1;

    bool whichAdjacent = static_cast< bool >( rand() & 0x1 );

    // If the player took a adjacent, choose an adjacent edge
    if( playerSquare == UPPER_LEFT )
    {
        // Look at the edges and take and open one
        if( whichAdjacent )
        {
            adjacent = CENTER_LEFT;
        }
        else
        {
            adjacent = UPPER_MID;
        }
    }
    else if( playerSquare == UPPER_RIGHT )
    {
        // Look at the edges and take and open one
        if( whichAdjacent )
        {
            adjacent = UPPER_MID;
        }
        else
        {
            adjacent = CENTER_RIGHT;
        }
    }
    else if( playerSquare == LOWER_RIGHT )
    {
        if( whichAdjacent )
        {
            adjacent = CENTER_RIGHT;
        }
        else
        {
            adjacent = LOWER_MID;
        }
    }
    else if( playerSquare == LOWER_LEFT )
    {
        if( whichAdjacent )
        {
            adjacent = LOWER_MID;
        }
        else
        {
            adjacent = CENTER_MID;
        }
    }

    return adjacent;
}

int TakeCenterSquare( char( &gameBoard )[ MAX_SQUARE ] )
{
    int square = -1;
    if( gameBoard[ CENTER_MID ] == EMPTY_SQUARE )
    {
        square = CENTER_MID;
    }

    return square;
}

int TakeAnySquare( char( &gameBoard )[ MAX_SQUARE ] )
{
    // Player is playing randomly or poorly, so just pick any remaining open square
    int square;
    for( square = UPPER_LEFT; square < MAX_SQUARE; ++square )
    {
        if( gameBoard[ square ] == EMPTY_SQUARE )
        {
            break;
        }
    }

    return square;
}

int IsFirstMove( char( &gameBoard )[ MAX_SQUARE ], int &playerSquare, bool &aiGoesFirst )
{
    // Examine the board, if the board is empty, AI goes first
    // Otherwise, if there's exactly one filled square, player
    // went first.
    int firstSquare = -1;
    int filledSquares = 0;
    for( int square = UPPER_LEFT; square < MAX_SQUARE; ++square )
    {
        if( gameBoard[ square ] != EMPTY_SQUARE )
        {
            // Let the AI know which square the player chose
            firstSquare = square;
        }

        filledSquares += ( gameBoard[ square ] != EMPTY_SQUARE ) ? 1 : 0;
        if( filledSquares > 1 )
        {
            // Not the first move, so clear the player square value
            firstSquare = -1;
            break;
        }
    }

    // Only set the aiGoesFirst value on the opening move.
    if( 0 == filledSquares )
    {
        aiGoesFirst = true;
    }
    else if( 1 == filledSquares )
    {
        aiGoesFirst = false;
        playerSquare = firstSquare;
    }

    return( filledSquares <= 1 );
}

int TakeCounteringSquare( char( &gameBoard )[ MAX_SQUARE ], int playerSquare )
{
    int square = -1;

    // On the first move, the appropriate response is to remove the square that
    // provides the player with multiple paths to victory; case are:
    // 1) player takes center - ai wants any corner,
    // 2) player takes corner - ai wants an adjacent edge,
    // 3) player takes edge   - ai wants the center square.
    if( playerSquare == CENTER_MID )
    {
        // Player takes the center, AI wants any corner
        square = TakeCornerSquare( gameBoard );
    }
    else if( playerSquare == UPPER_LEFT || playerSquare == UPPER_RIGHT || playerSquare == LOWER_RIGHT || playerSquare == LOWER_LEFT )
    {
        // Player took a corner, AI wants an adjacent edge
        square = TakeAdjacentSquare( gameBoard, playerSquare );
    }
    else
    {
        // Player took an edge, center square is the appropriate block
        square = CENTER_MID;
    }

    return square;
}

int GetAIMove( char (&gameBoard)[ MAX_SQUARE ], char aiChit )
{
    // The AI _always_ wants to win (or block) first.
    // After that, the order in which AI wants squares
    // is dependant on who went first.

    // If the AI goes first then it's move is to take a corner
    // If the AI goes second then it's move depends on what the
    // opponent did:
    // a) if the oppenent took a corner, AI picks an edge square
    //    adjacent to the opponent's move, else
    // b) if the oppenent took an edge, AI picks the center, else
    // c) the player took the center, then
    //   ai takes a corner.

    // After the second move, it's either:
    // a) block or
    // b) a cat game, so square choice is irrelevant

    // Step 1 - try to win (or block, it's the same logic):
    int square = -1;
    square = TakeWinningSquare( gameBoard, aiChit );

    // Step 2 - take a corner or an edge, depending on who goes first
    // First move response is the critical one, if it's not a response
    // to the first move, then just try tacking a corner (or use a
    // corner as the opening salvo.
    static int moveNumber = 0;
    static int playersFirstSquare = -1;
    bool aiGoesFirst = false;
    bool isFirstMove = IsFirstMove( gameBoard, playersFirstSquare, aiGoesFirst );
    if( isFirstMove )
    {
        // It's either the first or second move of the match
        moveNumber = aiGoesFirst ? 1 : 2;
    }

    if( square < UPPER_LEFT )
    {
        switch( moveNumber )
        {
        case 2:
            // Player moved first, make the correct countering move
            square = TakeCounteringSquare( gameBoard, playersFirstSquare );
            break;
        case 4:
            // AI's second move should be to take the center square
            // if it's available, otherwise take a corner
            square = TakeCenterSquare( gameBoard );
            break;
        default:
            break;
        }
    }

    if( square < UPPER_LEFT )
    {
        // By default, the AI wants a corner square
        square = TakeCornerSquare( gameBoard );
    }
    // If the player is not trying to win, we could end up here.
    if( square < UPPER_LEFT )
    {
        // On the way to a cat game, pick any open space
        square = TakeAnySquare( gameBoard );
        assert( ( UPPER_LEFT <= square ) && ( square < MAX_SQUARE ) );
    }

    // User gets a move, then it's AIs turn again
    moveNumber += 2;

    return square;
}

int IsGameOver( char( &gameBoard )[ MAX_SQUARE ] )
{
    // Look for winner
    int winner = NO_WINNER;

    // Just go through the array of winning combinations
    bool haveWinner = false;
    int idx;
    for( idx = 0; idx < winner_count; ++idx )
    {
        const int a = winners[ idx ][ 0 ];
        const int b = winners[ idx ][ 1 ];
        const int c = winners[ idx ][ 2 ];

        if(    gameBoard[ a ] != EMPTY_SQUARE
            && gameBoard[ a ] == gameBoard[ b ]
            && gameBoard[ a ] == gameBoard[ c ] )
        {
            haveWinner = true;
            break;
        }
    }

    if( haveWinner )
    {
        // There's a winner!  Figure out which player won.
        const int square = winners[ idx ][ 0 ];
        if( FIRST_PLAYER_CHIT == gameBoard[ square ] )
        {
            winner = FIRST_PLAYER_WINS;
        }
        else
        {
            winner = SECOND_PLAYER_WINS;
        }
    }
    else
    {
        // There's no winner, see if the board is full
        winner = CAT_GAME;
        for( int square = UPPER_LEFT; square < MAX_SQUARE; ++square )
        {
            if( gameBoard[ square ] == EMPTY_SQUARE )
            {
                winner = NO_WINNER;
                break;
            }
        }
    }

    return winner;
}

void DoMove( char( &gameBoard )[ MAX_SQUARE ], bool playerTurn, char playerChit, char aiChit )
{
    bool goodMove;
    do
    {
        int square;
        if( playerTurn )
        {
            square = GetPlayerMove( playerChit );
        }
        else
        {
            square = GetAIMove( gameBoard, aiChit );
        }

        goodMove = GoodMove( gameBoard, square );
        if( goodMove )
        {
            gameBoard[ square ] = playerTurn ? playerChit : aiChit;
            if( !playerTurn )
            {
                int row = ( square / 3 ) + 1;
                int col = ( square % 3 ) + 1;

                cout << "Computer's turn.\nComputer chooses: ";
                cout << "(" << row << "," << col << ")" << endl;
            }
        }
        else if( playerTurn )
        {
            cout << "That square is already taken, please pick a different square" << endl;
            PrintBoard( gameBoard );
        }
    } while( !goodMove );
}

void PrintWrapUp( char( &gameBoard )[ MAX_SQUARE ], int whoWon, char playerChit )
{
    // Print the final board layout
    PrintBoard( gameBoard );

    switch( whoWon )
    {
    case FIRST_PLAYER_WINS:
        if( playerChit == FIRST_PLAYER_CHIT )
        {
            cout << "Player wins!  Congratulations, well played.\n";
        }
        else
        {
            cout << "Computer wins, but I got to go first.\n";
        }
        cout << endl;
        break;

    case SECOND_PLAYER_WINS:
        if( playerChit == SECOND_PLAYER_CHIT )
        {
            cout << "Player wins!  You came from behind.\n";
        }
        else
        {
            cout << "Computer wins!  Phew, it's a lot harder to win going second.\n";
        }
        cout << endl;
        break;

    case CAT_GAME:
        cout << "Wow!  Great game, nobody was able to come out on top.\n";
        cout << endl;
        break;

    default:
        break;
    }
}

void GameLoop( bool playerFirst, char playerChit, char aiChit )
{
    // Reset the game board
    char gameBoard[ MAX_SQUARE ];
    for( int square = UPPER_LEFT; square < MAX_SQUARE; ++square )
    {
        gameBoard[ square ] = EMPTY_SQUARE;
    }

    bool playerTurn = playerFirst;
    bool oneTurn = false;
    int whoWon = NO_WINNER;

    do
    {
        // If the computer goes first, skip the drawing of the empty board
        // Once a move has been made, draw the board prior to each move.
        if( oneTurn || playerFirst )
        {
            PrintBoard( gameBoard );
        }
        oneTurn = true;

        DoMove( gameBoard, playerTurn, playerChit, aiChit );

        // Switch players
        playerTurn = !playerTurn;

        // Is the game over?
        whoWon = IsGameOver( gameBoard );
        if( NO_WINNER != whoWon )
        {
            PrintWrapUp( gameBoard, whoWon, playerChit );
        }
    } while( NO_WINNER == whoWon );
}

int main()
{
    cout << "Let's play Tic-Tac-Toe\n";
    cout << "The first to get three in a row horizontally, vertically, or diagonaly is the winner\n";
    cout << endl;

    // Play again loop
    bool playerFirst = false;
    bool playedOnce = false;
    do
    {
        // See if the user wants to play or not
        if( !MainMenu( playedOnce ) )
        {
            // User wants out.
            break;
        }

        // We have a player!
        if( !playedOnce )
        {
            // For the first time randomly choose who goes first.
            srand( static_cast< unsigned int >( time( nullptr ) ) );
            playerFirst = ( rand() & 0x1 ) == 0;
        }
        else
        {
            // After the first game, alternate who goes first.
            playerFirst = !playerFirst;
        }

        // Whoever goes first plays 'X', second plays 'O'
        char playerChit;
        char aiChit;
        if( playerFirst )
        {
            playerChit = FIRST_PLAYER_CHIT;
            aiChit = SECOND_PLAYER_CHIT;
        }
        else
        {
            playerChit = SECOND_PLAYER_CHIT;
            aiChit = FIRST_PLAYER_CHIT;
        }

        // Tell the player the order
        cout << ( playerFirst ? "You go" : "The computer goes" ) << " first\n";
        if( playerFirst )
        { 
            cout << "You play " << playerChit << " and the computer plays " << aiChit << "\n";
        }
        else
        {
            cout << "The computer plays " << aiChit << " and you play " << playerChit << "\n";
        }
        cout << endl;

        // Play a game of tic-tac-toe
        GameLoop( playerFirst, playerChit, aiChit );

        // Signal that the game has been played at least one time.
        playedOnce = true;
    } while( 1 );
}
