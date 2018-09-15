#ifndef MYSHELL_H
#define MYSHELL_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

struct Command {
    std::string name;
    std::vector<char> cargs;
    std::vector<std::string> sargs;
    char pd = '\0';

    void clear( void ) {
        name = "";
        cargs.clear();
        sargs.clear();
        pd = '\0';
    };
};

struct interpretor {
    static std::vector<Command> interpret( const std::vector<std::string> &command );
};

struct tokenizor {
    static std::vector<char> tokenizeArgs( const std::string &s );
    static std::vector<std::string> tokenize( const std::string &s );
};

class myshell {
public:
    myshell( void );
    void run( void );
private:
    std::string input;

    interpretor intr;
    tokenizor tknzr;
};

#endif //MYSHELL_H