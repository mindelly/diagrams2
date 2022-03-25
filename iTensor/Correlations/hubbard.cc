#include "itensor/all.h"
#include "myclass.h"
#include <iomanip> // для std::setprecision()
#include <iostream>
#include <string>
#include <fstream>
using namespace itensor;
using namespace std;
//------------------------------------------------------------------
//2-point correlation.
// Derived from : http://itensor.org/docs.cgi?vers=cppv3&page=formulas/correlator_mps
// see also: http://itensor.org/docs.cgi?page=tutorials/correlations
//

complex<double> Correlation(MPS &psi, string opname1, int i, string opname2, int j, const SiteSet &sites)
{
    if (j <= i)
        cout << "Error in Correlation: i=" << i << " j=" << j << endl, exit(0);
    ITensor op_i, op_j;
    for (int o = 1; o <= 2; o++)
    {
        ITensor &op_s = (o == 1) ? op_i : op_j;
        string name_s = (o == 1) ? opname1 : opname2;
        int s = (o == 1) ? i : j;
        if (name_s == "P") // Pair annihilation operator
        {
            op_s = op(sites, "Cup", s);
            op_s *= prime(op(sites, "Cdn", s));
            op_s.mapPrime(2, 1);
        }
        else if (name_s == "Pdag") // Pair creation operator
        {
            op_s = op(sites, "Cdagdn", s);
            op_s *= prime(op(sites, "Cdagup", s));
            op_s.mapPrime(2, 1);
        }
        else //Other single-site operator defined in ITEnsor for the FermionSite local Hilbert space
        {
            op_s = op(sites, name_s, s);
        }
    }
    // Are the two operators "fermionic" (i.e. anticommmute) ?
    const bool IsFermionic1 = (opname1 == "Cdagup" || opname1 == "Cdagdn" || opname1 == "Cup" || opname1 == "Cdn");
    const bool IsFermionic2 = (opname2 == "Cdagup" || opname2 == "Cdagdn" || opname2 == "Cup" || opname2 == "Cdn");
    if (IsFermionic1 != IsFermionic2)
        cout << "Error: mixing a fermionic and a non-fermion operator." << endl, exit(1);

    psi.position(i); //'gauge' the MPS to site i
    //Create the <psi| ('bra') from |psi> ('ket')
    auto psidag = dag(psi);

    //Prime the link indices to make them distinct from
    //the original ket links
    psidag.prime("Link");

    //below we will assume j > i

    //index linking i-1 to i:
    auto li_1 = leftLinkIndex(psi, i);

    auto C = prime(psi(i), li_1) * op_i;
    C *= prime(psidag(i), "Site");

    if (IsFermionic1)
    {
        for (int k = i + 1; k < j; ++k)
        {
            C *= psi(k);
            C *= op(sites, "F", k); // Jordan-Wigner fermion 'string' operator F =(−1)^{n}
            C *= prime(psidag(k), "Site");
        }
    }
    else
    {
        for (int k = i + 1; k < j; ++k)
        {
            C *= psi(k);
            C *= psidag(k);
        }
    }
    //index linking j to j+1:
    auto lj = rightLinkIndex(psi, j);

    C *= prime(psi(j), lj) * op_j;
    C *= prime(psidag(j), "Site");

    auto result = eltC(C);

    return result;
}
//------------------------------------------------------------------
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

