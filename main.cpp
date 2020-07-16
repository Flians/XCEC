#include "cec/cec.h"
#include "cec/simplify.h"
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
        init_fout(argv[3]);
        /* parse Verilog files */
        parser miter;
        miter.parse(argv[1], argv[2]);
        endTime = clock();
        cout << "The parsing time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << " S" << endl;

        /* simplify the graph */
        simplify sim;
        sim.clean_wire_buf(&miter.PIs);

        vector<vector<Node *>> &layers = sim.id_reassign_and_layered(miter.PIs, miter.POs);
        // sim.reduce_repeat_nodes(layers); // no considering the positions of ports for DC and HUMX
        // sim.id_reassign(layers);


        // sim.id_reassign(miter.PIs);
        // vector<vector<Node *> > &layers = sim.layer_assignment(miter.PIs, miter.POs);
        // sim.reduce_repeat_nodes(layers); // no considering the positions of ports for DC and HUMX
        // sim.id_reassign(miter.PIs);

/*
        for (auto &item : layers)
        {
            sort(item.begin(), item.end(), [](const Node *A, const Node *B) {
                return A->name < B->name;
            });
            for (auto &node : item)
            {
                cout << node->name << " " << (node->ins ? node->ins->size() : 0) << " " << (node->outs ? node->outs->size() : 0) << endl;
            }
        }
*/
        endTime = clock();
        double pre_time = (endTime - startTime) / 1000;
        cout << "The preprocess time is: " << pre_time / 1000 << " S" << endl;
        /* evaluate the graph */
        cec cec_;
        // pre_time > 112000 ? 1700000 - pre_time : 15 * pre_time
        // cec_.evaluate_by_z3(layers, 1000000);
        cec_.evaluate_by_stp(layers, 1000);
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