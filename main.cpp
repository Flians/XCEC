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
        sim.id_reassign_and_layered(miter.PIs, miter.POs);
        sim.merge_nodes_between_networks(); // no considering the positions of ports for DC and HUMX

        cec cec_;
        if (argc >= 5)
        {
            switch (smt_str[argv[4]])
            {
            case Z3:
                printf("The prover is %s\n", argv[4]);
                cec_.evaluate_by_z3(sim.get_layers(), 1000000, 10000);
                break;
            case BOOLECTOR:
                printf("The prover is %s\n", argv[4]);
                cec_.evaluate_by_boolector(sim.get_layers(), 1000, 10000);
                break;
            default:
                printf("The prover is stp\n");
                cec_.evaluate_by_stp(sim.get_layers(), 1000, 10000);
                break;
            }
        }
        else
        {
            printf("The prover is stp\n");
            cec_.evaluate_by_stp(sim.get_layers(), 1000, 10000);
        }
        close_fout();
        endTime = clock();
        cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;
    }
    else
    {
        printf("Please input three parameters, like \"./XCEC <golden.v> <revised.v> <output>\".");
    }
    return 0;
}