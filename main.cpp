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

        /* simplify the graph */
        simplify sim;
        sim.clean_wire_buf(PIs);
        // merge PIs and constants
        for (auto &con : verilog_parser.get_constants())
        {
            if (con->outs)
            {
                PIs->insert(PIs->end(), con);
            }
        }
        vector<vector<node *> > layers = sim.id_reassign_and_layered(PIs, POs);

        // sim.id_reassign(PIs);
        // vector<vector<node *> > layers = sim.layer_assignment(PIs, POs);

        // sim.reduce_repeat_nodes(layers); // no considering the positions of ports for DC and HUMX
        // sim.id_reassign(layers->front());

        endTime = clock();
        double pre_time = (endTime - startTime) / 1000;
        cout << "The preprocess time is: " << pre_time / 1000 << " S" << endl;
        /* evaluate the graph */
        cec cec_(argv[3]);
        // pre_time > 112000 ? 1700000 - pre_time : 15 * pre_time
        cec_.evaluate_by_z3(layers, 1700000);
        endTime = clock();
        cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;
    }
    else
    {
        printf("Please input three parameters, like \"./xec <golden.v> <revised.v> <output>\".");
    }
    return 0;
}