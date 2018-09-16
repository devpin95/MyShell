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

    auto n = coms.size();
    bool last_is_redirect = false;
    int fd[2] = {0, };
    int in = 0;
    int i;

    // loop through
    for ( i = 0; i < n - 1; ++i ) {
        if (  i + 1 < n ) {
            if ( coms[i+1].pd == '|' ) {
                // Handle piping between commands
                pipe( fd );

                do_fork( in, fd[1], coms[i] );

                close(fd[1]);
                in = fd[0];
            }
            else if ( coms[i+1].pd == '<' ) {
                // handle redirection from a file
                // This means the last command is a file name and we don't want to exec it
                last_is_redirect = true;
            }
            else if ( coms[i+1].pd == '>' ) {
                // handle redirection to a file
                // This means the last command is a file name and we don't want to exec it
                last_is_redirect = true;
//                int ffd = open( coms[i+1].name.c_str(), O_CREAT | O_TRUNC , 0600);

                waitpid( do_fork( in, STDOUT_FILENO, coms[i], coms[i+1].name, false, true ), NULL, 0 );

//                close(ffd);
//                in = fd[0];
            }
        }
    }

    if ( !last_is_redirect ) {
        // only do this if the last command is not a redirect
        waitpid(do_fork(in, STDOUT_FILENO, coms[i]), NULL, 0);
    }

//    pid = fork();
//    if ( pid < 0 ) {
//        fprintf( stderr, "Fork Failed");
//    }
//    if ( pid == 0 ) {
//
//        char** arr = new char*[coms[0].args.size() + 1];
//        for ( int j = 0; j < coms[0].args.size(); ++j) {
//            arr[j] = (char*)(coms[0].args[j].c_str());
//        }
//
//        arr[coms[0].args.size()] = nullptr;
//
//
//        execvp( coms[0].name.c_str(), arr );
//    }
//    else {
//        wait(NULL);
//    }
}

int myshell::do_fork(
        int infd,
        int outfd,
        const Command &com,
        std::string filename, // = ""
        bool redirect_in, // = false
        bool redirect_out // = false
        ) {

    pid_t pid;
    int redirect_fd;
    pid = fork();

    if ( pid == 0 ) {
        bool somename = true;
//        while (somename) {
//            somename = true;
//            struct timespec tt { 1, 0 };
//            nanosleep(&tt, nullptr);
//        }
        if ( infd != 0 ) {
            // redirect stdin to infd
            dup2( infd, STDIN_FILENO );
        }

        if ( outfd != 1 ) {
            // redirect stdout to outfd
            dup2( outfd, STDOUT_FILENO );
        }

        if ( redirect_out ) {
            // redirecting output to file
            int ret;

            redirect_fd = open( filename.c_str(), O_CREAT, 0600 );

            if ( redirect_fd == -1 ) {
                perror("error a");
            }

            fprintf(stderr, "%d\n", redirect_fd);

            ret = dup2( redirect_fd, STDOUT_FILENO );

            if ( ret == -1 ) {
                perror("error f");
            }

            fprintf(stderr, "%d\n", ret);
            close(redirect_fd);
        }

        char** arr = new char*[com.args.size() + 1];
        for ( int j = 0; j < com.args.size(); ++j) {
            arr[j] = (char*)(com.args[j].c_str());
        }
        arr[com.args.size()] = nullptr;

        return execvp( com.name.c_str(), arr );
    }

    return pid;
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