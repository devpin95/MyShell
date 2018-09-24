#include "myshell.h"

myshell::myshell( void ) {
    std::cout << "Starting myshell" << std::endl;
    getcwd(cd.cwd, PATH_MAX);

    for ( int i = 0; i < PATH_MAX; ++i ) {
        cd.pwd[i] = cd.cwd[i];
    }
}

void myshell::run( void ) {

    std::vector<std::string> tokens;
    std::vector<Command> commands;
    std::cout << "Welcome to myshell" << std::endl;

    do {
        std::cout << std::endl << "ms>";
        getline(std::cin, input);

        // check
        if ( std::cin.eof() ) {
            break;
        }

        if ( input == "exit" ) break;

        tokens = tokenizor::tokenize( input );
        commands = interpretor::interpret( tokens );
        execute( commands );
    } while ( true );

    std::cout << "Closing myshell" << std::endl;
    std::cout << "Bye" << std::endl;
}

void myshell::execute( const std::vector<Command> &coms ) {

    // check if the command is valid before continuing
    if ( !validCommand( coms ) ) return;

    // check if we are cding
    if ( coms[0].name == "cd" ) {
        cd.cdExec( coms[0] );
        return;
    }

    // it is a valid command and it is not a cd, so continue interpreting
    const int PIPE_READ_END = 0;        // the read end of the pipe fd
    const int PIPE_WRITE_END = 1;       // the write end of the pipe fd
    auto n = coms.size();               // the number of commands to perform
    bool piping = false;                // flag to indicate if we need to pipe
    int fd[2] = {0, };                  // array to hold the fds of pipe
    int write_to = 1;                   // the fd we need the child to write to
    int read_from = 0;                  // the fd we need the child to read from
    int pwrite_to = 1;                  // the fd of the pipe's write end
    int pread_from = 0;                 // the fd of the pipe's read end
    int parent_write = 0;               // where we want the parent to write to (for redirecting from a file)
    int pid = 0;                        // the pid of the child process, use in waitpid
    Command temp_com;                   // a temp command so that we can increment i inside the loop

    // loop through
    for ( int i = 0; i < n; ++i ) {
        temp_com = coms[i];             // keep a temp command so that we can inc i in the loop
        bool redirect_out = false;      // bool indicating we're redirecting to a file
        bool redirect_in = false;       // bool indicating we're redirecting from a file
        std::string infilename;         // the file we are redirecting from
        std::string outfilename;        // the file we are redirecting to
        std::string procfilename;       // te file that we need to send to the child process

        // first, check the next command after the current one
        if ( i + 1 < n ) {
            // if there is a second command, we need a pipe to continue

            // get the pipe file descriptors
            pipe(fd);
            piping = true;

            if (coms[i + 1].pd == '|')
            {
                // we pipe into the next command
                write_to = fd[PIPE_WRITE_END];
            }
            else if (coms[i + 1].pd == '<')
            {
                // we are redirecting the data from a file
                redirect_in = true;
                infilename = coms[i + 1].name;

                // otherwise, the next command is a redirect
                if (i + 2 < n && coms[i + 2].pd == '|')
                {
                    // we need to pipe out after the redirect
                    write_to = fd[PIPE_WRITE_END];
                }
                else if ( i + 2 < n && coms[i + 2].pd == '>' )
                {
                    // redirecting out after redirecting in
                    // otherwise, we are at the end
                    read_from = fd[PIPE_READ_END];      // we want to proc to read from here
                    pread_from = fd[PIPE_READ_END];     // save the read end of the pipe
                    parent_write = fd[PIPE_WRITE_END];  // we want the parent to write here
                    pwrite_to = fd[PIPE_WRITE_END];     // save the write end of the pipe
                    write_to = STDOUT_FILENO;           // we want the proc to write here

                    redirect_out = true;
                    outfilename = coms[i + 2].name;
                    procfilename = outfilename;
                    i += 1;
                }
                else
                {
                    // otherwise, we are at the end
                    read_from = fd[PIPE_READ_END];      // we want to proc to read from here
                    pread_from = fd[PIPE_READ_END];     // save the read end of the pipe
                    parent_write = fd[PIPE_WRITE_END];  // we want the parent to write here
                    pwrite_to = fd[PIPE_WRITE_END];     // save the write end of the pipe
                    write_to = STDOUT_FILENO;           // we want the proc to write here
                }

                i += 1;
            }
            else if (coms[i + 1].pd == '>')
            {
                // redirecting to a file
                redirect_out = true;
                procfilename = coms[i + 1].name;
                write_to = STDOUT_FILENO;
                i += 2;
            }
        }

        // if we're on the last command, and we aren't piping, we need to write to stdout
        if ( i == n - 1 ) {
            write_to = STDOUT_FILENO;
        }

        // fork the process
        pid = doFork(read_from, write_to, temp_com, procfilename, redirect_in, redirect_out, pread_from, pwrite_to);

        // check for error forking
        if ( pid == -1 ) {
            perror("Error forking");
            return;
        }

        // if we are redirecting from a file, we need to open it in the parent and pass it to the child
        if ( redirect_in ) {
            ssize_t size;
            size_t bufsize = 1000;
            char buf[bufsize];

            int redirect_fd = open( infilename.c_str(), O_RDONLY );

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
    }
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
        }

        char** arr = new char*[com.args.size() + 1];
        for ( int j = 0; j < com.args.size(); ++j) {
            arr[j] = (char*)(com.args[j].c_str());
        }
        arr[com.args.size()] = nullptr;

        execvp( com.name.c_str(), arr );
        perror("failed exec");
        exit(127);
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

    return command_list;
}

void CD::cdExec( const Command &com ) {

    if ( com.args.size() == 1 ) {
        char *temp = getenv("HOME");

        // check if the getenv call failed
        if ( temp == NULL ) {
            perror("Error executing command: ");
            return;
        }

        // change the cwd to "home"
        if ( chdir( temp ) == -1 ) {
            perror("Error executing command: ");
            return;
        }

    }
    // change the cwd to the previous cwd
    else if ( com.args[1] == "-" ) {
        if ( chdir( pwd ) == -1 ) {
            perror("Error executing command: ");
            return;
        }
    }
    // change the dir to the value passed in
    else if ( chdir( com.args[1].c_str() ) == -1 ) {
        perror("Error executing command: ");
        return;
    }

    for ( int i = 0; i < PATH_MAX; ++i ) {
        pwd[i] = cwd[i];
    }

    getcwd( cwd, PATH_MAX );
}