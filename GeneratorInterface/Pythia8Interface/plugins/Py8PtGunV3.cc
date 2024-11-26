#include "GeneratorInterface/Core/interface/GeneratorFilter.h"
#include "GeneratorInterface/ExternalDecays/interface/ExternalDecayDriver.h"

#include "GeneratorInterface/Pythia8Interface/interface/Py8GunBase.h"

#include <numeric>

namespace gen {

class Py8PtGunV3 : public Py8GunBase {

   public:

      Py8PtGunV3( edm::ParameterSet const& );
      ~Py8PtGunV3() {}

      bool generatePartonsAndHadronize() override;
      const char* classname() const override;

   private:

      // PtGun particle(s) characteristics
      double  fMinEta;
      double  fMaxEta;
      double  fMinPt ;
      double  fMaxPt ;
      double  fMinMass ;
      double  fMaxMass ;
      bool    fAddAntiParticle;

};

// implementation
//
Py8PtGunV3::Py8PtGunV3( edm::ParameterSet const& ps )
   : Py8GunBase(ps)
{

   // ParameterSet defpset ;
   edm::ParameterSet pgun_params =
      ps.getParameter<edm::ParameterSet>("PGunParameters"); // , defpset ) ;
   fMinEta     = pgun_params.getParameter<double>("MinEta"); // ,-2.5);
   fMaxEta     = pgun_params.getParameter<double>("MaxEta"); // , 2.5);
   fMinPt      = pgun_params.getParameter<double>("MinPt"); // ,  400.);
   fMaxPt      = pgun_params.getParameter<double>("MaxPt"); // ,  600.);
   fMinMass    = pgun_params.getParameter<double>("MinMass"); // ,  170.);
   fMaxMass    = pgun_params.getParameter<double>("MaxMass"); // ,  175.);
   fAddAntiParticle = pgun_params.getParameter<bool>("AddAntiParticle"); //, false) ;

}

int sum(std::vector <int> dist) {
    return std::accumulate(dist.begin(), dist.end(), 0);
}

// max_element is being used in invpdf function-get_inverse_pdf

double max_element(std::vector <double> dist) {
    double max = 0;
    int s = dist.size();
    for (int i = 0; i < s; i++) {
        double el = dist[i];
        if (max < el){max = el;}
    }
    return max;
}

std::vector <double> get_inverse_pdf(std::vector <int> dist) {
    std::vector <double> invpdf(dist.size());
    double sum_hist = sum(dist);
    int s = dist.size();
    for (int i = 0; i < s; i++) {
        if (dist[i] != 0 ) {
            invpdf[i] = sum_hist / dist[i];
            //std::cout << "Bin " << i << " -> " << invpdf[i] << std::endl;
        }
        else {invpdf[i] = 1;}
    }
    double max_invpdf = max_element(invpdf);
    for (int i = 0; i < s; i++) {
        invpdf[i] = invpdf[i] / max_invpdf;
    }
    return invpdf;
}  



double lookup_mass_invpdf(double mgen, std::vector <double> m_bins, std::vector <double> m_invpdf) {
    int im = 0;
    int s1 = m_bins.size();
    int s2 = m_invpdf.size();
    for (int ib = 0; ib < s1; ib++) {
        im = ib;
        if (ib + 1 >  s2 - 1) { break; }
        if (mgen <= m_bins[ib]) { break; }
    }
    return m_invpdf[im];
}

double lookup_pt_invpdf(double pTgen, std::vector <int> pT_bins, std::vector <double> pT_invpdf) {
    int ipt = 0;
    int s1 = pT_bins.size();
    int s2 = pT_invpdf.size();
    for (int ib = 0; ib < s1; ib++) {
        ipt = ib;
        if (ib + 1 >  s2 - 1) { break; }
        if (pTgen <= pT_bins[ib]) { break; }
    }
    return pT_invpdf[ipt];
}

double lookup_invpdf(double Mgen, std::vector <double> M_bins, int pTgen, std::vector <int> pT_bins, std::vector <double> invpdf) {
    unsigned int ibin = 0;
    unsigned int m1  = M_bins.size();
    unsigned int pt1 = pT_bins.size();
    unsigned int inv = invpdf.size();
    bool found_mass = false;
    bool found_end  = false;
    for (unsigned int ibx = 0; ibx < m1; ibx++) {
        if (found_mass || found_end) { break; }
        for (unsigned int iby = 0; iby < pt1; iby++) {
            ibin = (ibx*pt1)+ iby;
            if ( ((ibx*pt1) + iby + 1) >  (inv - 1) ) {
                found_end = true;
                break;
            }
            if ( (Mgen  <= M_bins[ibx]) && (pTgen <= pT_bins[iby]) ) {
                found_mass = true;
                break;
            }
        }
    }
    return invpdf[ibin];
}

double get_rand_el(std::vector <int> dist) {
    int randomIndex = rand() % dist.size();
      return dist[randomIndex];
}

std::vector <double> pT_bins = {111.11111111, 122.22222222, 133.33333333,
    144.44444444, 155.55555556, 166.66666667, 177.77777778,
    188.88888889, 200.        , 211.11111111, 222.22222222,
    233.33333333, 244.44444444, 255.55555556, 266.66666667,
    277.77777778, 288.88888889, 300. };
std::vector <int> m_bins   = {400. , 411.11111111, 422.22222222, 433.33333333,
    444.44444444, 455.55555556, 466.66666667, 477.77777778,
    488.88888889, 500.        , 511.11111111, 522.22222222,
    533.33333333, 544.44444444, 555.55555556, 566.66666667,
    577.77777778, 588.88888889, 600. };

std::vector <int> occ = {
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
    100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100//
};
std::vector <double> invpdf = get_inverse_pdf(occ);


bool Py8PtGunV3::generatePartonsAndHadronize()
{

   fMasterGen->event.reset();

   for ( size_t i=0; i<fPartIDs.size(); i++ )
   {

      int particleID = fPartIDs[i]; // this is PDG - need to convert to Py8 ???

      double rand_sampler = rand() / double(RAND_MAX);
      double pt           = (fMaxPt-fMinPt) * randomEngine().flat() + fMinPt;
      double mass         = (fMaxMass-fMinMass) * randomEngine().flat() + fMinMass;
      double weight       = lookup_invpdf(mass, m_bins, pt, pT_bins, invpdf);
     // while ( rand_sampler > weight ) {   #Unbiasing here
        // rand_sampler = rand() / double(RAND_MAX);
        // pt           = (fMaxPt-fMinPt) * randomEngine().flat() + fMinPt;
        // mass         = (fMaxMass-fMinMass) * randomEngine().flat() + fMinMass;
        // weight       = lookup_invpdf(mass, m_bins, pt, pT_bins, invpdf);
     // }
      // Calculate angles
      double phi = (fMaxPhi-fMinPhi) * randomEngine().flat() + fMinPhi;
      double eta = (fMaxEta-fMinEta) * randomEngine().flat() + fMinEta;
      double the = 2.*atan(exp(-eta));

      // Calculate momenta
      double pp = pt / sin(the); // sqrt( ee*ee - mass*mass );
      double ee = sqrt( pp*pp + mass*mass );

      double px = pt * cos(phi);
      double py = pt * sin(phi);
      double pz = pp * cos(the);

      if ( !((fMasterGen->particleData).isParticle( particleID )) )
      {
         particleID = std::fabs(particleID) ;
      }

      if( 1<= fabs(particleID) && fabs(particleID) <= 6) // quarks
        (fMasterGen->event).append( particleID, 23, 101, 0, px, py, pz, ee, mass );
      else if (fabs(particleID) == 21)                   // gluons
        (fMasterGen->event).append( 21, 23, 101, 102, px, py, pz, ee, mass );
      else                                               // other
        (fMasterGen->event).append( particleID, 1, 0, 0, px, py, pz, ee, mass );

      // Here also need to add anti-particle (if any)
      // otherwise just add a 2nd particle of the same type
      // (for example, gamma)
      if ( fAddAntiParticle )
      {
        if( 1 <= fabs(particleID) && fabs(particleID) <= 6){ // quarks
          (fMasterGen->event).append( -particleID, 23, 0, 101, -px, -py, -pz, ee, mass );
        }
        else if (fabs(particleID) == 21){                   // gluons
          (fMasterGen->event).append( 21, 23, 102, 101, -px, -py, -pz, ee, mass );
        }
        else if ( (fMasterGen->particleData).isParticle( -particleID ) ){
          (fMasterGen->event).append( -particleID, 1, 0, 0, -px, -py, -pz, ee, mass );
        }
        else {
          (fMasterGen->event).append( particleID, 1, 0, 0, -px, -py, -pz, ee, mass );
        }

      } // antiparticle

   } // fPartIDs

   if ( !fMasterGen->next() ) return false;

   event().reset(new HepMC::GenEvent);
   return toHepMC.fill_next_event( fMasterGen->event, event().get() );

} // generatePartonsAndHadronize()

const char* Py8PtGunV3::classname() const
{
   return "Py8PtGunV3";
}

typedef edm::GeneratorFilter<gen::Py8PtGunV3, gen::ExternalDecayDriver> Pythia8PtGunV3;

} // end namespace

using gen::Pythia8PtGunV3;
DEFINE_FWK_MODULE(Pythia8PtGunV3);
