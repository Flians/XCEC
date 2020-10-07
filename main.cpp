#include "cec/cec.h"
#include "cec/simplify.h"
#include "util/parser.h"

using namespace std;

enum SMT
{
    Z3,
    STP,
    BOOLECTOR
};

unordered_map<string, SMT> smt_str = {
    {"z3", Z3}, {"stp", STP}, {"boolector", BOOLECTOR}};

// cd build && cmake -G"Unix Makefiles && make" ../
int main(int argc, char *argv[])
{
    if (argc >= 4)
    {
        clock_t startTime, endTime;
        startTime = clock();
        init_fout(argv[3]);
        /* parse Verilog files */
        parser miter;
        miter.parse(argv[1], argv[2]);
        miter.clean_wires();
        miter.clean_buf();
        endTime = clock();
        cout << "The parsing time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;

        /* simplify the graph */
        simplify sim;
        startTime = clock();
        sim.id_reassign_and_layered(miter.PIs, miter.POs);
        sim.merge_nodes_between_networks(); // no considering the positions of ports for DC and HUMX
        endTime = clock();
        cout << "The parsing time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;

        int timeout = 100;
        int max_conflicts = -1;
        bool is_incremental = false;
        if (argc >= 6 && argv[5][0] == 'i') {
            cout << "The prover is incremental." << endl;
            is_incremental = true;
        }
        if (argc >= 8) {
            timeout = atoi(argv[6]);
            max_conflicts = atoi(argv[7]);
        }
        cec cec_;
        startTime = clock();
        if (argc >= 5)
        {
            switch (smt_str[argv[4]])
            {
            case Z3:
                cout << "The prover is " << argv[4] << endl;
                cec_.evaluate_by_z3(sim.get_layers(), timeout, max_conflicts, is_incremental);
                break;
            case BOOLECTOR:
                cout << "The prover is " << argv[4] << endl;
                cec_.evaluate_by_boolector(sim.get_layers(), timeout, max_conflicts, is_incremental);
                break;
            default:
                cout << "The prover is stp." << endl;
                cec_.evaluate_by_stp(sim.get_layers(), timeout, max_conflicts, is_incremental);
                break;
            }
        }
        else
        {
            cout << "The prover is stp." << endl;
            cec_.evaluate_by_stp(sim.get_layers(), 1000, -1, false);
        }
        close_fout();
        endTime = clock();
        cout << "The runtime is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;
    }
    else
    {
        cout << "Please input at least three parameters, like \"./XCEC <golden.v> <revised.v> <output> <i/u> <timeout(-1 unlimited) max_conflicts(-1 unlimited)>\"." << endl;
    }
    return 0;
}