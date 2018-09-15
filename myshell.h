#ifndef MYSHELL_H
#define MYSHELL_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

struct interpretor {
    static int interpret( const std::vector<std::string> &command );
};

struct tokenizor {
    static std::vector<char> tokenizeArgs( const std::string &s );
    static std::vector<std::string> tokenize( const std::string &s );
};

struct proc {
    std::string name;
    std::vector<char> cargs;
    std::vector<std::string> sargs;
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