#include "myshell.h"

myshell::myshell( void ) {
    std::cout << "Starting myshell" << std::endl;
}

void myshell::run( void ) {
    std::cout << "Welcome to myshell" << std::endl;
    do {
        std::cout << "ms>";
        getline(std::cin, input);
        tokenizor
    } while ( input != "exit" );
}

std::vector<std::string> myshell::tokenizor::tokenize(const std::string s) {
    const char delim = ' ';     // split up the string by the delimiter
    std::vector<std::string> tokens;    // places the tokens in the return vector
    std::string substring;

    for ( int i = 0; i < s.length(); ++i ) {
        if ( isspace( s[i] ) ) {
            tokens.push_back( substring );
            substring.clear();
        } else {
            substring += s[i];
        }
    }
}