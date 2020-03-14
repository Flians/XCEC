#include "util/parser.h"
#include "cec/simplify.h"
#include "cec/cec.h"

using namespace std;
// cd build && cmake -G"Unix Makefiles && make" ../
int main(int argc, char *argv[])
{
    if (argc >= 4)
    {
        /* parse Verilog files */
        parser verilog_parser;
        vector<node *> *PIs = NULL;
        vector<node *> *POs = NULL;
        verilog_parser.parse(argv[1], argv[2], PIs, POs);
        /*
        cout << ">>> before: " << endl;
        verilog_parser.printG(miter);
        */

        /* simplify the graph */
        simplify sim;
        sim.clean_wire_buf(PIs);
        /*
        cout << ">>> after: " << endl;
        verilog_parser.printG(miter);
        */

        /* evaluate the graph */
        cec cec_(argv[3]);
        cec_.evaluate(PIs);
    }
    else
    {
        printf("Please input three parameters, like \"./xec <golden.v> <revised.v> <output>\".");
    }
    return 0;
}