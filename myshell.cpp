#include "myshell.h"

myshell::myshell( void ) {
    std::cout << "Starting myshell" << std::endl;
}

void myshell::run( void ) {
    std::vector<std::string> tkns;
    std::cout << "Welcome to myshell" << std::endl;
    do {
        std::cout << "ms>";
        getline(std::cin, input);
        tkns = tknzr.tokenize( input );
        interpretor::interpret( tkns );
    } while ( input != "exit" );
}

std::vector<char> tokenizor::tokenizeArgs( const std::string &s ) {
    // expects the first letter to be '-'
    // returns each argument as

    std::vector<char> tokens;

    for ( int i = 1; i < s.length(); ++i ) {
        tokens.push_back( s[i] );
    }

    return tokens;
}

std::vector<std::string> tokenizor::tokenize(const std::string &s) {
    std::string ts;                     // holds the token taken from the ss
    std::stringstream ss;               // stringstream used to split by spaces
    std::vector<std::string> tokens;    // vector to hold the tokens

    ss << s;    // insert the entered command into the stringstream

    do {
        if ( !ss.good() ) break;
        ss >> ts;
        tokens.push_back( ts );
    } while ( true );

    return tokens;
}

int interpretor::interpret( const std::vector<std::string> &command ) {
    // branches the program based on the command passed in
    // types of commands:
    //      single word commands (ls, pwd, etc.)
    //      single commands w/ arguments (ls -a)

    bool looking_for_first_comm = true;
    bool looking_for_arguments = true;
    bool has_pipe = false;
    bool has_directional = false;

    proc first_proc;

    for ( int i = 0; i < command.size(); ++i ) {
        if ( looking_for_first_comm ) {
            // looking for file names

            first_proc.name = command[i];
            looking_for_first_comm = false;

            std::cout << "Command: " << command[i] << std::endl;
        }
        else
        {
            // looking for arguments or redirects/pipes
            if ( command[i][0] == '-' )
            {
                // argument
                std::vector<char> rgs = tokenizor::tokenizeArgs( command[i] );
                first_proc.cargs.insert( first_proc.cargs.end(), rgs.begin(), rgs.end() );
            }
            else if ( command[i][0] == '|' )
            {
                std::cout << "pipey boi: " << command[i] << std::endl;
                looking_for_first_comm = true;
            }
            else if ( command[i][0] == '<' )
            {
                std::cout << "arrow boi left: " << command[i] << std::endl;
                looking_for_first_comm = true;
            }
            else if ( command[i][0] == '>' )
            {
                std::cout << "arrow boi right: " << command[i] << std::endl;
                looking_for_first_comm = true;
            }
            else
            {
                first_proc.sargs.push_back( command[i] );
            }
        }
    }

    std::cout << "c args: ";
    for ( int j = 0; j < first_proc.cargs.size(); ++j ) {
        std::cout << first_proc.cargs[j] << " ";
    }
    std::cout << std::endl;

    std::cout << "s args: ";
    for ( int j = 0; j < first_proc.sargs.size(); ++j ) {
        std::cout << first_proc.sargs[j] << " ";
    }
    std::cout << std::endl;
}