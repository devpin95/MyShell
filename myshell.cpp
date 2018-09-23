#include <cstring>
#include "myshell.h"

myshell::myshell( void ) {
    std::cout << "Starting myshell" << std::endl;
}

void myshell::run( void ) {

//    getcwd(cwd, PATH_MAX);

    std::vector<std::string> tokens;
    std::vector<Command> commands;
    std::cout << "Welcome to myshell" << std::endl;

    do {
        std::cout << std::endl << "ms>";
        getline(std::cin, input);

        if ( input == "exit" ) break;

        tokens = tokenizor::tokenize( input );
        commands = interpretor::interpret( tokens );
        execute( commands );
    } while ( true );
}

void myshell::execute( const std::vector<Command> &coms ) {

    if ( !validCommand( coms ) ) return;

    const int PIPE_READ_END = 0;
    const int PIPE_WRITE_END = 1;
    auto n = coms.size();
    bool piping = false;
    int fd[2] = {0, };
    int write_to = 1;
    int read_from = 0;
    int pwrite_to = 1;
    int pread_from = 0;
    int parent_write = 0;
    //int i;
    int pid = 0;
    Command temp_com;

    // loop through
    for ( int i = 0; i < n; ++i ) {
        temp_com = coms[i];
        bool redirect_out = false;
        bool redirect_in = false;
        std::string filename;

        if ( i + 1 < n ) {
            pipe(fd);
            piping = true;

            if (coms[i + 1].pd == '|')
            {
                // we pipe into the next command
                write_to = fd[PIPE_WRITE_END];
            }
            else if (coms[i + 1].pd == '<')
            {
                redirect_in = true;
                filename = coms[i + 1].name;

                // otherwise, the next command is a redirect
                if (i + 2 < n && coms[i + 2].pd == '|')
                {
                    // we need to pipe out after the redirect
                    write_to = fd[PIPE_WRITE_END];
                }
                else if ( i + 2 < n && coms[i + 2].pd == '>' )
                {
                    // redirecting out after redirecting in
                }
                else
                {
                    // otherwise, we are at the end
                    read_from = fd[PIPE_READ_END];
                    pread_from = fd[PIPE_READ_END];
                    parent_write = fd[PIPE_WRITE_END];
                    pwrite_to = fd[PIPE_WRITE_END];
                    write_to = STDOUT_FILENO;
                }

                i += 1;
            }
            else if (coms[i + 1].pd == '>')
            {
                redirect_out = true;
                filename = coms[i + 1].name;
                write_to = STDOUT_FILENO;
                i += 2;
            }
        }

        if ( i == n - 1 ) {
            write_to = STDOUT_FILENO;
        }

        pid = doFork(read_from, write_to, temp_com, filename, redirect_in, redirect_out, pread_from, pwrite_to);

        if ( pid == -1 ) {
            perror("Error forking");
        }

        if ( redirect_in ) {
            ssize_t size;
            size_t bufsize = 1000;
            char buf[bufsize];

            int redirect_fd = open( filename.c_str(), O_RDONLY );

            if ( redirect_fd == -1 ) {
                perror("error redirecting");
            }

            while ( true ) {
                size = read( redirect_fd, &buf, bufsize );

                if ( size == -1 ) {
                    perror("something went wrong reading in the parent 1");
                    return;
                }
                else if ( size == 0 ){
                    break;
                }

                size = write( parent_write, &buf, (size_t)size );
                if ( size == -1 ) {
                    perror("something went wrong writing in the parent 2");
                    return;
                }
                else if ( size == 0 ) {
                    break;
                }
            }

            close(redirect_fd);
        }

        if ( piping ) {
            close(fd[PIPE_WRITE_END]);
            read_from = fd[PIPE_READ_END];
        }

        if ( i == n - 1 ) {
            waitpid(pid, NULL, 0);
        }

//            if ( coms[i+1].pd == '|' ) {
//                // Handle piping between commands
//                pipe( fd );
//
//                doFork(in, fd[1], coms[i]);
//
//                close(fd[1]);
//                in = fd[0];
//            }
//            else if ( coms[i+1].pd == '<' ) {
//                // handle redirection from a file
//                // This means the last command is a file name and we don't want to exec it
//                ++i;
//                if ( i >= n - 1 ) {
//                    last_is_redirect = true;
//
//                    waitpid(doFork(in, STDOUT_FILENO, coms[i], coms[i + 1].name, true, false), NULL, 0 );
//                }
//            }
//            else if ( coms[i+1].pd == '>' ) {
//                // handle redirection to a file
//                // This means the last command is a file name and we don't want to exec it
//                last_is_redirect = true;
//
//                waitpid(doFork(in, STDOUT_FILENO, coms[i], coms[i + 1].name, false, true), NULL, 0 );
//
//                close(ffd);
//                in = fd[0];
//            }
//        }
    }

//    if ( !last_is_redirect ) {
//        // only do this if the last command is not a redirect
//        waitpid(doFork(in, STDOUT_FILENO, coms[i]), NULL, 0);
//    }
}

