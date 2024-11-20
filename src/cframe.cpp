#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <climits>
#include <string>
#include <vector>
#include <unordered_map>

#include <unistd.h>
#include <mpi.h>

#include "cframe.h"

class Circuit {

private:
    // Defines the type of different gates present
    enum GateName { And, Or, Not, Xor, Nand, Nor, Buf, Xnor };

    typedef struct {
        enum Gate;
    } Gate;

    std::unordered_map<std::string, std::pair<std::string, std::vector<std::string>>> circuit;

public:

















};
