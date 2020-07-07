#include "cec/cec.h"
#include "cec/simplify.h"
#include "testOp.h"
#include "util/parser.h"

using namespace std;

// cd build && cmake -G"Unix Makefiles && make" ../
int main(int argc, char *argv[])
{
    // testOp::test();
    if (argc >= 4)
    {
        clock_t startTime, endTime;
        startTime = clock();
        /* parse Verilog files */
        parser verilog_parser;
        vector<node *> *PIs = nullptr;
        vector<node *> *POs = nullptr;
        verilog_parser.parse(argv[1], argv[2], PIs, POs);
        endTime = clock();
        cout << "The parsing time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;

        /*
        cout << ">>> before: " << endl;
        verilog_parser.printG(miter);
        */

        /* simplify the graph */
        simplify sim;
        sim.clean_wire_buf(PIs);
        // merge PIs and constants
        for (auto &con : *verilog_parser.get_constants())
        {
            if (con->outs)
            {
                PIs->insert(PIs->end(), con);
            }
        }
        sim.id_reassign(PIs);
        vector<vector<node *> *> *layers = sim.layer_assignment(PIs, POs);
        // sim.reduce_repeat_nodes(layers);
        // sim.id_reassign(layers->at(0));
        // PIs->insert(PIs->end(),verilog_parser.get_constants()->begin(),verilog_parser.get_constants()->end());
        /*
        cout << ">>> after: " << endl;
        verilog_parser.printG(miter);
        */
        endTime = clock();
        cout << "The preprocess time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;
        /* evaluate the graph */
        cec cec_(argv[3]);
        // cec_.evaluate_from_PIs_to_POs(PIs);
        cec_.evaluate_by_z3(layers, 1600000 - (endTime - startTime)/1000);
        endTime = clock();
        cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;
        /* free up space */
        for (auto &item : *layers)
            cleanVP(item);
        vector<vector<node *> *>().swap(*layers);
    }
    else
    {
        printf("Please input three parameters, like \"./xec <golden.v> <revised.v> <output>\".");
    }

    Z3_del_context(logic);
    return 0;
}