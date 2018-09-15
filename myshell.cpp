#include "myshell.h"

myshell::myshell( void ) {
    std::cout << "Starting myshell" << std::endl;
}

void myshell::run( void ) {
    std::vector<std::string> tokens;
    std::cout << "Welcome to myshell" << std::endl;
    do {
        std::cout << "ms>";
        getline(std::cin, input);
        tokens = tokenizor::tokenize( input );
        interpretor::interpret( tokens );
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

std::vector<Command> interpretor::interpret( const std::vector<std::string> &command ) {
    // branches the program based on the command passed in
    // types of commands:
    //      single word commands (ls, pwd, etc.)
    //      single commands w/ arguments (ls -a)

    bool looking_for_first_comm = true;

    std::vector<Command> command_list;
    Command temp_command;

    for ( int i = 0; i < command.size(); ++i ) {
        if ( looking_for_first_comm )
        {
            // looking for file names

            temp_command.name = command[i];
            looking_for_first_comm = false;
        }
        else
        {
            // looking for arguments or redirects/pipes
            if ( command[i][0] == '-' )
            {
                // argument
                std::vector<char> rgs = tokenizor::tokenizeArgs( command[i] );
                temp_command.cargs.insert( temp_command.cargs.end(), rgs.begin(), rgs.end() );
            }
            else if ( command[i][0] == '|' )
            {
                looking_for_first_comm = true;
                command_list.push_back(temp_command);
                temp_command.clear();
                temp_command.pd = '|';
            }
            else if ( command[i][0] == '<' )
            {
                looking_for_first_comm = true;
                command_list.push_back(temp_command);
                temp_command.clear();
                temp_command.pd = '<';
            }
            else if ( command[i][0] == '>' )
            {
                looking_for_first_comm = true;
                command_list.push_back(temp_command);
                temp_command.clear();
                temp_command.pd = '>';
            }
            else
            {
                temp_command.sargs.push_back( command[i] );
            }
        }
    }

    command_list.push_back( temp_command );

//    for ( int i = 0; i < command_list.size(); ++i ) {
//        std::cout << "Command: "<< command_list[i].name << " " << command_list[i].pd << std::endl;
//
//        std::cout << "c args: ";
//        for ( int j = 0; j < command_list[i].cargs.size(); ++j ) {
//            std::cout << command_list[i].cargs[j] << " ";
//        }
//        std::cout << std::endl;
//
//        std::cout << "s args: ";
//        for ( int j = 0; j < command_list[i].sargs.size(); ++j ) {
//            std::cout << command_list[i].sargs[j] << " ";
//        }
//        std::cout << std::endl;
//    }

    return command_list;
}