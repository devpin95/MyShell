#ifndef MYSHELL_H
#define MYSHELL_H

#include <iostream>
#include <string>
#include <vector>

class myshell {
public:
    myshell( void );
    void run( void );
private:
    std::string input;

    struct tokenizor {
        std::vector<std::string> tokenize( const std::string s );
    };
};

#endif //MYSHELL_H