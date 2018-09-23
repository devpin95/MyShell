#ifndef MYSHELL_H
#define MYSHELL_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <wait.h>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <climits>

struct Command {
    std::string name;
    std::vector<std::string> args;
    char pd = '\0';

    void clear( void ) {
        name = "";
        args.clear();
        pd = '\0';
    };
};

struct interpretor {
    static std::vector<Command> interpret( const std::vector<std::string> &command );
};

struct tokenizor {
    static std::vector<std::string> tokenize( const std::string &s );
};

class myshell {
public:
    myshell( void );
    void run( void );
    void execute( const std::vector<Command> &coms );
    bool validCommand(const std::vector<Command> &coms);
    int doFork(
            int read_from,
            int write_to,
            const Command &com,
            std::string filename = "",
            bool redirect_in = false,
            bool redirect_out = false,
            int pread_from = 0,
            int pwrite_to = 1);
private:
    std::string input;
    char *cwd[PATH_MAX] = {0, };
};

#endif //MYSHELL_H