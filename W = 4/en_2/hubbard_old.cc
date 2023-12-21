#include "itensor/all.h"
#include "myclass.h"
#include <iomanip> // для std::setprecision()
#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
using namespace itensor;
using namespace std;

class MyDMRGObserver : public DMRGObserver
{
    const double precision;
    double previous_energy;
    int ns; //sweep number
public:
    MyDMRGObserver(const MPS &psi, double prec, double E0) : DMRGObserver(psi), precision(prec), previous_energy(E0)
    {
        ns = 0;
    }
    bool checkDone(Args const &args = Args::global())
    {
        ns++;
        const double energy = args.getReal("Energy", 0);
        cout << "    Energy change:" << energy - previous_energy << endl;

        if ((abs(energy - previous_energy) < precision))
        {
            cout << "\n    Energy has converged -> stop the DMRG.\n";
            return true;
        }
        else
        {
            previous_energy = energy;
            return false;
        }
    }
};
//------------------------------------------------------------------
auto populate_auxiliary(int Nx, int Ny, int Nup, int Ndown, string initial_state){

    string to_start; 
    string to_proceed;
    string to_set;
    auto Nmin = Nup <= Ndown ? Nup : Ndown;
    auto dominant = Nup >= Ndown ? "Up" : "Dn";

    int max_pos = 0;
    int num = 0;

    //Local Hilbert space, with fermion number Nf and total S^z conservation
    auto sites = Electron(Nx*Ny, {"ConserveNf", true, "ConserveSz", true});
    // Initialize all sites with initial_state
    auto state = InitState(sites, initial_state);

    // Getting spin-up or spin-down as to_start
    if (Nup >= Ndown){
        to_start = (Nup+Ndown < Nx*Ny) ? "Dn" : "Up";
    }
    else {
        to_start = (Nup+Ndown < Nx*Ny) ? "Up" : "Dn";
    }
    
    for (auto i: range1(Ny)){
        if (to_start == "Up"){
            to_start = "Dn";
            to_proceed = "Up";
        }
        else{
            to_start = "Up";
            to_proceed = "Dn";
        }

        num = 0;

        for (int j = i; j<= 2*Nmin; j+=Ny){
            if (num%2 == 1){
                state.set(j, to_start);
                to_set = to_start;
                num+=1;
                
            }
            else{
                // cout << to_proceed << endl;
                state.set(j, to_proceed);
                to_set = to_proceed;
                num+=1;
                // cout << initState;
            }
            max_pos = j > max_pos ? j : max_pos;

            if (to_set == "Up"){
                Nup-=1;
                cout << j << "_" << to_set << " ";
            }
            else {
                Ndown-=1;
                cout  << j << "_" << to_set << " ";
            }
        }
            

    }
    // cout << endl;
    // cout << max_pos << endl;
    // cout << initState << endl;
    if (max_pos < Nx*Ny)
        for (int i = max_pos+1; i <= max_pos+max(Nup,Ndown); i++){
            state.set(i, dominant);
            cout << i << "_" << dominant << " ";
        }
struct retVals {        // Declare a local structure 
    InitState val1;
    SiteSet val2;
  };
// auto returned_mps = psi0(state);
return retVals {state, sites};

}

//------------------------------------------------------------------
auto populate_ladder(int Nx, int Ny, int Nup, int Ndown){
    if (Nup+Ndown <= Nx*Ny){
        auto to_return = populate_auxiliary(Nx, Ny, Nup, Ndown, "Emp");
        return to_return;
    }
        
    else{
        auto to_return = populate_auxiliary(Nx, Ny, Nx*Ny - Ndown, Nx*Ny - Nup, "UpDn");
        return to_return;
    }
}
//------------------------------------------------------------------
int run_dmrg(int Nx, int Ny, int Nup, int Ndown, bool yperiodic, double t_parallel, double t_perp,double U, string energy_file_name)
{

    const int N = Nx*Ny; //Number of sites
    
    //Local Hilbert space, with fermion number Nf and total S^z conservation
    // auto sites = Electron(Nx*Ny, {"ConserveNf", true, "ConserveSz", true});
    auto [state, sites] = populate_ladder(Nx, Ny, Nup, Ndown);
    auto lattice = squareLattice(Nx, Ny,{"YPeriodic=",yperiodic});

    //Hubbard Hamiltonian
    auto ampo = AutoMPO(sites);
    double t = 0;
    for (auto b : lattice){
        if (b.s1 == b.s2 - 1){
            t = t_perp;
        }
        else{
            t = t_parallel;
        }
        ampo += -t, "Cdagup", b.s1, "Cup", b.s2;
        ampo += -t, "Cdagup", b.s2, "Cup", b.s1;
        ampo += -t, "Cdagdn", b.s1, "Cdn", b.s2;
        ampo += -t, "Cdagdn", b.s2, "Cdn", b.s1;
    }

    for (int j = 1; j <= Nx*Ny; ++j)
    { //Hubbard terms
        ampo += U, "Nupdn", j;
    }
    auto H = toMPO(ampo);
    auto Nmin = Nup <= Ndown ? Nup : Ndown;
    auto is_up = Nup >= Ndown;
    auto Nleft = max(Nup, Ndown) - Nmin;
    
    int prev = 0;

    MPS psi0(state);

    const double energy0 = inner(psi0, H, psi0); //<psi|H|psi>
    cout << "Initial energy=" << energy0;

    //DMRG sweeps
    auto sweeps = Sweeps(400);
    sweeps.maxdim() = 10, 50, 100, 200, 400, 800, 1200, 2000, 2500;
    sweeps.cutoff() = 1E-10;
    sweeps.noise() = 1e-9;

    // sweeps.cutoff() = 1E-15;

    // cout << "initState=" << initState << endl;
    // auto [energy,psi] = dmrg(H,randomMPS(initState),sweeps,"Silent");
    double EnergyConvergenceThreshold = 1e-6;
    MyDMRGObserver obs(psi0, EnergyConvergenceThreshold, energy0);

    std::setprecision(16);
    
    auto start = chrono::steady_clock::now();
    auto [energy, psi] = dmrg(H, psi0, sweeps, obs, "Quiet");
    auto end = chrono::steady_clock::now();

    ofstream myfile;

    unsigned int precision = 16;
    myfile.precision(precision);

    myfile.open (energy_file_name.c_str(), std::ios_base::app);
        myfile <<"L="<< Nx << " W=" << Ny << " Nup =" << Nup << " Ndown = " << Ndown << " E = " << energy << " time = " << chrono::duration_cast<chrono::milliseconds>(end - start).count() <<  endl;
    myfile.close();

    return 0;
}

// int main(int argc, char **argv){
int main(int argc, char *argv[]){
    std::cout << std::setprecision(16);
    int Nx =  atoi(argv[1]);
    int Ny =  atoi(argv[2]);
    int Nup = atoi(argv[3]);
    int Ndown = atoi(argv[4]);
    // bool yperiodic = atoi(argv[5]);
    // double t_parallel = atoi(argv[6]);
    // double t_perp = atoi(argv[7]);
    // double U = atoi(argv[8]);
    string energy_file_name = argv[5];

    run_dmrg(Nx,Ny,Nup,Ndown,false,1.0,1.0,-7.0,energy_file_name);

    return 0;
}