//------------------------------------------------------------------
int run_dmrg(int Nx, int Ny, int Nup, int Ndown, bool yperiodic, double t_parallel, double t_perp,double U, string energy_file_name)
{

    const int N = Nx*Ny; //Number of sites
    //Local Hilbert space, with fermion number Nf and total S^z conservation
    auto sites = Electron(Nx*Ny, {"ConserveNf", true, "ConserveSz", true});
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

    InitState initState(sites, "0");
    for (int i = 1; i <= N; ++i)
    {
        if ( prev == 0 && i<= 2*Nmin){
            initState.set(i, "Up");
            prev = 1;
            }
        else if ( prev == 1 && i<= 2*Nmin){
            initState.set(i, "Dn");
            prev = 0;
            }
        else if ( prev == 0 && i<= 2*Nmin+Nleft){
            initState.set(i, "Up");
            prev = 0;
            }
    }

    MPS psi0(initState);
    const double energy0 = inner(psi0, H, psi0); //<psi|H|psi>
    cout << "Initial energy=" << energy0;
    
    //DMRG sweeps
    auto sweeps = Sweeps(400);
    sweeps.maxdim() = 10, 50, 100, 200, 400, 800, 1200, 2000, 2500;
    sweeps.cutoff() = 1E-10;
    sweeps.noise() = 1e-8;
    
    // sweeps.cutoff() = 1E-15;

    // cout << "initState=" << initState << endl;
    // auto [energy,psi] = dmrg(H,randomMPS(initState),sweeps,"Silent");
    double EnergyConvergenceThreshold = 1e-7;                     
    MyDMRGObserver obs(psi0, EnergyConvergenceThreshold, energy0);

    auto [energy, psi] = dmrg(H, randomMPS(initState), sweeps, obs, "Quiet");
    cout << "Final energy=" << energy << endl;
    ofstream myfile;

    unsigned int precision = 16;
    myfile.precision(precision);

    // myfile.open (energy_file_name.c_str(), std::ios_base::app);
    //     myfile << "W4 " << " Nup= " << Nup << " Ndown= " << Ndown << "E = " << fixed << energy << endl;
    // myfile.close();
    //Two-point correlations
    cout << "Superconducting pair correlations:" << endl;
    
    ofstream myfile;
    myfile.open ("Superconducting_W4_15_5.txt");
    for (int i = 1; i <= N; i++)
        for (int j = i + 1; j <= N; j++)
        {
            myfile << "<CupCdn(" << i << ")CdagdnCdagup(" << j << ")>=" << Correlation(psi, "P", i, "Pdag", j, sites) << endl;
        }
    myfile.close();


    // cout << "Density-density correlations:" << endl;
    // myfile.open ("Density_W5_8_1.txt");
    // for (int i = 1; i <= N; i++)
    //     for (int j = i + 1; j <= N; j++)
    //     {
    //         myfile << "<n(" << i << ")n(" << j << ")>=" << Correlation(psi, "Ntot", i, "Ntot", j, sites) << endl;
    //     }
    // myfile.close();
    
    cout << "Single-particle two-point correlators:" << endl;

    myfile.open ("Single_particle_W4_15_5.txt");
    for (int i = 1; i <= N; i++)
        for (int j = i + 1; j <= N; j++)
        {
            myfile << "<Cdagup(" << i << ")Cup(" << j << ")>=" << Correlation(psi, "Cdagup", i, "Cup", j, sites) << endl;
        }
    myfile.close();
    return 0;
}

// int my_test(int i, string filename){
//     ofstream myfile;
//     myfile.open (filename.c_str(), std::ios_base::app);
//         myfile << "Toss a coin " <<  i << endl;
//     myfile.close();
//     return 0;
// }

int main(){
    std::cout << std::setprecision(16); // задаем точность в 16 цифр
    //
    // run_dmrg(40,5,1,1,false,1.0,1.0,-7.0,"W5_energies.txt");

    // for(int i = 13; i <= 15; i++){
    //     for (int j = 0; j<=1; j++){
    run_dmrg(40,4,15,5,false,1.0,1.0,-7.0,"W4_periodic_correlations.txt");
        // }
    // }
    // run_dmrg(40,4,35,1,true,1.0,1.0,-7.0,"W4_periodic_energies.txt");

    // for(int i = 40; i <= 50; i+=5){
    //     for(int j = 0; j <= 1; j++){
    //         run_dmrg(40,4,i,j,true,1.0,1.0,-7.0,"W4_periodic_energies.txt");
    //     }
    // }
    // bool yperiodic, double t_parallel, double t_perp,double U, string energy_file_name


    return 0;
}

