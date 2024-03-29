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

// Function by F.Misguich
complex<double> Correlation(MPS &psi, string opname1, int i, string opname2, int j, const SiteSet &sites)
{
    // if (j <= i)
        // cout << "Error in Correlation: i=" << i << " j=" << j << endl, exit(0);
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

    // if (i!=j){

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
    // }
    //index linking j to j+1:
    auto lj = rightLinkIndex(psi, j);

    C *= prime(psi(j), lj) * op_j;
    C *= prime(psidag(j), "Site");

    auto result = eltC(C);

    return result;
}


// My corr.function copied from repo.
//TODO: support for pair operators
template <class T> std::vector<std::vector<T>>
myCopiedCorrelationMatrixT(const MPS& _psi,
                   const SiteSet& sites,
                   const string& _op1,
                   const string& _op2,
                   detail::RangeHelper<int> site_range,
                   Args const& args
                  )
{
    assert(checkConsistent(_psi,sites));
    
    // Decide if we need to calculate a non-hermitian corr. matrix which is roughly double the work.
    bool isHermitian=false; // Assume non-hermitian
    if (args.defined("isHermitian")) // Did the user explicitly request something?
        isHermitian= args.getBool("isHermitian"); // Honour users request
    else
    {
        ITensor O1=sites.op(_op1, 1); 
        ITensor O2=sites.op(_op2, 1);
        // We need to decide if O1==O2^dagger allowing for some round off errors.
        double eps=norm(O1 / norm(O1) - dag(swapPrime(O2, 0, 1) / norm(O2)));
        if (eps<1e-10 || _op1==_op2) isHermitian=true;
        // ISy needs this ^^^^^^^^ but only for efficiency
    }

//
// Fix up the site range from default.
//
    fixRange(site_range,_psi.length());
    auto start_site = current(site_range);
    auto end_site = last(site_range)-1;
//
// Copy _psi (because we need to move the ortho centre around), set the ortho centre
// and calculate the norm constant.
//
    MPS psi = _psi; 
    if (!isOrtho(psi)) psi.orthogonalize();
    psi.position(start_site);
    Real norm2_psi = norm(psi(start_site));
    norm2_psi*=norm2_psi; 
    //
    //  Handle fermion operators.
    //
    string op1=_op1;
    string op2=_op2;
    string onsiteOp = op1+"*"+op2;
    SiteTerm st1(op1,1); //Site number doesn't matter?
    SiteTerm st2(op2,1); //Site number doesn't matter?
    bool fermionic1=isFermionic(st1);
    bool fermionic2=isFermionic(st2);
    if (fermionic1!=fermionic2) //for example A_i*C_j
    {
        //println("Operators ",onsiteOp);
        throw std::runtime_error("correlationMatrix: Mixed fermionic and bosonic operators are not supported yet.");      
    }
 
    // Create and initialize the correlation matrix.
    auto Nb = length(site_range);
    std::vector<std::vector<T>> C; //correlation matrix.
    for (auto i:site_range) C.push_back(std::vector<T>(Nb)),(void)i;// (void)i eliminates an unused i warning
    
    ITensor L(1.0); //Accumulated contraction from the left.
    if (start_site > 1)
    {
        Index lind = commonIndex(psi(start_site), psi(start_site - 1));
        L = delta(dag(lind), prime(lind)); //DxD kroneker delta.
    }   
     
    for(auto i:site_range)
    {
        auto ci = i - start_site;  //index into cm matrix.
        ITensor Li = L * psi(i); //Update accumulated contraction from the left.
 
        // Get j == i diagonal correlations

        // We now need to prime all indices on psi(i) except the link to site i+1.
        IndexSet linds = uniqueInds(psi(i), psi(i+1)); //indices in psi(i) that are not in psi(i+1), i.e. not the link
        ITensor psi_i_dag = dag(prime(psi(i), linds));
        ITensor c=Li * sites.op(onsiteOp,i) * psi_i_dag/ norm2_psi;

        assert(order(c)==0); //If there is any screw up in the priming we get a higher order tensor out.
        C[ci][ci] =eltT<T>(c);
        
        //  Get j > i correlations
        if (fermionic2) op1 += "*F"; 
        ITensor Li12 = (Li * sites.op(op1, i)) * dag(prime(psi(i)));
        for (auto j=i + 1;j<=end_site;j++)
        {
          auto cj = j - start_site; //index into cm matrix.
          Index lind = commonIndex(psi(j), Li12);
          Li12 *= psi(j);

          c = (Li12 * sites.op(op2,j)) * dag(prime(prime(psi(j), "Site"), lind));
          C[ci][cj] = eltT<T>(c) / norm2_psi;
          
          if (isHermitian)
            C[cj][ci] = conj(C[ci][cj]); // not always valid.

          if (fermionic2)
            Li12 *= sites.op("F",j) * dag(prime(psi(j)));              
          else
            Li12 *= dag(prime(psi(j), "Link")); //Prime *all* links                          
            
        } // for j
        op1=_op1; // restore op1
        
        if (!isHermitian) //If isHermitian=false the we must calculate the below diag elements explicitly.
        {
            //  Get j < i correlations by swapping the operators
            if (fermionic1) op2 += "*F"; 
            ITensor Li21 = (Li * sites.op(op2, i)) * dag(prime(psi(i)));
            if (fermionic1) Li21=-Li21; //Required because we swapped fermionic ops, instead of sweeping right to left.
            for (auto j=i + 1;j<=end_site;j++)
            {
                auto cj = j - start_site; //index into cm matrix.
                Index lind = commonIndex(psi(j), Li21);
                Li21 *= psi(j);

                c = (Li21 * sites.op(op1,j)) * dag(prime(prime(psi(j), "Site"), lind));
                C[cj][ci] = eltT<T>(c) / norm2_psi;

                if (fermionic1)
                    Li21 *= sites.op("F",j) * dag(prime(psi(j)));              
                else
                    Li21 *= dag(prime(psi(j), "Link")); //Prime *all* links                          

            } // for j
            op2=_op2; //restore op2
        } //if isHermitian
        
        
        L = (L * psi(i)) * dag(prime(psi(i), "Link")); //Update accumulated contraction from the left.
    } // for i
        
    return C;
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
int run_dmrg(int Nx, int Ny, int Nup, int Ndown, bool yperiodic, double t_parallel, double t_perp,double U, 
    string energy_file_name, string superconductive_corr_file_name,
    string one_particle_corr_file_name)
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
    sweeps.maxdim() = 10, 20, 50, 100, 200, 200, 300, 400, 800, 1200, 1800;
    sweeps.cutoff() = 1E-10;
    sweeps.noise() = 1e-9;
    
    double EnergyConvergenceThreshold = 1e-7;                     
    MyDMRGObserver obs(psi0, EnergyConvergenceThreshold, energy0);

    std::setprecision(16);

    auto [energy, psi] = dmrg(H, psi0, sweeps, obs, "Quiet");
    cout << "Final energy=" << energy << endl;
    // ofstream myfile;

    unsigned int precision = 16;
    // myfile.precision(precision);

    // myfile.open (energy_file_name.c_str(), std::ios_base::app);
    //     myfile << "W" << Ny << " Nup= " << Nup << " Ndown= " << Ndown << " E = " << fixed << energy << endl;
    // myfile.close();

    // // Two-point correlations
    // cout << "Superconducting pair correlations:" << endl;
    
    // // ofstream myfile;
    // myfile.open (superconductive_corr_file_name.c_str(), std::ios_base::app);
    // for (int i = 1; i <= N; i++)
    //     for (int j = 1; j <= N; j++)
    //     {
    //         myfile << "<CupCdn(" << i << ")CdagdnCdagup(" << j << ")>=" << Correlation(psi, "P", i, "Pdag", j, sites) << endl;
    //     }
    // myfile.close();


//     // cout << "Density-density correlations:" << endl;
//     // myfile.open ("Density_W5_8_1.txt");
//     // for (int i = 1; i <= N; i++)
//     //     for (int j = i + 1; j <= N; j++)
//     //     {
//     //         myfile << "<n(" << i << ")n(" << j << ")>=" << Correlation(psi, "Ntot", i, "Ntot", j, sites) << endl;
//     //     }
//     // myfile.close();
    
    cout << "Single-particle two-point correlators:" << endl;

    auto corr_matrix = myCopiedCorrelationMatrixT<Complex>(psi, sites, "Cdagup", "Cup", range1(0), Args::global());

    cout << corr_matrix.size() << endl;

    // cout << "<Cdagup(" << 1 << ")Cup(" << 2 << ")>=" << Correlation(psi, "Cdagup", 1, "Cup", 2, sites) << endl;

    for (int i = 0; i < corr_matrix.size(); i++){
        for (int j = 0; j < corr_matrix[i].size(); j++){
            cout << "i=" << i << " " << "j=" << j << " " << corr_matrix[i][j] << endl;

        }
    }
    //     cout << i << endl;
    // myfile.open (one_particle_corr_file_name.c_str(), std::ios_base::app);
    // for (int i = 1; i <= N; i++)
    //     for (int j = 1; j <= N; j++)
    //     {
    //         myfile << "<Cdagup(" << i << ")Cup(" << j << ")>=" << Correlation(psi, "Cdagup", i, "Cup", j, sites) << endl;
    //     }
    // myfile.close();
    // return 0;
// }
    return 0;
}

int main(){
    std::setprecision(16); // задаем точность в 16 цифр
    // for (int i = 30; i <= 50; i+=5){
            // for(int j = 5; j<=10; j+=5){
    int i = 5;
    int j = 2;
    run_dmrg(10,1,i,j,false,1.0,1.0,-7.0,"W1_10_energies.txt", "W1_10_"+to_string(i)+"_"+to_string(j)+"_"+"superconductive_2.txt", "W1_10_"+to_string(i)+"_"+to_string(j)+"_"+"point_2.txt");
    
    
    


    return 0;
}

