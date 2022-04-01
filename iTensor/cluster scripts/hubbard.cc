#include "itensor/all.h"
#include "myclass.h"
#include <cmath>
#include <iostream>
using namespace itensor;
using namespace std;

//------------------------------------------------------------------
int main(int argc, char **argv)
{
    const int Nx =  40; //atoi(argv[1]);
    const int Ny =  1; //atoi(argv[2]);
    const int Nup = atoi(argv[1]);
    const int Ndown = atoi(argv[2]);
    const bool yperiodic = false;
    const int N = Nx*Ny; //Number of sites
    //Local Hilbert space, with fermion number Nf and total S^z conservation
    auto sites = Electron(Nx*Ny, {"ConserveNf", true, "ConserveSz", true});
    auto lattice = squareLattice(Nx, Ny,{"YPeriodic=",yperiodic});

    cout << Nup << " " << Ndown << endl;
    //Hubbard Hamiltonian
    auto ampo = AutoMPO(sites);
    const double t = 1.0, U = -7.0; //Hubbard model parameters
    for (auto b : lattice){
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

    InitState initState(sites, "0");
    for (int i = 1; i <= N; ++i)
    {
        if ( i <= Nmin)
            initState.set(i, "UpDn");
        else if (is_up == true & Nleft!=0 & i<= Nleft)
            initState.set(i, "Up");
        else if (is_up == false & Nleft!=0 & i<=Nleft)
            initState.set(i, "Dn");
    }

    //DMRG sweeps
    auto sweeps = Sweeps(25);
    sweeps.maxdim() = 400;
    // sweeps.cutoff() = 1E-12;
    // sweeps.noise() = 1e-13;

    cout << "initState=" << initState << endl;
    // auto [energy,psi] = dmrg(H,randomMPS(initState),sweeps,"Silent");
    auto [energy, psi] = dmrg(H, randomMPS(initState), sweeps, "Quiet");
    cout << "Final energy=" << energy << endl;
    exit(1);
}
