#include "myshell.h"

myshell::myshell( void ) {
    std::cout << "Starting myshell" << std::endl;
}

void myshell::run( void ) {
    std::vector<std::string> tokens;
    std::vector<Command> commands;
    std::cout << "Welcome to myshell" << std::endl;

    do {
        std::cout << "ms>";
        getline(std::cin, input);

        if ( input == "exit" ) break;

        tokens = tokenizor::tokenize( input );
        commands = interpretor::interpret( tokens );
        execute( commands );
    } while ( true );
}

void myshell::execute( const std::vector<Command> &coms ) {
    pid = fork();
    if ( pid < 0 ) {
        fprintf( stderr, "Fork Failed");
    }
    if ( pid == 0 ) {

        char** arr = new char*[coms[0].args.size() + 1];
        for ( int j = 0; j < coms[0].args.size(); ++j) {
            arr[j] = (char*)(coms[0].args[j].c_str());
        }

        arr[coms[0].args.size()] = nullptr;


        execvp( coms[0].name.c_str(), arr );
    }
    else {
        wait(NULL);
    }
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
            temp_command.args.push_back( command[i] );
            looking_for_first_comm = false;
        }
        else
        {
            // looking for arguments or redirects/pipes
            if ( command[i][0] == '|' )
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
                temp_command.args.push_back( command[i] );
            }
        }
    }

    //temp_command.args.push_back( nullptr );
    command_list.push_back( temp_command );

//    for ( int i = 0; i < command_list.size(); ++i ) {
//        std::cout << "Command: "<< command_list[i].name << " " << command_list[i].pd << std::endl;
//        std::cout << "args: " << command_list[i].args << std::endl;
//    }

    return command_list;
}