bool myshell::validCommand(const std::vector<Command> &coms) {
    for ( int i = 0; i < coms.size(); ++i ) {
        if ( coms[i].name == "cd" )
        {
            if ( coms.size() > 1 ) {
                fprintf(stderr, "Invalid command\n");
                //std::cout << "Invalid command" << std::endl;
                return false;
            }
        }
        else if ( i > 1 && coms[i].pd == '<' )
        {
            fprintf(stderr, "Invalid redirect\n");
            //std::cout << "Invalid redirect" << std::endl;
            return false;
        }
        else if ( i < coms.size() - 1 && coms[i].pd == '>' )
        {
            fprintf(stderr, "Invalid redirect\n");
            //std::cout << "Invalid redirect" << std::endl;
            return false;
        }
    }


    return true;
}

int myshell::doFork(
        int read_from,
        int write_to,
        const Command &com,
        std::string filename, // = ""
        bool redirect_in, // = false
        bool redirect_out, // = false
        int pread_from, // = 0,
        int pwrite_to // = 1
) {

    pid_t pid;
    int redirect_fd;
    pid = fork();

    if ( pid == 0 ) {
//        bool somename = true;
//        while (somename) {
//            somename = true;
//            struct timespec tt { 1, 0 };
//            nanosleep(&tt, nullptr);
//        }

        // -----------------------------------------------------------------------------------------

        if ( read_from != 0 ) {
            // redirect stdin to infd
            dup2( read_from, STDIN_FILENO );
            close(read_from);
        }

        if ( write_to != 1 ) {
            // redirect stdout to outfd
            dup2( write_to, STDOUT_FILENO );
            close(write_to);
        }

        if ( redirect_out ) {
            // redirecting output to file
            int ret;

            redirect_fd = open( filename.c_str(), O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );

            if ( redirect_fd == -1 ) {
                perror("error redirecting");
            }

            ret = dup2( redirect_fd, STDOUT_FILENO );

            if ( ret == -1 ) {
                perror("error dup");
            }

            close(redirect_fd);
        }

        // -----------------------------------------------------------------------------------------

        if ( redirect_in ) {
            close(pwrite_to);
            // redirecting file to process

//            int ret;
//            size_t bufsize = 1000;
//            char buf[bufsize];
//
//            redirect_fd = open( filename.c_str(), O_RDONLY, S_IRUSR | S_IWUSR );
//
//            if ( redirect_fd == -1 ) {
//                perror("error redirecting");
//            }
//
//            while ( true ) {
//                auto val = read( redirect_fd, &buf, bufsize );
//                if ( val == -1 ) {
//                    perror("something went wrong writing");
//                    break;
//                }
//                else if ( val == 0 ){
//                    break;
//                }
//                val = write( STDIN_FILENO, &buf, bufsize );
//                if ( val == -1 && errno == EAGAIN ) {
//                    fprintf(stderr,
//                            "something went wrong writing %s %d %d\n",
//                            std::strerror(errno), STDIN_FILENO, errno);
//                    break;
//                }
//                else if ( val == 0 ){
//                    break;
//                }
//            }
//
//            //close(STDIN_FILENO);
//            close(redirect_fd);

//            int ret;
//
//
//            if ( redirect_fd == -1 ) {
//                perror("error redirecting");
//            }
//
//            ret = dup2( redirect_fd, STDIN_FILENO );
//
//            if ( ret == -1 ) {
//                perror("error dup");
//            }
//
//            std::ifstream redirect_f;
//            redirect_f.open( filename.c_str() );
//
//
//            if ( !redirect_f.is_open() ) {
//                perror("Couldn't open file");
//            }
//
//            while ( !redirect_f.eof() ) {
//                std::string s;
//                redirect_f >> s;
//
//                if ( write( STDIN_FILENO, s.c_str(), s.length() ) == -1 ) {
//                    perror("Something happened while writing");
//                }
//            }
//
//            redirect_f.close();
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